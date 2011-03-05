/*-----------------------------------------.---------------------------------.
| Filename: BinaryDecisionTreeFunction.h   | Extra Tree Inference            |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
# define LBCPP_FUNCTION_BINARY_DECISION_TREE_H_

# include "Data/BinaryDecisionTree.h"
# include <lbcpp/Distribution/Distribution.h>
# include <lbcpp/Learning/LearnableFunction.h>

namespace lbcpp 
{

class BinaryDecisionTreeFunction : public LearnableFunction
{
public:
  BinaryDecisionTreeFunction();

  virtual TypePtr getSupervisionType() const = 0;
  
  /* Function */
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index == 0 ? (TypePtr)objectClass : getSupervisionType();}
  
  virtual String getOutputPostFix() const
    {return T("BinaryTree");}
  
  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    if (!parameters && parameters.staticCast<BinaryDecisionTree>()->getNumNodes())
      return Variable::missingValue(getOutputType());
    return parameters.staticCast<BinaryDecisionTree>()->makePrediction(context, inputs[0]);
  }

  /* BinaryDecisionTreeFunction */
  BinaryDecisionTreePtr getTree() const
    {return parameters ? parameters.staticCast<BinaryDecisionTree>() : BinaryDecisionTreePtr();}

  void setTree(BinaryDecisionTreePtr tree)
    {parameters = tree;}
};

extern ClassPtr binaryDecisionTreeFunctionClass;
typedef ReferenceCountedObjectPtr<BinaryDecisionTreeFunction> BinaryDecisionTreeFunctionPtr;

class RegressionBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return doubleType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return doubleType;}
};

class BinaryClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType, probabilityType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return probabilityType;}
};

class ClassificationBinaryDecisionTreeFunction : public BinaryDecisionTreeFunction
{
public:
  virtual TypePtr getSupervisionType() const
    {return enumValueType;}
  
  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
    {return inputVariables[1]->getType();}
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_BINARY_DECISION_TREE_H_
