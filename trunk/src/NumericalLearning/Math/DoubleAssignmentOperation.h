/*-----------------------------------------.---------------------------------.
| Filename: DoubleAssignmentOperation.h    | Double Assignment Operations:   |
| Author  : Francis Maes                   |   addWeighted                   |
| Started : 19/10/2010 15:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_
# define LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_

# include <lbcpp/NumericalLearning/NumericalLearning.h>
# include "../../Core/Object/DenseObjectObject.h"
# include "../../Core/Object/DenseDoubleObject.h"
# include "../../Core/Object/SparseDoubleObject.h"

namespace lbcpp
{

/*
** DoubleAssignmentOperation
*/
struct DoubleAssignmentOperation
{
  DoubleAssignmentOperation(ExecutionContext& context) : context(context) {}

  ExecutionContext& context;

  void compute(ObjectPtr& target, const ObjectPtr& source)
    {jassert(false);}

  void compute(ObjectPtr& value, const PerceptionPtr& perception, const Variable& input)
    {jassert(false);}

  void compute(double& value, double otherValue)
    {jassert(false);}
};

template<class OperationType>
struct DefaultDoubleAssignmentCallback : public PerceptionCallback
{
  DefaultDoubleAssignmentCallback(const ObjectPtr& object, OperationType& operation)
    : PerceptionCallback(operation.context), operation(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    Variable targetVariable = object->getVariable(variableNumber);
    double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
    operation.compute(targetValue, value);
    object->setVariable(context, variableNumber, Variable(targetValue, targetVariable.getType()));
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    jassert(value);
    ObjectPtr targetObject = object->getVariable(variableNumber).getObject();
    operation.compute(targetObject, value);
    object->setVariable(context, variableNumber, targetObject);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (!subObject)
    {
      subObject = operation.context.createObject(subPerception->getOutputType());
      object->setVariable(context, variableNumber, subObject);
    }
    operation.compute(subObject, subPerception, subInput);
  }

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.isObject())
      sense(variableNumber, value.getObject());
    else
      sense(variableNumber, value.getDouble());
  }

  OperationType& operation;
  ObjectPtr object;
};

template<class OperationType>
struct DenseObjectAssignmentCallback : public PerceptionCallback
{
  DenseObjectAssignmentCallback(DenseObjectObject* object, OperationType& operation)
    : PerceptionCallback(operation.context), operation(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    operation.compute(targetObject, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    operation.compute(targetObject, subPerception, subInput);
  }

  virtual void sense(size_t variableNumber, const Variable& value)
    {sense(variableNumber, value.getObject());}

  OperationType& operation;
  DenseObjectObject* object;
};

template<class OperationType>
struct DenseDoubleAssignmentCallback : public PerceptionCallback
{
  DenseDoubleAssignmentCallback(DenseDoubleObject* object, OperationType& operation)
    : PerceptionCallback(operation.context), operation(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    double& targetValue = object->getValueReference(variableNumber);
    if (object->isMissing(targetValue))
      targetValue = 0.0;
    operation.compute(targetValue, value);
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const Variable& value)
    {sense(variableNumber, value.getDouble());}

