/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/Inference.h>
#include <lbcpp/Object/Predicate.h>
#include <lbcpp/Object/ProbabilityDistribution.h>
#include <lbcpp/Object/Vector.h>
#include "ExtraTreeInferenceLearner.h"
using namespace lbcpp;

/*
** SingleExtraTreeInferenceLearner
*/
SingleExtraTreeInferenceLearner::SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : Inference(T("SingleExtraTreeInferenceLearner")),
    numAttributeSamplesPerSplit(numAttributeSamplesPerSplit),
    minimumSizeForSplitting(minimumSizeForSplitting) {}

Variable SingleExtraTreeInferenceLearner::run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
{
  BinaryDecisionTreeInferencePtr inference = input[0].getObjectAndCast<BinaryDecisionTreeInference>();
  VariableContainerPtr trainingData = input[1].getObjectAndCast<VariableContainer>();
  jassert(inference && trainingData);
  if (!trainingData->size())
    return Variable();

  Variable firstExample = trainingData->getVariable(0);
  jassert(firstExample);
  TypePtr inputClass = firstExample[0].getType();
  TypePtr outputClass = firstExample[1].getType();
#ifdef JUCE_DEBUG
  for (size_t i = 1; i < trainingData->size(); ++i)
  {
    Variable example = trainingData->getVariable(i);
    jassert(example[0].getType() == inputClass);
    jassert(example[1].getType() == outputClass);
  }
#endif // JUCE_DEBUG

  BinaryDecisionTreePtr tree = sampleTree(inputClass, outputClass, trainingData);
  if (tree)
  {
    std::cout << "Finished learning tree, numNodes = " << tree->getNumNodes() << std::endl;
    inference->setTree(tree);
  }
  return Variable();
}

static bool isVariableConstant(VariableContainerPtr container, size_t index1, int index2, Variable& value)
{
  size_t n = container->size();
  if (n <= 1)
    return true;
  value = container->getVariable(0)[index1];
  if (index2 >= 0)
    value = value[(size_t)index2];
  for (size_t i = 1; i < n; ++i)
  {
    Variable otherValue = container->getVariable(i)[index1];
    if (index2 >= 0)
      otherValue = otherValue[(size_t)index2];

    if (value != otherValue)
      return false;
  }
  return true;
}

static bool isInputVariableConstant(VariableContainerPtr trainingData, size_t variableIndex, Variable& value)
  {return isVariableConstant(trainingData, 0, variableIndex, value);}

static bool isOutputConstant(VariableContainerPtr trainingData, Variable& value)
  {return isVariableConstant(trainingData, 1, -1, value);}

