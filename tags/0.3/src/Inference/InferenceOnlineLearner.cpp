/*-----------------------------------------.---------------------------------.
| Filename: InferenceOnlineLearner.cpp     | Inference Online Learners       |
| Author  : Francis Maes                   |                                 |
| Started : 26/05/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "InferenceOnlineLearner/StochasticGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/MiniBatchGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/BatchGradientDescentOnlineLearner.h"
#include "InferenceOnlineLearner/RandomizerInferenceOnlineLearner.h"
#include "InferenceOnlineLearner/StoppingCriterionInferenceOnlineLearner.h"
using namespace lbcpp;

static bool isRandomizationRequired(InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency, InferenceOnlineLearner::UpdateFrequency randomizationFrequency)
{
  jassert(learningUpdateFrequency != InferenceOnlineLearner::never);

  if (randomizationFrequency == InferenceOnlineLearner::never ||
      randomizationFrequency == InferenceOnlineLearner::perStep)
    return false;
  if (learningUpdateFrequency == InferenceOnlineLearner::perStep ||
      learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch)
    return true;
  if (learningUpdateFrequency == InferenceOnlineLearner::perEpisode)
  {
    jassert(randomizationFrequency != InferenceOnlineLearner::perPass); // this combination is not supported
    return false;
  }
  if (learningUpdateFrequency == InferenceOnlineLearner::perPass)
    return false;
  jassert(false);
  return false;
}

InferenceOnlineLearnerPtr lbcpp::gradientDescentInferenceOnlineLearner(
        InferenceOnlineLearner::UpdateFrequency randomizationFrequency,
        InferenceOnlineLearner::UpdateFrequency learningUpdateFrequency, IterationFunctionPtr learningRate, bool normalizeLearningRate,
        InferenceOnlineLearner::UpdateFrequency regularizerUpdateFrequency, ScalarVectorFunctionPtr regularizer,
        InferenceOnlineLearner::UpdateFrequency criterionTestFrequency, StoppingCriterionPtr stoppingCriterion, bool restoreBestParametersWhenLearningStops)
{
  jassert(learningUpdateFrequency != InferenceOnlineLearner::never);
  InferenceOnlineLearnerPtr res;

  size_t miniBatchSize = 0;
  if (learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch)
  {
    miniBatchSize = learningUpdateFrequency - InferenceOnlineLearner::perStepMiniBatch;
    if (miniBatchSize <= 1)
      learningUpdateFrequency = InferenceOnlineLearner::perStep;
  }

  // base learner
  if (learningUpdateFrequency == InferenceOnlineLearner::perStep)
    res = new StochasticGradientDescentOnlineLearner(learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else if (learningUpdateFrequency >= InferenceOnlineLearner::perStepMiniBatch && miniBatchSize < 100)
    res = new MiniBatchGradientDescentOnlineLearner(miniBatchSize, learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);
  else
    res = new BatchGradientDescentOnlineLearner(learningUpdateFrequency,
                                              learningRate, normalizeLearningRate, regularizerUpdateFrequency, regularizer);

  // randomization
  if (isRandomizationRequired(learningUpdateFrequency, randomizationFrequency))
    res = InferenceOnlineLearnerPtr(new RandomizerInferenceOnlineLearner(randomizationFrequency, res));

  // stopping criterion and best parameters restore
  jassert(!restoreBestParametersWhenLearningStops || stoppingCriterion);
  if (stoppingCriterion)
  {
    jassert(criterionTestFrequency != InferenceOnlineLearner::never);
    res = res->addStoppingCriterion(criterionTestFrequency, stoppingCriterion, restoreBestParametersWhenLearningStops);
  }
  else
    jassert(!restoreBestParametersWhenLearningStops);
  return res;
}

InferenceOnlineLearnerPtr InferenceOnlineLearner::addStoppingCriterion(UpdateFrequency criterionTestFrequency, StoppingCriterionPtr criterion, bool restoreBestParametersWhenLearningStops) const
{
  InferenceOnlineLearnerPtr pthis(const_cast<InferenceOnlineLearner* >(this));
  return new StoppingCriterionInferenceOnlineLearner(pthis, criterionTestFrequency, criterion, restoreBestParametersWhenLearningStops);
}

/*
** UpdatableInferenceOnlineLearner
*/
UpdatableInferenceOnlineLearner::UpdatableInferenceOnlineLearner(UpdateFrequency updateFrequency)
  : updateFrequency(updateFrequency), epoch(0) {}

void UpdatableInferenceOnlineLearner::stepFinishedCallback(InferencePtr inference, const Variable& input, const Variable& supervision, const Variable& prediction)
{
  ++epoch;
  if (updateFrequency == perStep)
    update(inference);
  if (updateFrequency >= perStepMiniBatch)
  {
    int miniBatchSize = updateFrequency - perStepMiniBatch;
    if (miniBatchSize <= 1 || (epoch % miniBatchSize) == 0)
      update(inference);
  }
}

void UpdatableInferenceOnlineLearner::episodeFinishedCallback(InferencePtr inference)
{
  if (updateFrequency == perEpisode)
    update(inference);
}

void UpdatableInferenceOnlineLearner::passFinishedCallback(InferencePtr inference)
{
  if (updateFrequency == perPass)
    update(inference);
}

ClassPtr lbcpp::inferenceOnlineLearnerClass()
  {static TypeCache cache(T("InferenceOnlineLearner")); return cache();}

void declareInferenceOnlineLearnerClasses()
{
  LBCPP_DECLARE_ABSTRACT_CLASS(InferenceOnlineLearner, Object);
    LBCPP_DECLARE_CLASS(RandomizerInferenceOnlineLearner, InferenceOnlineLearner);
    LBCPP_DECLARE_CLASS(StoppingCriterionInferenceOnlineLearner, InferenceOnlineLearner);

    LBCPP_DECLARE_ABSTRACT_CLASS(GradientDescentOnlineLearner, InferenceOnlineLearner);
      LBCPP_DECLARE_CLASS(StochasticGradientDescentOnlineLearner, GradientDescentOnlineLearner);
      LBCPP_DECLARE_CLASS(MiniBatchGradientDescentOnlineLearner, GradientDescentOnlineLearner);
      LBCPP_DECLARE_CLASS(BatchGradientDescentOnlineLearner, GradientDescentOnlineLearner);
}