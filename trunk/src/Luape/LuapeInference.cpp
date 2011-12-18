/*-----------------------------------------.---------------------------------.
| Filename: LuapeInference.cpp             | Lua-evolved function            |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2011 15:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Luape/LuapeInference.h>
#include <lbcpp/Luape/LuapeBatchLearner.h>
#include <lbcpp/Learning/Numerical.h> // for convertSupervisionVariableToBoolean
using namespace lbcpp;

/*
** LuapeInference
*/
LuapeInference::LuapeInference()
  : universe(new LuapeNodeUniverse())
{
}

LuapeSamplesCachePtr LuapeInference::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t n = data.size();
  LuapeSamplesCachePtr res = new LuapeSamplesCache(inputs, n);
  for (size_t i = 0; i < n; ++i)
    res->setInputObject(inputs, i, data[i]->getVariable(0).getObject());
  return res;
}

Variable LuapeInference::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  return computeNode(context, inputs[0].getObject());
}

Variable LuapeInference::computeNode(ExecutionContext& context, const ObjectPtr& inputObject) const
{
  LuapeInstanceCachePtr cache = new LuapeInstanceCache();
  cache->setInputObject(inputs, inputObject);
  return cache->compute(context, node);
}

void LuapeInference::setLearner(const LuapeLearnerPtr& learner, size_t maxIterations)
  {setBatchLearner(new LuapeBatchLearner(learner, maxIterations));}

/*
** LuapeRegressor
*/
size_t LuapeRegressor::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRegressor::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? doubleType : (TypePtr)objectClass;}

TypePtr LuapeRegressor::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return doubleType;
}

double LuapeRegressor::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  // compute RMSE
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == data.size());
  double res = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = pred->getValue(i) - data[i].staticCast<Pair>()->getSecond().getDouble();
    res += delta * delta;
  }
  return sqrt(res / (double)n);
}

/*
** LuapeBinaryClassifier
*/
size_t LuapeBinaryClassifier::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeBinaryClassifier::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? sumType(booleanType, probabilityType) : (TypePtr)objectClass;}

TypePtr LuapeBinaryClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return probabilityType;
}

Variable LuapeBinaryClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  double activation = computeNode(context, inputs[0].getObject()).getDouble();
  jassert(activation != doubleMissingValue);
  return Variable(1.0 / (1.0 + exp(-activation)), probabilityType);
}

double LuapeBinaryClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == data.size());
  size_t numErrors = 0;
  for (size_t i = 0; i < n; ++i)
  {
    bool predicted = (pred->getValue(i) > 0);
    Variable supervision = data[i]->getVariable(1);
    bool correct;
    if (!lbcpp::convertSupervisionVariableToBoolean(supervision, correct))
      jassert(false);
    if (predicted != correct)
      ++numErrors;
  }
  return numErrors / (double)n;
}

/*
** LuapeClassifier
*/
size_t LuapeClassifier::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeClassifier::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? sumType(enumValueType, lbcpp::doubleVectorClass()) : (TypePtr)objectClass;}

EnumerationPtr LuapeClassifier::getLabelsFromSupervision(TypePtr supervisionType)
{
  return supervisionType.isInstanceOf<Enumeration>()
    ? supervisionType.staticCast<Enumeration>() : DoubleVector::getElementsEnumeration(supervisionType);
}

TypePtr LuapeClassifier::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  EnumerationPtr enumeration = getLabelsFromSupervision(inputVariables[1]->getType());
  jassert(enumeration);
  doubleVectorClass = denseDoubleVectorClass(enumeration, doubleType);
  node = new LuapeVectorSumNode(enumeration);
  return enumeration;
}

DenseDoubleVectorPtr LuapeClassifier::computeActivations(ExecutionContext& context, const ObjectPtr& input) const
  {return computeNode(context, input).getObjectAndCast<DenseDoubleVector>();}

Variable LuapeClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  DenseDoubleVectorPtr activations = computeActivations(context, inputs[0].getObject());
  return Variable(activations->getIndexOfMaximumValue(), getOutputType());
}

double LuapeClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
  size_t numErrors = 0;
  size_t n = pred->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
    Variable supervision = data[i]->getVariable(1);
    size_t correctClass;
    if (supervision.isInteger())
      correctClass = (size_t)supervision.getInteger();
    else
      correctClass = (size_t)supervision.getObjectAndCast<DoubleVector>()->getIndexOfMaximumValue();
    if (j != correctClass)
      ++numErrors;
  }
  return numErrors / (double)n;
}

/*
** LuapeRanker
*/
size_t LuapeRanker::getNumRequiredInputs() const
  {return 2;}

TypePtr LuapeRanker::getRequiredInputType(size_t index, size_t numInputs) const
  {return index ? simpleDenseDoubleVectorClass : vectorClass();}

TypePtr LuapeRanker::initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
{
  node = new LuapeScalarSumNode();
  return simpleDenseDoubleVectorClass;
}

Variable LuapeRanker::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  // supervision = inputs[1]
  ObjectVectorPtr alternatives = inputs[0].getObjectAndCast<ObjectVector>();
  size_t n = alternatives->getNumElements();
  DenseDoubleVectorPtr scores = new DenseDoubleVector(n, 0.0);
  for (size_t i = 0; i < n; ++i)
  {
    ObjectPtr alternative = alternatives->getElement(i).getObject();
    double score = computeNode(context, alternative).getDouble();
    scores->setValue(i, score);
  }
  return scores;
}

LuapeSamplesCachePtr LuapeRanker::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t numSamples = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    numSamples += alternatives->getNumElements();
  }
  
  LuapeSamplesCachePtr res = new LuapeSamplesCache(inputs, numSamples);
  size_t index = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();
    for (size_t j = 0; j < n; ++j)
      res->setInputObject(inputs, index++, alternatives->getElement(j).getObject());
  }
  return res;
}

double LuapeRanker::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const std::vector<ObjectPtr>& data) const
{
  jassert(false); // not yet implemented
  return 0.0;
}