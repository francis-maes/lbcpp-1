/*-----------------------------------------.---------------------------------.
| Filename: CrossValidationInference.h     | Cross Validation Inference      |
| Author  : Francis Maes                   |                                 |
| Started : 02/10/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_META_CROSS_VALIDATION_H_
# define LBCPP_INFERENCE_META_CROSS_VALIDATION_H_

# include <lbcpp/Data/Pair.h>
# include <lbcpp/Function/Evaluator.h>
# include <lbcpp/Inference/ParallelInference.h>

namespace lbcpp
{

class CrossValidateStepInference : public Inference
{
public:
  CrossValidateStepInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel)
    : Inference(name), evaluator(evaluator), inferenceModel(inferenceModel) {}
  CrossValidateStepInference() {}

  virtual TypePtr getInputType() const
    {return containerClass(pairClass(inferenceModel->getInputType(), inferenceModel->getSupervisionType()));}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return nilType;}


protected:
  friend class CrossValidateStepInferenceClass;

  EvaluatorPtr evaluator;
  InferencePtr inferenceModel;

  virtual Variable run(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    const ContainerPtr& trainingData = pair->getFirst().getObjectAndCast<Container>();
    const ContainerPtr& evaluationData = pair->getSecond().getObjectAndCast<Container>();
    InferencePtr inference = inferenceModel->cloneAndCast<Inference>();
    jassert(trainingData && evaluationData && inference);
    returnCode = context->train(inference, trainingData);
    if (returnCode != finishedReturnCode)
      return Variable();
    returnCode = context->evaluate(inference, evaluationData, evaluator);
    return Variable();
  }
};

class CrossValidationInference : public SharedParallelInference
{
public:
  CrossValidationInference(const String& name, EvaluatorPtr evaluator, InferencePtr inferenceModel, size_t numFolds)
    : SharedParallelInference(name, new CrossValidateStepInference(name + T(" step"), evaluator, inferenceModel)), inferenceModel(inferenceModel), numFolds(numFolds) {}
  CrossValidationInference() {}

  virtual TypePtr getInputType() const
    {return subInference->getInputType();}

  virtual TypePtr getSupervisionType() const
    {return nilType;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return nilType;}

  virtual String getDescription(const Variable& input, const Variable& supervision) const
  {
    const ContainerPtr& trainingData = input.getObjectAndCast<Container>();
    return String((int)numFolds) + T("-Cross Validating ") + inferenceModel->getName() + T(" with ") +
      String((int)trainingData->getNumElements()) + T(" ") + trainingData->getElementsType()->getName() + T("s");
  }

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextWeakPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    const ContainerPtr& trainingData = input.getObjectAndCast<Container>();

    ParallelInferenceStatePtr state(new ParallelInferenceState(input, supervision));
    state->reserve(numFolds);
    for (size_t i = 0; i < numFolds; ++i)
    {
      Variable fold = Variable::pair(trainingData->invFold(i, numFolds), trainingData->fold(i, numFolds));
      state->addSubInference(subInference, fold, supervision);
    }
    return state;
  }

  virtual Variable finalizeInference(InferenceContextWeakPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
    {return Variable();}

protected:
  friend class CrossValidationInferenceClass;

  InferencePtr inferenceModel;
  size_t numFolds;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_META_CROSS_VALIDATION_H_
