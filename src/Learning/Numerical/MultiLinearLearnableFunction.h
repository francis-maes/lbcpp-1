/*-----------------------------------------.---------------------------------.
| Filename: MultiLinearLearnableFunction.h | MultiLinear Learnable Function  |
| Author  : Francis Maes                   |                                 |
| Started : 17/02/2011 01:12               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_

# include <lbcpp/Function/ScalarFunction.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Learning/BatchLearner.h>
# include <lbcpp/Learning/Numerical.h>

namespace lbcpp
{

// DoubleVector<T>, optional ScalarVectorFunction -> DenseDoubleVector<>
class MultiLinearLearnableFunction : public NumericalLearnableFunction
{
public:
  const CompositeDoubleVectorPtr& getParameters() const
    {return parameters.staticCast<CompositeDoubleVector>();}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)doubleVectorClass();}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    featuresEnumeration = DoubleVector::getElementsEnumeration(inputVariables[0]->getType());
    jassert(featuresEnumeration);

    elementsEnumeration = inputVariables[1]->getType().dynamicCast<Enumeration>();
    if (!elementsEnumeration)
    {
      context.errorCallback(T("Could not identify elements enumeration"));
      return TypePtr();
    }

    parametersClass = compositeDoubleVectorClass();
    outputName = T("prediction");
    outputShortName = T("p");
    setBatchLearner(stochasticBatchLearner());
    return denseDoubleVectorClass(elementsEnumeration);
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const CompositeDoubleVectorPtr& parameters = getParameters();
    const DoubleVectorPtr& inputVector = inputs[0].getObjectAndCast<DoubleVector>();

    if (!parameters || !parameters->getNumSubVectors() || !inputVector)
      return Variable::missingValue(getOutputType());
    
    size_t n = parameters->getNumSubVectors();
    jassert(n == elementsEnumeration->getNumElements());
    DenseDoubleVectorPtr res = new DenseDoubleVector(getOutputType());
    for (size_t i = 0; i < n; ++i)
    {
      double prediction = inputVector->dotProduct(parameters->getSubVector(i));
      if (!isNumberValid(prediction))
        return Variable::missingValue(getOutputType());
      res->setValue(i, prediction);
    }
    return res;
  }

  virtual bool computeAndAddGradient(const FunctionPtr& lossFunction, const Variable* inputs, const Variable& prediction,
                                      double& exampleLossValue, DoubleVectorPtr& target, double weight) const
  {
    ScalarVectorFunctionPtr scalarVectorFunction = lossFunction.dynamicCast<ScalarVectorFunction>();
    jassert(scalarVectorFunction);
    const DoubleVectorPtr& input = inputs[0].getObjectAndCast<DoubleVector>();
    const Variable& supervision = inputs[1];
    const DenseDoubleVectorPtr& predictedScores = prediction.getObjectAndCast<DenseDoubleVector>(); 

    DenseDoubleVectorPtr lossGradient = new DenseDoubleVector(getOutputType());
    scalarVectorFunction->computeScalarVectorFunction(predictedScores, &supervision, &exampleLossValue, &lossGradient, 1.0);
    if (!isNumberValid(exampleLossValue) || !isNumberValid(lossGradient->l2norm()))
      return false;
    size_t n = lossGradient->getNumElements();

    CompositeDoubleVectorPtr compositeTarget = target.dynamicCast<CompositeDoubleVector>();
    if (!target || !compositeTarget->getNumSubVectors())
    {
      compositeTarget = new CompositeDoubleVector(parametersClass);
      createCompositeSubVectors(compositeTarget);
      target = compositeTarget;
    }

    jassert(compositeTarget && compositeTarget->getNumSubVectors() == n);
    for (size_t i = 0; i < n; ++i)
      input->addWeightedTo(compositeTarget->getSubVector(i), 0, weight * lossGradient->getValue(i));
    return true;
  }

  lbcpp_UseDebuggingNewOperator

protected:
  EnumerationPtr featuresEnumeration;
  EnumerationPtr elementsEnumeration;

  void createCompositeSubVectors(const CompositeDoubleVectorPtr& composite) const
  {
    size_t n = elementsEnumeration->getNumElements();
    size_t numFeatures = featuresEnumeration->getNumElements();
    jassert(numFeatures);
    ClassPtr subVectorsClass = denseDoubleVectorClass(featuresEnumeration);
    for (size_t i = 0; i < n; ++i)
      composite->appendSubVector(i * numFeatures, new DenseDoubleVector(subVectorsClass));
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_MULTI_LINEAR_LEARNABLE_FUNCTION_H_