  OperationType& operation;
  DenseDoubleObject* object;
};

template<class OperationType>
void doubleAssignmentOperation(OperationType& operation, const DenseDoubleObjectPtr& target, const SparseDoubleObjectPtr& source)
{
  std::vector<double>& targetValues = target->getValues();
  const std::vector< std::pair<size_t, double> >& sourceValues = source->getValues();
  if (sourceValues.size())
  {
    size_t lastIndex = sourceValues.back().first;
    if (targetValues.size() <= lastIndex)
      targetValues.resize(lastIndex + 1, 0.0);
    for (size_t i = 0; i < sourceValues.size(); ++i)
    {
      const std::pair<size_t, double>& sv = sourceValues[i];
      operation.compute(targetValues[sv.first], sv.second);
    }
  }
}

template<class OperationType>
bool doubleAssignmentOperationSpecialImplementation(OperationType& operation, const ObjectPtr& target, const ObjectPtr& source)
{
  DenseDoubleObjectPtr denseTarget = target.dynamicCast<DenseDoubleObject>();
  SparseDoubleObjectPtr sparseSource = source.dynamicCast<SparseDoubleObject>();
  if (denseTarget && sparseSource)
  {
    doubleAssignmentOperation(operation, denseTarget, sparseSource);
    return true;
  }
  return false;
}

template<class OperationType>
void doubleAssignmentOperation(OperationType& operation, const ObjectPtr& target, const ObjectPtr& source)
{
  if (doubleAssignmentOperationSpecialImplementation(operation, target, source))
    return;

  Object::VariableIterator* iterator = source->createVariablesIterator();
  for (; iterator->exists(); iterator->next())
  {
    size_t index;
    Variable sourceVariable = iterator->getCurrentVariable(index);
    if (sourceVariable.isMissingValue())
      continue;
    Variable targetVariable = target->getVariable(index);

    if (sourceVariable.isObject())
    {
      jassert(targetVariable.isObject());
      ObjectPtr targetObject = targetVariable.getObject();
      operation.compute(targetObject, sourceVariable.getObject());
      target->setVariable(operation.context, index, targetObject);
    }
    else
    {
      jassert(sourceVariable.isDouble() && targetVariable.isDouble());
      double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
      operation.compute(targetValue, sourceVariable.getDouble());
      target->setVariable(operation.context, index, Variable(targetValue, targetVariable.getType()));
    }
  }
  delete iterator;
}

/*
** AddWeighted
*/
struct AddWeightedOperation : public DoubleAssignmentOperation
{
  AddWeightedOperation(ExecutionContext& context, double weight)
    : DoubleAssignmentOperation(context), weight(weight) {}

  double weight;

  void compute(ObjectPtr& target, const ObjectPtr& source)
    {lbcpp::addWeighted(context, target, source, weight);}

  void compute(ObjectPtr& value, const PerceptionPtr& perception, const Variable& input)
    {lbcpp::addWeighted(context, value, perception, input, weight);}

  void compute(double& value, double otherValue)
    {value += weight * otherValue;}
};

void addWeighted(ExecutionContext& context, ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight)
{
  context.checkInheritance(input.getType(), perception->getInputType());
  if (target)
    context.checkInheritance((TypePtr)target->getClass(), perception->getOutputType());

  jassert(input.exists());
  if (!weight)
    return;
  TypePtr type = perception->getOutputType();
  if (type == nilType)
    return;
  if (!target)
    target = context.createObject(type);

  AddWeightedOperation operation(context, weight);
  DenseDoubleObject* denseDoubleTarget = dynamic_cast<DenseDoubleObject* >(target.get());
  if (denseDoubleTarget)
  {
    typedef DenseDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseDoubleTarget, operation);
    perception->computePerception(context, input, &callback);
    return;
  }

  DenseObjectObject* denseObjectTarget = dynamic_cast<DenseObjectObject* >(target.get());
  if (denseObjectTarget)
  {
    typedef DenseObjectAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseObjectTarget, operation);
    perception->computePerception(context, input, &callback);
    return;
  }

  {
    typedef DefaultDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(target, operation);
    perception->computePerception(context, input, &callback);
  }
}

void addWeighted(ExecutionContext& context, ObjectPtr& target, const ObjectPtr& source, double weight)
{
  if (!weight)
    return;
  if (!target)
    target = context.createObject(source->getClass());
  if (target == source)
    lbcpp::multiplyByScalar(context, target, 1 + weight);
  else
  {
    AddWeightedOperation operation(context, weight);
    doubleAssignmentOperation(operation, target, source);
  }
}

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_MATH_DOUBLE_ASSIGNMENT_OPERATION_H_
