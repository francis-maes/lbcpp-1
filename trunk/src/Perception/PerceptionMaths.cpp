/*-----------------------------------------.---------------------------------.
| Filename: PerceptionMaths.cpp            | Perception Math Functions       |
| Author  : Francis Maes                   |                                 |
| Started : 25/08/2010 16:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Perception/PerceptionMaths.h>
#include "../Data/Object/DenseDoubleObject.h"
#include "../Data/Object/DenseObjectObject.h"
using namespace lbcpp;

/*
** DoubleConstUnaryOperation
*/
struct DoubleConstUnaryOperation
{
  void sense(double value)
    {jassert(false);}
  void sense(PerceptionPtr perception, const Variable& input)
    {jassert(false);}
  void sense(ObjectPtr object)
    {jassert(false);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, ObjectPtr object)
{
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = object->getVariable(i);
    if (v.isMissingValue())
      continue;

    if (v.isObject())
      operation.sense(v.getObject());
    else
    {
      jassert(v.isDouble());
      operation.sense(v.getDouble());
    }
  }
}

template<class OperationType>
struct DoubleConstUnaryOperationCallback : public PerceptionCallback
{
  DoubleConstUnaryOperationCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, double value)
    {operation.sense(value);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {operation.sense(value);}

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.isObject())
      operation.sense(value.getObject());
    else
    {
      jassert(value.isDouble() && !value.isMissingValue());
      operation.sense(value.getDouble());
    }
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& input)
    {operation.sense(subPerception, input);}
};

template<class OperationType>
void doubleConstUnaryOperation(OperationType& operation, const PerceptionPtr& perception, const Variable& input)
{
  typedef DoubleConstUnaryOperationCallback<OperationType> Callback;
  Callback callback(operation);
  perception->computePerception(input, &callback);
}

/*
** L0 Norm
*/
struct ComputeL0NormOperation : public DoubleConstUnaryOperation
{
  ComputeL0NormOperation() : res(0) {}

  size_t res;

  void sense(double value)
    {if (value) ++res;}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l0norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l0norm(object);}
};

size_t lbcpp::l0norm(const ObjectPtr& object)
{
  if (object)
  {
    ComputeL0NormOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0;
}

size_t lbcpp::l0norm(const PerceptionPtr& perception, const Variable& input)
  {ComputeL0NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** L1 Norm
*/
struct ComputeL1NormOperation : public DoubleConstUnaryOperation
{
  ComputeL1NormOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += fabs(value);}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::l1norm(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::l1norm(object);}
};

double lbcpp::l1norm(const ObjectPtr& object)
{
  if (object)
  {
    ComputeL1NormOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0.0;
}

double lbcpp::l1norm(const PerceptionPtr& perception, const Variable& input)
  {ComputeL1NormOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Sum of squares
*/
struct ComputeSumOfSquaresOperation : public DoubleConstUnaryOperation
{
  ComputeSumOfSquaresOperation() : res(0.0) {}

  double res;

  void sense(double value)
    {res += value * value;}

  void sense(PerceptionPtr perception, const Variable& input)
    {res += lbcpp::sumOfSquares(perception, input);}

  void sense(ObjectPtr object)
    {res += lbcpp::sumOfSquares(object);}
};

double lbcpp::sumOfSquares(const ObjectPtr& object)
{
  if (object)
  {
    ComputeSumOfSquaresOperation operation;
    doubleConstUnaryOperation(operation, object);
    return operation.res;
  }
  else
    return 0.0;
}

double lbcpp::sumOfSquares(const PerceptionPtr& perception, const Variable& input)
  {ComputeSumOfSquaresOperation operation; doubleConstUnaryOperation(operation, perception, input); return operation.res;}

/*
** Unary Operation
*/
struct DoubleUnaryOperation
{
  void compute(double& value)
    {jassert(false);}
  void compute(ObjectPtr object)
    {jassert(false);}
};

template<class OperationType>
void doubleUnaryOperation(OperationType& operation, const ObjectPtr& object)
{
  size_t n = object->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v = object->getVariable(i);
    if (v.isMissingValue())
      continue;

    if (v.isObject())
      operation.compute(v.getObject());
    else
    {
      jassert(v.isDouble());
      double value = v.getDouble();
      operation.compute(value);
      object->setVariable(i, Variable(value, v.getType()));
    }
  }
}

/*
** MultiplyByScalar
*/
struct MultiplyByScalarOperation : public DoubleUnaryOperation
{
  MultiplyByScalarOperation(double scalar)
    : scalar(scalar) {}

  double scalar;

  void compute(double& value)
    {value *= scalar;}

  void compute(ObjectPtr object)
    {multiplyByScalar(object, scalar);}
};

void lbcpp::multiplyByScalar(const ObjectPtr& object, double scalar)
{
  if (scalar != 1.0 && object)
  {
    MultiplyByScalarOperation operation(scalar);
    doubleUnaryOperation(operation, object);
  }
}

/*
** Dot-product
*/
struct ComputeDotProductCallback : public PerceptionCallback
{
  ComputeDotProductCallback()
    : res(0.0) {}

  double res;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.exists())
    {
      if (value.isObject())
        sense(variableNumber, value.getObject());
      else
        sense(variableNumber, value.getDouble());
    }
  }
};

