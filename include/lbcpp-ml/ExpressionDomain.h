/*-----------------------------------------.---------------------------------.
| Filename: ExpressionDomain.h             | Expression Domain               |
| Author  : Francis Maes                   |                                 |
| Started : 03/10/2012 13:54               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_EXPRESSION_DOMAIN_H_
# define LBCPP_ML_EXPRESSION_DOMAIN_H_

# include "Domain.h"
# include "Expression.h"
# include "ExpressionUniverse.h"
# include "Problem.h"
# include "Search.h"
# include <lbcpp/Luape/LuapeCache.h>

namespace lbcpp
{

/*
** Expression Domain
*/
class ExpressionDomain : public Domain
{
public:
  ExpressionDomain(ExpressionUniversePtr universe = ExpressionUniversePtr());

  const ExpressionUniversePtr& getUniverse() const
    {return universe;}

  /*
  ** Inputs
  */
  size_t getNumInputs() const
    {return inputs.size();}

  const VariableExpressionPtr& getInput(size_t index) const
    {jassert(index < inputs.size()); return inputs[index];}
  
  const std::vector<VariableExpressionPtr>& getInputs() const
    {return inputs;}

  VariableExpressionPtr addInput(const TypePtr& type, const String& name);
  
  /*
  ** Available Constants
  */
  size_t getNumConstants() const
    {return constants.size();}

  const ConstantExpressionPtr& getConstant(size_t index) const
    {jassert(index < constants.size()); return constants[index];}

  void addConstant(const Variable& value)
    {constants.push_back(new ConstantExpression(value));}

  /*
  ** Active variables
  */
  size_t getNumActiveVariables() const
    {return activeVariables.size();}

  ExpressionPtr getActiveVariable(size_t index) const;

  const std::set<ExpressionPtr>& getActiveVariables() const
    {return activeVariables;}

  void addActiveVariable(const ExpressionPtr& node)
    {activeVariables.insert(node);}

  void clearActiveVariables()
    {activeVariables.clear();}

  /*
  ** Supervision variable
  */
  VariableExpressionPtr getSupervision() const
    {return supervision;}

  VariableExpressionPtr createSupervision(const TypePtr& type, const String& name);

  /*
  ** Available Functions
  */
  size_t getNumFunctions() const
    {return functions.size();}

  const FunctionPtr& getFunction(size_t index) const
    {jassert(index < functions.size()); return functions[index];}

  void addFunction(const FunctionPtr& function)
    {functions.push_back(function);}

  size_t getMaxFunctionArity() const;

  /*
  ** Accepted target types
  */
  bool isTargetTypeAccepted(TypePtr type);
  void addTargetType(TypePtr type)
    {targetTypes.insert(type);}
  void clearTargetTypes()
    {targetTypes.clear();}

  /*
  ** Symbol Map
  */
  const std::map<ObjectPtr, size_t>& getSymbolMap() const;
  size_t getSymbolIndex(const ObjectPtr& symbol) const;
  size_t getNumSymbols() const;
  ObjectPtr getSymbol(size_t index) const;
  static size_t getSymbolArity(const ObjectPtr& symbol);

  /*
  ** Object
  */
  virtual String toShortString() const;

  /*
  ** Search space - bof
  */
  PostfixExpressionTypeSpacePtr getSearchSpace(ExecutionContext& context, size_t complexity, bool verbose = false) const; // cached with initialState = vector<TypePtr>()

  PostfixExpressionTypeSpacePtr createTypeSearchSpace(ExecutionContext& context, const std::vector<TypePtr>& initialState, size_t complexity, bool verbose) const;
  void enumerateNodesExhaustively(ExecutionContext& context, size_t complexity, std::vector<ExpressionPtr>& res, bool verbose = false, const PostfixExpressionSequencePtr& subSequence = PostfixExpressionSequencePtr()) const;

  LuapeSamplesCachePtr createCache(size_t size, size_t maxCacheSizeInMb = 512) const;

protected:
  friend class ExpressionDomainClass;

  ExpressionUniversePtr universe;
  std::vector<VariableExpressionPtr> inputs;
  VariableExpressionPtr supervision;
  std::vector<ConstantExpressionPtr> constants;
  std::vector<FunctionPtr> functions;
  std::set<TypePtr> targetTypes;
  std::set<ExpressionPtr> activeVariables;

  CriticalSection typeSearchSpacesLock;
  std::vector<PostfixExpressionTypeSpacePtr> typeSearchSpaces;

  std::map<ObjectPtr, size_t> symbolMap;
  std::vector<ObjectPtr> symbols;

  void addSymbol(ObjectPtr symbol);
};

typedef ReferenceCountedObjectPtr<ExpressionDomain> ExpressionDomainPtr;
extern ClassPtr expressionDomainClass;

/*
** Expression Problems
*/
class ExpressionProblem : public Problem
{
public:
  ExpressionProblem();

  virtual DomainPtr getDomain() const
    {return domain;}

  virtual FitnessLimitsPtr getFitnessLimits() const
    {return limits;}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const;
  virtual bool loadFromString(ExecutionContext& context, const String& str);

  virtual void initialize(ExecutionContext& context) = 0;

protected:
  ExpressionDomainPtr domain;
  FitnessLimitsPtr limits;
};

typedef ReferenceCountedObjectPtr<ExpressionProblem> ExpressionProblemPtr;

/*
** Expression Search Spaces
*/
class ExpressionActionCodeGenerator : public Object
{
public:
  ExpressionActionCodeGenerator(int generator = 0)
    : generator(generator) {}

  enum
  {
    symbolOnly = 0,
    symbolAndStep,
    symbolAndStackSize,
    symbolAndStepAndStackSize,
  };

  size_t getActionCode(ExpressionDomainPtr domain, ObjectPtr symbol, size_t step, size_t stackSize, size_t maxNumSteps)
  {
    size_t symbolCode = domain->getSymbolIndex(symbol);
    if (generator == symbolOnly)
      return symbolCode;
    else if (generator == symbolAndStep)
      return symbolCode * maxNumSteps + step;
    else if (generator == symbolAndStackSize)
      return symbolCode * maxNumSteps + stackSize;
    else
      return (symbolCode * maxNumSteps + step) * maxNumSteps + stackSize;
  }

private:
  int generator;
};

typedef ReferenceCountedObjectPtr<ExpressionActionCodeGenerator> ExpressionActionCodeGeneratorPtr;

class ExpressionState : public SearchState
{
public:
  ExpressionState(ExpressionDomainPtr domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator);
  ExpressionState() {}

  const ExpressionDomainPtr& getDomain() const
    {return domain;}

  size_t getMaxSize() const
    {return maxSize;}
  
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  friend class ExpressionStateClass;

  ExpressionDomainPtr domain;
  size_t maxSize;
  ExpressionActionCodeGeneratorPtr actionCodeGenerator;
};

typedef ReferenceCountedObjectPtr<ExpressionState> ExpressionStatePtr;

extern ExpressionStatePtr prefixExpressionState(ExpressionDomainPtr domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator = ExpressionActionCodeGeneratorPtr());
extern ExpressionStatePtr postfixExpressionState(ExpressionDomainPtr domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator = ExpressionActionCodeGeneratorPtr());
extern ExpressionStatePtr typedPostfixExpressionState(ExpressionDomainPtr domain, size_t maxSize, ExpressionActionCodeGeneratorPtr codeGenerator = ExpressionActionCodeGeneratorPtr());

}; /* namespace lbcpp */

#endif // !LBCPP_ML_EXPRESSION_DOMAIN_H_