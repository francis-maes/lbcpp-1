/*-----------------------------------------.---------------------------------.
| Filename: LuapeLearner.cpp          | Luape Graph Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 17/11/2011 11:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeLearner.h"
using namespace lbcpp;

/*
** LuapeLearner
*/
bool LuapeLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  this->problem = problem;
  this->function = function;
  if (!function->getVotes())
    function->setVotes(function->createVoteVector());
  if (!function->getGraph())
    function->setGraph(problem->createInitialGraph(context));
  this->graph = function->getGraph();
  return true;
}

bool LuapeLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  // we store a local copy to perform (fast) evaluation at each iteration
  if (isTrainingData)
    trainData = data;
  else
    validationData = data;
  function->setGraphSamples(context, isTrainingData, data);
  return true;
}

/*
** BoostingLearner
*/
BoostingLearner::BoostingLearner(BoostingWeakLearnerPtr weakLearner)
  : weakLearner(weakLearner) {}

bool BoostingLearner::initialize(ExecutionContext& context, const LuapeProblemPtr& problem, const LuapeInferencePtr& function)
{
  if (!LuapeLearner::initialize(context, problem, function))
    return false;
  jassert(weakLearner);
  return weakLearner->initialize(context, problem, function);
}

bool BoostingLearner::setExamples(ExecutionContext& context, bool isTrainingData, const std::vector<ObjectPtr>& data)
{
  if (!LuapeLearner::setExamples(context, isTrainingData, data))
    return false;
  (isTrainingData ? predictions : validationPredictions) = function->makeCachedPredictions(context, isTrainingData);
  return true;
}

LuapeNodePtr BoostingLearner::createDecisionStump(ExecutionContext& context, const LuapeNodePtr& numberNode) const
{
  double threshold = computeBestStumpThreshold(context, numberNode);
  return graph->getUniverse()->makeFunctionNode(stumpLuapeFunction(threshold), numberNode);
}

LuapeNodePtr BoostingLearner::doWeakLearning(ExecutionContext& context) const
{
  LuapeNodePtr weakNode = weakLearner->learn(context, refCountedPointerFromThis(this));
  if (!weakNode)
    context.errorCallback(T("Failed to find a weak learner"));
  if (weakNode->getType() != booleanType)
    weakNode = createDecisionStump(context, weakNode); // transforms doubles into decision stumps
  return weakNode;
}

LuapeNodePtr BoostingLearner::doWeakLearningAndAddToGraph(ExecutionContext& context, BooleanVectorPtr& weakPredictions)
{
  LuapeNodePtr weakNode;

  // do weak learning
  {
    TimedScope _(context, "weak learning");
    weakNode = doWeakLearning(context);
    if (!weakNode)
      return LuapeNodePtr();
  }
 
  {
    TimedScope _(context, "add to graph");

    // add missing nodes to graph
    jassert(weakNode->getType() == booleanType);
    graph->pushMissingNodes(context, new LuapeYieldNode(weakNode));

    // update the weak learner
    weakLearner->update(context, refCountedPointerFromThis(this), weakNode);

    // retrieve weak predictions
    weakPredictions = graph->updateNodeCache(context, weakNode, true).staticCast<BooleanVector>();
    jassert(weakPredictions);
  }
  return weakNode;
}

void BoostingLearner::updatePredictionsAndEvaluate(ExecutionContext& context, size_t yieldIndex, const LuapeNodePtr& weakNode) const
{
  VectorPtr trainSamples = graph->updateNodeCache(context, weakNode, true);
  function->updatePredictions(predictions, yieldIndex, trainSamples);
  context.resultCallback(T("train error"), function->evaluatePredictions(context, predictions, trainData));

  if (validationPredictions)
  {
    VectorPtr validationSamples = graph->updateNodeCache(context, weakNode, false);
    function->updatePredictions(validationPredictions, yieldIndex, validationSamples);
    context.resultCallback(T("validation error"), function->evaluatePredictions(context, validationPredictions, validationData));
  }
}
