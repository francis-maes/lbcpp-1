/*-----------------------------------------.---------------------------------.
| Filename: Expression.h                   | Expression classes              |
| Author  : Francis Maes                   |                                 |
| Started : 18/11/2011 15:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_H_
# define LBCPP_LUAPE_NODE_H_

# include "Function.h"
# include <lbcpp/Data/Table.h>
# include <lbcpp/Data/DoubleVector.h>

namespace lbcpp
{

/*
** DataVector
*/
class DataVector;
typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

class DataVector : public Object
{
public:
  enum Implementation
  {
    constantValueImpl = 0,
    ownedVectorImpl,
    cachedVectorImpl,
    noImpl
  };

  DataVector(Implementation implementation, const IndexSetPtr& indices, const TypePtr& elementsType);
  DataVector(const IndexSetPtr& indices, const VectorPtr& ownedVector);
  DataVector();

  static DataVectorPtr createConstant(IndexSetPtr indices, const ObjectPtr& constantValue);
  static DataVectorPtr createCached(IndexSetPtr indices, const VectorPtr& cachedVector);
  
  const TypePtr& getElementsType() const
    {return elementsType;}

  struct const_iterator
  {
    typedef std::vector<size_t>::const_iterator index_iterator;

    const_iterator(const DataVector* owner, size_t position, index_iterator it)
      : owner(owner), position(position), it(it) {}
    const_iterator(const const_iterator& other)
      : owner(other.owner), position(other.position), it(other.it) {}
    const_iterator() : owner(NULL), position(0) {}

    const_iterator& operator =(const const_iterator& other)
      {owner = other.owner; position = other.position; it = other.it; return *this;}

    const_iterator& operator ++()
    {
      ++position;
      ++it;
      return *this;
    }

    inline unsigned char getRawBoolean() const
    {
      if (owner->implementation == constantValueImpl)
        return owner->constantRawBoolean;
      size_t index = (owner->implementation == ownedVectorImpl ? position : *it);
      if (owner->elementsType == booleanType)
        return owner->vector.staticCast<BooleanVector>()->getData()[index];
      else if (owner->elementsType == probabilityType)
      {
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0.5 ? 1 : 0);
      }
      else
      {
        jassert(owner->elementsType == doubleType);
        double value = owner->vector.staticCast<DenseDoubleVector>()->getValue(index);
        return value == doubleMissingValue ? 2 : (value > 0 ? 1 : 0);
      }
    }

    inline int getRawInteger() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return (int)NewInteger::get(owner->constantRawObject);
      case ownedVectorImpl: return (int)NewInteger::get(owner->vector->getElement(position));
      case cachedVectorImpl: return (int)NewInteger::get(owner->vector->getElement(*it));
      default: jassert(false); return 0;
      }
    }

    inline double getRawDouble() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawDouble;
      case ownedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(position);
      case cachedVectorImpl: return owner->vector.staticCast<DenseDoubleVector>()->getValue(*it);
      default: jassert(false); return 0.0;
      }
    }

    inline const ObjectPtr& getRawObject() const
    {
      switch (owner->implementation)
      {
      case constantValueImpl: return owner->constantRawObject;
      case ownedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(position);
      case cachedVectorImpl: return owner->vector.staticCast<ObjectVector>()->get(*it);
      default: jassert(false); static ObjectPtr empty; return empty;
      }
    }

    bool operator ==(const const_iterator& other) const
      {return owner == other.owner && position == other.position;}

    bool operator !=(const const_iterator& other) const
      {return owner != other.owner || position != other.position;}

    size_t getIndex() const
      {return *it;}

  private:
    friend class DataVector;

    const DataVector* owner;
    size_t position;
    index_iterator it;
  };

  const_iterator begin() const
    {return const_iterator(this, 0, indices->begin());}

  const_iterator end() const
    {return const_iterator(this, indices->size(), indices->end());}

  size_t size() const
    {return indices->size();}

  const IndexSetPtr& getIndices() const
    {return indices;}

  Implementation getImplementation() const
    {return implementation;}

  const VectorPtr& getVector() const
    {return vector;}

  ObjectPtr sampleElement(RandomGeneratorPtr random) const;