struct DefaultComputeDotProductCallback : public ComputeDotProductCallback
{
  DefaultComputeDotProductCallback(const ObjectPtr& object)
    : object(object) {}

  ObjectPtr object;

  virtual void sense(size_t variableNumber, double value)
  {
    if (value)
    {
      Variable objectValue = object->getVariable(variableNumber);
      if (objectValue.exists())
        res += objectValue.getDouble() * value;
    }
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    Variable objectValue = object->getVariable(variableNumber);
    if (objectValue.exists())
      res += dotProduct(objectValue.getObject(), value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (subObject)
      res += dotProduct(subObject, subPerception, subInput);
  }
};

struct ComputeDotProductWithDenseObjectCallback : public ComputeDotProductCallback
{
  ComputeDotProductWithDenseObjectCallback(DenseObjectObject* object)
    : object(object) {}

  DenseObjectObject* object;

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr objectValue = object->getObject(variableNumber);
    if (objectValue)
      res += dotProduct(objectValue, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getObject(variableNumber);
    if (subObject)
      res += dotProduct(subObject, subPerception, subInput);
  }
};

struct ComputeDotProductWithDenseDoubleCallback : public ComputeDotProductCallback
{
  ComputeDotProductWithDenseDoubleCallback(DenseDoubleObject* object)
    : object(object) {}

  DenseDoubleObject* object;

  virtual void sense(size_t variableNumber, double value)
  {
    if (value)
    {
      double& objectValue = object->getValueReference(variableNumber);
      if (!object->isMissing(value))
        res += objectValue * value;
    }
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
    {jassert(false);}
};

double lbcpp::dotProduct(const ObjectPtr& object, const PerceptionPtr& perception, const Variable& input)
{
  if (!object)
    return 0.0;

  DenseDoubleObject* denseDoubleObject = dynamic_cast<DenseDoubleObject* >(object.get());
  if (denseDoubleObject)
  {
    ComputeDotProductWithDenseDoubleCallback callback(denseDoubleObject);
    perception->computePerception(input, &callback);
    return callback.res;
  }

  DenseObjectObject* denseObjectObject = dynamic_cast<DenseObjectObject* >(object.get());
  if (denseObjectObject)
  {
    ComputeDotProductWithDenseObjectCallback callback(denseObjectObject);
    perception->computePerception(input, &callback);
    return callback.res;
  }

  {
    DefaultComputeDotProductCallback callback(object);
    perception->computePerception(input, &callback);
    return callback.res;
  }
}

double lbcpp::dotProduct(const ObjectPtr& object1, const ObjectPtr& object2)
{
  jassert(object1->getClass() == object2->getClass());
  
  double res = 0.0;
  size_t n = object1->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v1 = object1->getVariable(i);
    Variable v2 = object2->getVariable(i);
    if (v1.exists() && v2.exists())
    {
      if (v1.isObject())
        res += dotProduct(v1.getObject(), v2.getObject());
      else
      {
        jassert(v1.isDouble());
        res += v1.getDouble() * v2.getDouble();
      }
    }
  }
  return res;
}


/*
** DoubleAssignmentOperation
*/
struct DoubleAssignmentOperation
{
  void compute(ObjectPtr& target, ObjectPtr source)
    {jassert(false);}

  void compute(ObjectPtr& value, PerceptionPtr perception, const Variable& input)
    {jassert(false);}

  void compute(double& value, double otherValue)
    {jassert(false);}
};

template<class OperationType>
struct DoubleAssignmentCallback : public PerceptionCallback
{
  DoubleAssignmentCallback(OperationType& operation)
    : operation(operation) {}

  OperationType& operation;

