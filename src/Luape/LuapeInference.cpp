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
#include "NodeBuilder/NodeBuilderTypeSearchSpace.h"
using namespace lbcpp;

/*
** LuapeInference
*/
LuapeInference::LuapeInference(LuapeUniversePtr universe)
  : universe(universe)
{
  if (!universe)
    this->universe = new LuapeUniverse();
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

void LuapeInference::setLearner(const LuapeLearnerPtr& learner, bool verbose)
{
  learner->setVerbose(verbose);
  setBatchLearner(new LuapeBatchLearner(learner));
}

void LuapeInference::setRootNode(ExecutionContext& context, const LuapeNodePtr& node)
{
  if (node != this->node)
  {
    if (this->node)
    {
      trainingCache->uncacheNode(context, this->node);
      if (validationCache)
        validationCache->uncacheNode(context, this->node);
    }
    this->node = node;
    trainingCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
    if (validationCache)
      validationCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
  }
}

LuapeGraphBuilderTypeSearchSpacePtr LuapeInference::getSearchSpace(ExecutionContext& context, size_t complexity) const
{
  LuapeInference* pthis = const_cast<LuapeInference* >(this);

  if (complexity >= typeSearchSpaces.size())
    pthis->typeSearchSpaces.resize(complexity + 1);
  if (typeSearchSpaces[complexity])
    return typeSearchSpaces[complexity];

  LuapeGraphBuilderTypeSearchSpacePtr res = new LuapeGraphBuilderTypeSearchSpace(refCountedPointerFromThis(this), complexity);
  res->pruneStates(context);
  res->assignStateIndices(context);
  pthis->typeSearchSpaces[complexity] = res;
  return res;
}

LuapeSamplesCachePtr LuapeInference::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data) const
{
  size_t n = data.size();
  LuapeSamplesCachePtr res = new LuapeSamplesCache(universe, inputs, n, 512); // default: 512 Mb cache
  VectorPtr supervisionValues = vector(data[0]->getVariableType(1), n);
  for (size_t i = 0; i < n; ++i)
  {
    res->setInputObject(inputs, i, data[i]->getVariable(0).getObject());
    supervisionValues->setElement(i, data[i]->getVariable(1));
  }
  res->cacheNode(context, supervision, supervisionValues, T("Supervision"));
  res->recomputeCacheSize();
  return res;
}

void LuapeInference::setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
{
  supervision = new LuapeInputNode(trainingData[0]->getVariableType(1), T("supervision"));
  trainingCache = createSamplesCache(context, trainingData);
  if (validationData.size())
    validationCache = createSamplesCache(context, validationData);
}

std::vector<LuapeSamplesCachePtr> LuapeInference::getSamplesCaches() const
{
  std::vector<LuapeSamplesCachePtr> res;
  res.push_back(trainingCache);
  if (validationCache)
    res.push_back(validationCache);
  return res;
}

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

double LuapeRegressor::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  // compute RMSE
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  const DenseDoubleVectorPtr& sup = supervisions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == sup->getNumValues());
  double res = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double delta = pred->getValue(i) - sup->getValue(i);
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
  return probabilityType;
}

Variable LuapeBinaryClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  double activation = computeNode(context, inputs[0].getObject()).getDouble();
  jassert(activation != doubleMissingValue);
  return Variable(1.0 / (1.0 + exp(-activation)), probabilityType);
}

double LuapeBinaryClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  const DenseDoubleVectorPtr& pred = predictions.staticCast<DenseDoubleVector>();
  size_t n = pred->getNumValues();
  jassert(n == supervisions->getNumElements());
  size_t numErrors = 0;
  for (size_t i = 0; i < n; ++i)
  {
    bool predicted = (pred->getValue(i) > 0);
    Variable supervision = supervisions->getElement(i);
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
  labels = getLabelsFromSupervision(inputVariables[1]->getType());
  jassert(labels);
  doubleVectorClass = denseDoubleVectorClass(labels, doubleType);
  return denseDoubleVectorClass(labels, probabilityType);
}

DenseDoubleVectorPtr LuapeClassifier::computeActivations(ExecutionContext& context, const ObjectPtr& input) const
  {return computeNode(context, input).getObjectAndCast<DenseDoubleVector>();}

Variable LuapeClassifier::computeFunction(ExecutionContext& context, const Variable* inputs) const
{
  DenseDoubleVectorPtr activations = computeActivations(context, inputs[0].getObject());
  DenseDoubleVectorPtr probabilities = new DenseDoubleVector((ClassPtr)getOutputType());
  size_t n = activations->getNumElements();
  double Z = 0.0;
  for (size_t i = 0; i < n; ++i)
  {
    double prob = 1.0 / (1.0 + exp(-activations->getValue(i)));
    Z += prob;
    probabilities->setValue(i, prob);
  }
  if (Z)
    probabilities->multiplyByScalar(1.0 / Z);
  return probabilities;
}

double LuapeClassifier::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  ObjectVectorPtr pred = predictions.staticCast<ObjectVector>();
  size_t numErrors = 0;
  size_t n = pred->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    size_t j = pred->getAndCast<DenseDoubleVector>(i)->getIndexOfMaximumValue();
    Variable supervision = supervisions->getElement(i);
    size_t correctClass;
    if (lbcpp::convertSupervisionVariableToEnumValue(supervision, correctClass))
    {
      if (j != correctClass)
        ++numErrors;
    }
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

LuapeSamplesCachePtr LuapeRanker::createSamplesCache(ExecutionContext& context, const std::vector<ObjectPtr>& data, std::vector<size_t>& exampleSizes) const
{
  size_t numSamples = 0;
  
  exampleSizes.resize(data.size());
  for (size_t i = 0; i < data.size(); ++i)
  {
    const PairPtr& rankingExample = data[i].staticCast<Pair>();
    const ContainerPtr& alternatives = rankingExample->getFirst().getObjectAndCast<Container>();
    size_t n = alternatives->getNumElements();;
    exampleSizes[i] = n;
    numSamples += n;
  }
  
  LuapeSamplesCachePtr res = new LuapeSamplesCache(universe, inputs, numSamples);
  DenseDoubleVectorPtr supervisionValues = new DenseDoubleVector(numSamples, 0.0);
  size_t index = 0;
  for (size_t i = 0; i < data.size(); ++i)
  {
    const ObjectPtr& rankingExample = data[i];
    const ContainerPtr& alternatives = rankingExample->getVariable(0).getObjectAndCast<Container>();
    const DenseDoubleVectorPtr& costs = rankingExample->getVariable(1).getObjectAndCast<DenseDoubleVector>();
    size_t n = exampleSizes[i];
    for (size_t j = 0; j < n; ++j)
    {
      res->setInputObject(inputs, index, alternatives->getElement(j).getObject());
      supervisionValues->setValue(index, costs->getValue(j));
      ++index;
    }
  }
  res->cacheNode(context, supervision, supervisionValues, T("Supervision"));
  res->recomputeCacheSize();
  return res;
}

void LuapeRanker::setSamples(ExecutionContext& context, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
{
  supervision = new LuapeInputNode(trainingData[0]->getVariableType(1), T("supervision"));
  trainingCache = createSamplesCache(context, trainingData, trainingExampleSizes);
  trainingCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
  if (validationData.size())
  {
    validationCache = createSamplesCache(context, validationData, validationExampleSizes);
    validationCache->cacheNode(context, node, VectorPtr(), "Prediction node", false);
  }
}
double LuapeRanker::evaluatePredictions(ExecutionContext& context, const VectorPtr& predictions, const VectorPtr& supervisions) const
{
  jassert(false); // not yet implemented
  return 0.0;
}