protected:
  Implementation implementation;
  IndexSetPtr indices;
  TypePtr elementsType;

  unsigned char constantRawBoolean;
  double constantRawDouble;
  ObjectPtr constantRawObject;

  VectorPtr vector;       // ownedVectorImpl and cachedVectorImpl
};

typedef ReferenceCountedObjectPtr<DataVector> DataVectorPtr;

class Expression : public Object
{
public:
  Expression(const TypePtr& type = nilType);

  const TypePtr& getType() const
    {return type;}

  void setType(const TypePtr& type)
    {this->type = type;}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const = 0;
  
  DataVectorPtr compute(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices = IndexSetPtr()) const;

  virtual size_t getNumSubNodes() const
    {return 0;}
    
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {jassert(false); static ExpressionPtr empty; return empty;} 

  size_t getAllocationIndex() const
    {return allocationIndex;}

  void addImportance(double delta);

  double getImportance() const
    {return importance;}

  void setImportance(double importance)
    {jassert(isNumberValid(importance)); this->importance = importance;}

  size_t getDepth() const;
  size_t getNodeDepth(ExpressionPtr node) const;
  size_t getTreeSize() const;
  ExpressionPtr getNodeByTreeIndex(size_t index) const;

  ExpressionPtr cloneAndSubstitute(ExpressionPtr sourceNode, ExpressionPtr targetNode) const;
  void getInternalNodes(std::vector<ExpressionPtr>& res) const;
  void getLeafNodes(std::vector<ExpressionPtr>& res) const;
  ExpressionPtr sampleNode(RandomGeneratorPtr random, double functionSelectionProbability) const;
  ExpressionPtr sampleNode(RandomGeneratorPtr random) const;
  ExpressionPtr sampleSubNode(RandomGeneratorPtr random) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExpressionClass;

  TypePtr type;
  size_t allocationIndex;
  double importance;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const = 0;
};

extern ClassPtr expressionClass;

/*
** Input
*/
class VariableExpression : public Expression
{
public:
  VariableExpression(const TypePtr& type, const String& name, size_t inputIndex);
  VariableExpression();

  virtual String toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class VariableExpressionClass;

  String name;
  size_t inputIndex;
};

/*
** Constant
*/
class ConstantExpression : public Expression
{
public:
  ConstantExpression(const ObjectPtr& value);
  ConstantExpression() {}

  virtual String toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  const ObjectPtr& getValue() const
    {return value;}

  void setValue(const ObjectPtr& value)
    {type = value->getClass(); this->value = value;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ConstantExpressionClass;

  ObjectPtr value;
};

/*
** Function
*/
class FunctionExpression : public Expression
{
public:
  FunctionExpression(const FunctionPtr& function, const std::vector<ExpressionPtr>& arguments);
  FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument1, const ExpressionPtr& argument2);
  FunctionExpression(const FunctionPtr& function, const ExpressionPtr& argument);
  FunctionExpression() {}

  virtual String toShortString() const;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

  virtual size_t getNumSubNodes() const
    {return arguments.size();}
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const FunctionPtr& getFunction() const
    {return function;}

  size_t getNumArguments() const
    {return arguments.size();}

  const ExpressionPtr& getArgument(size_t index) const
    {jassert(index < arguments.size()); return arguments[index];}

  const std::vector<ExpressionPtr>& getArguments() const
    {return arguments;}

  std::vector<ExpressionPtr>& getArguments()
    {return arguments;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class FunctionExpressionClass;

  FunctionPtr function;
  std::vector<ExpressionPtr> arguments;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  void initialize();
};

extern ClassPtr functionExpressionClass;

/*
** Test
*/
class TestExpression : public Expression
{
public:
  TestExpression(const ExpressionPtr& conditionNode, const ExpressionPtr& failureNode, const ExpressionPtr& successNode, const ExpressionPtr& missingNode);
  TestExpression(const ExpressionPtr& conditionNode, TypePtr outputType);
  TestExpression() {}