  virtual void sense(size_t variableNumber, const Variable& value)
  {
    if (value.isObject())
      sense(variableNumber, value.getObject());
    else
      sense(variableNumber, value.getDouble());
  }
}; 

template<class OperationType>
struct DefaultDoubleAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DefaultDoubleAssignmentCallback(const ObjectPtr& object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    Variable targetVariable = object->getVariable(variableNumber);
    double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
    BaseClass::operation.compute(targetValue, value);
    object->setVariable(variableNumber, Variable(targetValue, targetVariable.getType()));
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr targetObject = object->getVariable(variableNumber).getObject();
    BaseClass::operation.compute(targetObject, value);
    object->setVariable(variableNumber, targetObject);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr subObject = object->getVariable(variableNumber).getObject();
    if (!subObject)
    {
      subObject = Variable::create(subPerception->getOutputType()).getObject();
      object->setVariable(variableNumber, subObject);
    }
    BaseClass::operation.compute(subObject, subPerception, subInput);
  }

  ObjectPtr object;
};

template<class OperationType>
struct DenseObjectAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DenseObjectAssignmentCallback(DenseObjectObject* object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    BaseClass::operation.compute(targetObject, value);
  }

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
  {
    ObjectPtr& targetObject = object->getObjectReference(variableNumber);
    BaseClass::operation.compute(targetObject, subPerception, subInput);
  }

  DenseObjectObject* object;
};

template<class OperationType>
struct DenseDoubleAssignmentCallback : public DoubleAssignmentCallback<OperationType>
{
  typedef DoubleAssignmentCallback<OperationType> BaseClass;

  DenseDoubleAssignmentCallback(DenseDoubleObject* object, OperationType& operation)
    : BaseClass(operation), object(object) {}

  virtual void sense(size_t variableNumber, double value)
  {
    double& targetValue = object->getValueReference(variableNumber);
    if (object->isMissing(targetValue))
      targetValue = 0.0;
    BaseClass::operation.compute(targetValue, value);
  }

  virtual void sense(size_t variableNumber, const ObjectPtr& value)
    {jassert(false);}

  virtual void sense(size_t variableNumber, const PerceptionPtr& subPerception, const Variable& subInput)
    {jassert(false);}

  DenseDoubleObject* object;
};

template<class OperationType>
void doubleAssignmentOperation(OperationType& operation, const ObjectPtr& target, const ObjectPtr& source)
{
  size_t n = source->getNumVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable sourceVariable = source->getVariable(i);
    if (sourceVariable.isMissingValue())
      continue;
    Variable targetVariable = target->getVariable(i);

    if (sourceVariable.isObject())
    {
      jassert(targetVariable.isObject());
      ObjectPtr targetObject = targetVariable.getObject();
      operation.compute(targetObject, sourceVariable.getObject());
      target->setVariable(i, targetObject);
    }
    else
    {
      jassert(sourceVariable.isDouble() && targetVariable.isDouble());
      double targetValue = targetVariable.isMissingValue() ? 0.0 : targetVariable.getDouble();
      operation.compute(targetValue, sourceVariable.getDouble());
      target->setVariable(i, Variable(targetValue, targetVariable.getType()));
    }
  }
}

/*
** AddWeighted
*/
struct AddWeightedOperation : public DoubleAssignmentOperation
{
  AddWeightedOperation(double weight)
    : weight(weight) {}

  double weight;

  void compute(ObjectPtr& target, ObjectPtr source)
    {lbcpp::addWeighted(target, source, weight);}

  void compute(ObjectPtr& value, PerceptionPtr perception, const Variable& input)
    {lbcpp::addWeighted(value, perception, input, weight);}

  void compute(double& value, double otherValue)
    {value += weight * otherValue;}
};

void lbcpp::addWeighted(ObjectPtr& target, const PerceptionPtr& perception, const Variable& input, double weight)
{
  if (!weight)
    return;
  if (!target)
    target = Variable::create(perception->getOutputType()).getObject();

  AddWeightedOperation operation(weight);
  DenseDoubleObject* denseDoubleTarget = dynamic_cast<DenseDoubleObject* >(target.get());
  if (denseDoubleTarget)
  {
    typedef DenseDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseDoubleTarget, operation);
    perception->computePerception(input, &callback);
    return;
  }

  DenseObjectObject* denseObjectTarget = dynamic_cast<DenseObjectObject* >(target.get());
  if (denseObjectTarget)
  {
    typedef DenseObjectAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(denseObjectTarget, operation);
    perception->computePerception(input, &callback);
    return;
  }

  {
    typedef DefaultDoubleAssignmentCallback<AddWeightedOperation> Callback;
    Callback callback(target, operation);
    perception->computePerception(input, &callback);
  }
}

void lbcpp::addWeighted(ObjectPtr& target, const ObjectPtr& source, double weight)
{
  if (!weight)
    return;
  if (!target)
    target = Variable::create(source->getClass()).getObject();
  if (target == source)
    lbcpp::multiplyByScalar(target, 1 + weight);
  else
  {
    AddWeightedOperation operation(weight);
    doubleAssignmentOperation(operation, target, source);
  }
}