bool SingleExtraTreeInferenceLearner::shouldCreateLeaf(VariableContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const
{
  size_t n = trainingData->size();
  jassert(n);

  if (n < minimumSizeForSplitting || variables.empty())
  {
    if (n == 1)
      leafValue = trainingData->getVariable(0)[1];
    else
      leafValue = createOutputDistribution(outputType, trainingData);
    return true;
  }
  return isOutputConstant(trainingData, leafValue);
}
///////////////////////////////////// Create Output Distribution
Variable SingleExtraTreeInferenceLearner::createOutputDistribution(TypePtr outputType, VariableContainerPtr trainingData) const
{
  EnumerationPtr enumeration = outputType.dynamicCast<Enumeration>();
  if (enumeration)
  {
    DiscreteProbabilityDistributionPtr res = new DiscreteProbabilityDistribution(enumeration);
    for (size_t i = 0; i < trainingData->size(); ++i)
      res->increment(trainingData->getVariable(i)[1]);
    return res;
  }
  else
  {
    // Not Implemented
    jassert(false);
    return Variable();
  }
}

///////////////////////////////////// 

///////////////////////////////////// Split Predicate Sampling functions /////////////////////
Variable sampleNumericalSplit(RandomGenerator& random, VariableContainerPtr trainingData, size_t variableIndex)
{
  double minValue = DBL_MAX, maxValue = -DBL_MAX;
  size_t n = trainingData->size();
  for (size_t i = 0; i < n; ++i)
  {
    double value = trainingData->getVariable(i)[variableIndex].getDouble();
    if (value < minValue)
      minValue = value;
    if (value > maxValue)
      maxValue = value;
  }
  return RandomGenerator::getInstance().sampleDouble(minValue, maxValue);
}


Variable sampleEnumerationSplit(RandomGenerator& random, EnumerationPtr enumeration, VariableContainerPtr trainingData, size_t variableIndex)
{
  size_t n = enumeration->getNumElements();

  // enumerate possible values
  std::set<size_t> possibleValues;
  for (size_t i = 0; i < trainingData->size(); ++i)
  {
    Variable value = trainingData->getVariable(i)[0][variableIndex];
    if (value.isNil())
      possibleValues.insert(n);
    else
      possibleValues.insert((size_t)value.getInteger()); // we use this special index to denote the "Nil" value
  }
  jassert(possibleValues.size() >= 2);

  // convert from std::set to std::vector
  std::vector<size_t> possibleValuesVector;
  possibleValuesVector.reserve(possibleValues.size());
  for (std::set<size_t>::const_iterator it = possibleValues.begin(); it != possibleValues.end(); ++it)
    possibleValuesVector.push_back(*it);

  // sample selected values
  std::set<size_t> selectedValues;
  random.sampleSubset(possibleValuesVector, possibleValues.size() / 2, selectedValues);

  // create mask
  BooleanVectorPtr mask = new BooleanVector(n + 1);
  size_t numBits = 0;
  for (size_t i = 0; i < mask->size(); ++i)
  {
    bool bitValue;
    if (possibleValues.find(i) == possibleValues.end())
      bitValue = random.sampleBool(); // 50% probability for values that do not appear in the training data
    else
      bitValue = (selectedValues.find(i) != selectedValues.end()); // true for selected values
    mask->set(i, bitValue);
  }

#ifdef JUCE_DEBUG
  PredicatePtr predicate = BinaryDecisionTree::getSplitPredicate(mask);
  size_t numPos = 0, numNeg = 0;
  for (size_t i = 0; i < trainingData->size(); ++i)
  {
    Variable inputOutputPair = trainingData->getVariable(i);
    if (predicate->compute(inputOutputPair[0][variableIndex]))
      ++numPos;
    else
      ++numNeg;
  }
  //std::cout << "Mask: " << mask->toString() << " " << trainingData->size() << " => " << numPos << " + " << numNeg << std::endl;
  jassert(numPos && numNeg);
#endif // JUCE_DEBUG

  return mask;
}

PredicatePtr sampleSplit(RandomGenerator& random, VariableContainerPtr trainingData, TypePtr inputType, size_t variableIndex, Variable& splitArgument)
{
  TypePtr variableType = inputType->getStaticVariableType(variableIndex);
  if (variableType->inheritsFrom(doubleType()))
  {
    splitArgument = sampleNumericalSplit(random, trainingData, variableIndex);
    return BinaryDecisionTree::getSplitPredicate(splitArgument);
  }

  EnumerationPtr enumeration = variableType.dynamicCast<Enumeration>();
  if (enumeration)
  {
    splitArgument = sampleEnumerationSplit(random, enumeration, trainingData, variableIndex);
    return BinaryDecisionTree::getSplitPredicate(splitArgument);
  }

  jassert(false);
  return PredicatePtr();
}
///////////////////////////////////// Split Scoring  /////////////////////
double computeSplitScore(VariableContainerPtr examples, size_t variableIndex, PredicatePtr predicate, VariableContainerPtr& negativeExamples, VariableContainerPtr& positiveExamples)
{
  VectorPtr neg = new Vector(examples->getStaticType());
  VectorPtr pos = new Vector(examples->getStaticType());
  for (size_t i = 0; i < examples->size(); ++i)
  {
    Variable inputOutputPair = examples->getVariable(i);
    if (predicate->compute(inputOutputPair[0][variableIndex]))
      pos->append(inputOutputPair);
    else
      neg->append(inputOutputPair);
  }
  jassert(pos->size() && neg->size());
  negativeExamples = neg;
  positiveExamples = pos;

  return RandomGenerator::getInstance().sampleDouble(); // FIXME
}

///////////////////////////////////// 

void SingleExtraTreeInferenceLearner::sampleTreeRecursively(BinaryDecisionTreePtr tree, size_t nodeIndex, TypePtr inputType, TypePtr outputType, VariableContainerPtr trainingData, const std::vector<size_t>& variables)
{
  jassert(trainingData->size());

  // update "non constant variables" set
  std::vector<size_t> nonConstantVariables;
  nonConstantVariables.reserve(variables.size());
  for (size_t i = 0; i < variables.size(); ++i)
  {
    Variable value;
    if (!isInputVariableConstant(trainingData, variables[i], value))
      nonConstantVariables.push_back(variables[i]);
  }
  
  Variable leafValue;
  if (shouldCreateLeaf(trainingData, nonConstantVariables, outputType, leafValue))
  {
    tree->createLeaf(nodeIndex, leafValue);
    return;
  }

  // select K split variables
  std::vector<size_t> splitVariables;
  if (numAttributeSamplesPerSplit >= nonConstantVariables.size())
    splitVariables = nonConstantVariables;
  else
    RandomGenerator::getInstance().sampleSubset(nonConstantVariables, numAttributeSamplesPerSplit, splitVariables); 
  size_t K = splitVariables.size();
  
  // generate split predicates, score them, and keep the best one
  PredicatePtr bestSplitPredicate;
  size_t bestSplitVariable;
  Variable bestSplitArgument;
  double bestSplitScore = -DBL_MAX;
  VariableContainerPtr bestNegativeExamples, bestPositiveExamples;
  for (size_t i = 0; i < K; ++i)
  {
    Variable splitArgument;
    PredicatePtr splitPredicate = sampleSplit(random, trainingData, inputType, splitVariables[i], splitArgument);
    VariableContainerPtr negativeExamples, positiveExamples;
    double splitScore = computeSplitScore(trainingData, splitVariables[i], splitPredicate, negativeExamples, positiveExamples);
    if (splitScore > bestSplitScore)
    {
      bestSplitPredicate = splitPredicate;
      bestSplitVariable = splitVariables[i];
      bestSplitArgument = splitArgument;
      bestNegativeExamples = negativeExamples;
      bestPositiveExamples = positiveExamples;
    }
  }

  jassert(bestSplitArgument && bestNegativeExamples && bestPositiveExamples);

  // allocate child nodes
  size_t leftChildIndex = tree->allocateNodes(2);

  // create the node
  tree->createInternalNode(nodeIndex, bestSplitVariable, bestSplitArgument, leftChildIndex);

  // call recursively
  sampleTreeRecursively(tree, leftChildIndex, inputType, outputType, bestNegativeExamples, nonConstantVariables);
  sampleTreeRecursively(tree, leftChildIndex + 1, inputType, outputType, bestPositiveExamples, nonConstantVariables);
}

BinaryDecisionTreePtr SingleExtraTreeInferenceLearner::sampleTree(TypePtr inputClass, TypePtr outputClass, VariableContainerPtr trainingData)
{
  size_t n = trainingData->size();
  if (!n)
    return BinaryDecisionTreePtr();

  // we start with all variables
  std::vector<size_t> variables(inputClass->getNumStaticVariables());
  for (size_t i = 0; i < variables.size(); ++i)
    variables[i] = i;

  // create the initial binary decision tree
  BinaryDecisionTreePtr res = new BinaryDecisionTree();
  res->reserveNodes(n);
  size_t nodeIndex = res->allocateNodes(1);
  jassert(nodeIndex == 0);

  // sample tree recursively
  sampleTreeRecursively(res, nodeIndex, inputClass, outputClass, trainingData, variables);
  return res;
}

//////////////////////////

ExtraTreeInference::ExtraTreeInference(const String& name, size_t numTrees, size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting)
  : ParallelVoteInference(name)
{
  InferencePtr baseLearner = new SingleExtraTreeInferenceLearner(numAttributeSamplesPerSplit, minimumSizeForSplitting);
  subInferences.resize(numTrees);
  for (size_t i = 0; i < numTrees; ++i)
  {
    InferencePtr treeInference = new BinaryDecisionTreeInference(name);
    treeInference->setBatchLearner(baseLearner);
    subInferences.set(i, treeInference);
  }
  setBatchLearner(parallelInferenceLearner());
}