  virtual String toShortString() const;
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

  virtual size_t getNumSubNodes() const;
  virtual const ExpressionPtr& getSubNode(size_t index) const;

  static void dispatchIndices(const DataVectorPtr& conditionValues, IndexSetPtr& failureIndices, IndexSetPtr& successIndices, IndexSetPtr& missingIndices);

  const ExpressionPtr& getCondition() const
    {return conditionNode;}

  const ExpressionPtr& getFailure() const
    {return failureNode;}

  void setFailure(const ExpressionPtr& node)
    {failureNode = node;}

  const ExpressionPtr& getSuccess() const
    {return successNode;}

  void setSuccess(const ExpressionPtr& node)
    {successNode = node;}

  const ExpressionPtr& getMissing() const
    {return missingNode;}

  void setMissing(const ExpressionPtr& node)
    {missingNode = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class TestExpressionClass;

  ExpressionPtr conditionNode;
  ExpressionPtr failureNode;
  ExpressionPtr successNode;
  ExpressionPtr missingNode;

  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  DataVectorPtr getSubSamples(ExecutionContext& context, const ExpressionPtr& subNode, const TablePtr& data, const IndexSetPtr& subIndices) const;
};

/*
** Sequence
*/
class SequenceExpression : public Expression
{
public:
  SequenceExpression(TypePtr type, const std::vector<ExpressionPtr>& nodes);
  SequenceExpression(TypePtr type) : Expression(type) {}
  SequenceExpression() {}

  virtual String toShortString() const;

  virtual size_t getNumSubNodes() const
    {return nodes.size();}
    
  virtual const ExpressionPtr& getSubNode(size_t index) const
    {return nodes[index];}
  
  void pushNode(ExecutionContext& context, const ExpressionPtr& node, const std::vector<TablePtr>& cachesToUpdate = std::vector<TablePtr>());

  void clearNodes()
    {nodes.clear();}

  void reserveNodes(size_t size)
    {nodes.reserve(size);}

  const std::vector<ExpressionPtr>& getNodes() const
    {return nodes;}

  void setNodes(const std::vector<ExpressionPtr>& nodes)
    {this->nodes = nodes;}

  void setNode(size_t index, const ExpressionPtr& node)
    {jassert(index < nodes.size()); nodes[index] = node;}

protected:
  friend class SequenceExpressionClass;

  std::vector<ExpressionPtr> nodes;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const = 0;
  virtual void updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const = 0;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;
};

typedef ReferenceCountedObjectPtr<SequenceExpression> SequenceExpressionPtr;

/*
** Sum
*/
class ScalarSumExpression : public SequenceExpression
{
public:
  ScalarSumExpression(const std::vector<ExpressionPtr>& nodes, bool convertToProbabilities, bool computeAverage);
  ScalarSumExpression(bool convertToProbabilities = false, bool computeAverage = true);

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

protected:
  friend class ScalarSumExpressionClass;

  bool convertToProbabilities;
  bool computeAverage;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const;
};

class VectorSumExpression : public SequenceExpression
{
public:
  VectorSumExpression(EnumerationPtr enumeration, bool convertToProbabilities);
  VectorSumExpression() {}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

protected:
  friend class VectorSumExpressionClass;

  bool convertToProbabilities;

  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const;
  virtual DataVectorPtr computeSamples(ExecutionContext& context, const TablePtr& data, const IndexSetPtr& indices) const;

  DenseDoubleVectorPtr convertToProbabilitiesUsingSigmoid(const DenseDoubleVectorPtr& activations) const;
};

class CreateSparseVectorExpression : public SequenceExpression
{
public:
  CreateSparseVectorExpression(const std::vector<ExpressionPtr>& nodes);
  CreateSparseVectorExpression();
    
  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const;

protected:
  virtual VectorPtr createEmptyOutputs(size_t numSamples) const;
  virtual void updateOutputs(const VectorPtr& outputs, const DataVectorPtr& newNodeValues, size_t newNodeIndex) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
