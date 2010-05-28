/*-----------------------------------------.---------------------------------.
| Filename: InferenceContext.cpp           | Inference Context               |
| Author  : Francis Maes                   |                                 |
| Started : 09/04/2010 12:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/ParameterizedInference.h>
#include <lbcpp/Inference/DecoratorInference.h>
#include <lbcpp/Inference/SequentialInference.h>
#include <lbcpp/Inference/ParallelInference.h>
#include <lbcpp/Inference/InferenceBatchLearner.h>
#include <lbcpp/Object/ObjectPair.h>
using namespace lbcpp;

/*
** InferenceContext
*/
ObjectPtr InferenceContext::runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  return inference->run(InferenceContextPtr(this), input, supervision, returnCode);
}

SequentialInferenceStatePtr InferenceContext::makeSequentialInferenceInitialState(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = new SequentialInferenceState(input, supervision);

  state->setCurrentObject(inference->prepareInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return SequentialInferenceStatePtr();

  state->setCurrentSubInference(inference->getInitialSubInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return SequentialInferenceStatePtr();

  return state;
}

void InferenceContext::makeSequentialInferenceNextState(SequentialInferencePtr inference, SequentialInferenceStatePtr state, ObjectPtr subOutput, ReturnCode& returnCode)
{
  state->setCurrentObject(inference->finalizeSubInference(state, subOutput, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return;

  state->setCurrentSubInference(inference->getNextSubInference(state, returnCode));
  if (returnCode != Inference::finishedReturnCode)
    return;

  state->incrementStepNumber();
}

ObjectPtr InferenceContext::runSequentialInference(SequentialInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
{
  SequentialInferenceStatePtr state = makeSequentialInferenceInitialState(inference, input, supervision, returnCode);
  if (!state)
    return ObjectPtr();
  while (!state->isFinal())
  {
    ObjectPairPtr currentInputAndSupervision = inference->prepareSubInference(state, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();

    ObjectPtr subOutput = runInference(state->getCurrentSubInference(), currentInputAndSupervision->getFirst(), currentInputAndSupervision->getSecond(), returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();

    makeSequentialInferenceNextState(inference, state, subOutput, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return state->getCurrentObject();
  }
  return inference->finalizeInference(state, returnCode);
}

Inference::ReturnCode InferenceContext::train(InferenceBatchLearnerPtr learner, InferencePtr inference, ObjectContainerPtr examples)
{
  ReturnCode res = Inference::finishedReturnCode;
  runInference(learner, inference, examples, res);
  return res;
}

void InferenceContext::callStartInferences(size_t count)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->startInferencesCallback(count);
}

void InferenceContext::callFinishInferences()
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->finishInferencesCallback();
}

void InferenceContext::callPreInference(InferenceStackPtr stack, ObjectPtr& input, ObjectPtr& supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->preInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callPostInference(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
{
  for (int i = (int)callbacks.size() - 1; i >= 0; --i)
    callbacks[i]->postInferenceCallback(stack, input, supervision, output, returnCode);
}

void InferenceContext::callClassification(InferenceStackPtr stack, ClassifierPtr& classifier, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->classificationCallback(stack, classifier, input, supervision, returnCode);
}

void InferenceContext::callRegression(InferenceStackPtr stack, RegressorPtr& regressor, ObjectPtr& input, ObjectPtr& supervision, ReturnCode& returnCode)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    callbacks[i]->regressionCallback(stack, regressor, input, supervision, returnCode);
}

void InferenceContext::appendCallback(InferenceCallbackPtr callback)
  {jassert(callback); callbacks.push_back(callback);}

void InferenceContext::removeCallback(InferenceCallbackPtr callback)
{
  for (size_t i = 0; i < callbacks.size(); ++i)
    if (callbacks[i] == callback)
    {
      callbacks.erase(callbacks.begin() + i);
      break;
    }
}

void InferenceContext::clearCallbacks()
  {callbacks.clear();}

class DefaultInferenceContext : public InferenceContext
{
public:
  virtual String getName() const
    {return getClassName();}

  virtual ReturnCode runWithUnsupervisedExamples(InferencePtr inference, ObjectContainerPtr examples)
    {return runWithSupervisedExamples(inference, examples->apply(new ObjectToObjectPairFunction(true, false)));}
};

/*
** SingleThreadedInferenceContext
*/
class SingleThreadedInferenceContext : public DefaultInferenceContext
{
public:
  SingleThreadedInferenceContext()
    : stack(new InferenceStack()) {}

  virtual ReturnCode runWithSupervisedExamples(InferencePtr inference, ObjectContainerPtr examples)
  {
    callStartInferences(examples->size());
    
    ReturnCode returnCode = Inference::finishedReturnCode;
    for (size_t i = 0; i < examples->size(); ++i)
    {
      ObjectPairPtr example = examples->get(i).dynamicCast<ObjectPair>();
      jassert(example);
      runInference(inference, example->getFirst(), example->getSecond(), returnCode);
      if (returnCode == Inference::errorReturnCode)
        break;
      returnCode = Inference::finishedReturnCode;
    }
    callFinishInferences();
    return returnCode;
  }

  virtual ObjectPtr runInference(InferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    stack->push(inference);
    ObjectPtr output;
    returnCode = Inference::finishedReturnCode;
    callPreInference(stack, input, supervision, output, returnCode);
    if (returnCode == Inference::errorReturnCode)
    {
      std::cerr << "Warning: pre-inference failed" << std::endl;
      jassert(false);
      return ObjectPtr();
    }
    
    if (returnCode == Inference::canceledReturnCode)
      {jassert(output);}
    else if (!output)
      output = InferenceContext::runInference(inference, input, supervision, returnCode);  

    callPostInference(stack, input, supervision, output, returnCode);
    stack->pop();
    return output;
  }

  virtual ObjectPtr runParallelInference(ParallelInferencePtr inference, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return ObjectPtr();
    
    size_t n = state->getNumSubInferences();
    for (size_t i = 0; i < n; ++i)
    {
      InferencePtr subInference = state->getSubInference(i);
      if (subInference)
      {
        returnCode = Inference::finishedReturnCode;
        ObjectPtr subOutput = runInference(subInference, state->getSubInput(i), state->getSubSupervision(i), returnCode);
        if (returnCode == Inference::errorReturnCode)
        {
          Object::error("InferenceContext::runParallelInferences", "Could not finish sub inference");
          return ObjectPtr(); 
        }
        state->setSubOutput(i, subOutput);
      }
    }
    return inference->finalizeInference(InferenceContextPtr(this), state, returnCode);
  }

  virtual ObjectPtr runClassification(ClassificationInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ClassifierPtr classifier = step->getClassifier();
    callClassification(stack, classifier, input, supervision, returnCode);
    if (returnCode == Inference::errorReturnCode)
    {
      Object::error("InferenceContext::runClassification", "Could not classify");
      return ObjectPtr(); 
    }
    jassert(classifier);
    step->setClassifier(classifier);
    if (returnCode == Inference::canceledReturnCode)
      return ObjectPtr();
    FeatureGeneratorPtr inputFeatures = input.dynamicCast<FeatureGenerator>();    
    return classifier->predictProbabilities(inputFeatures);
  }

  virtual ObjectPtr runRegression(RegressionInferenceStepPtr step, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    RegressorPtr regressor = step->getRegressor();
    callRegression(stack, regressor, input, supervision, returnCode);
    if (returnCode == Inference::errorReturnCode)
    {
      Object::error("InferenceContext::runRegression", "Could not regress");
      return ObjectPtr(); 
    }
    jassert(regressor);
    step->setRegressor(regressor);
    if (returnCode == Inference::canceledReturnCode)
      return ObjectPtr();
    FeatureGeneratorPtr inputFeatures = input.dynamicCast<FeatureGenerator>();    
    return new Scalar(regressor->predict(inputFeatures));
  }

private:
  InferenceStackPtr stack;
};

InferenceContextPtr lbcpp::singleThreadedInferenceContext()
  {return new SingleThreadedInferenceContext();}
