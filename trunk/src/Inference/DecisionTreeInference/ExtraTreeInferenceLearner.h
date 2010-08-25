/*-----------------------------------------.---------------------------------.
| Filename: ExtraTreeInferenceLearner.h    | Extra Tree Batch Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 25/06/2010 18:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
# define LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_

# include "ExtraTreeInference.h"
# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Data/RandomGenerator.h>

namespace lbcpp 
{

// Input: (Inference, training data ObjectContainer) pair
// Supervision: None
// Output: BinaryDecisionTree
class SingleExtraTreeInferenceLearner : public Inference
{
public:
  SingleExtraTreeInferenceLearner(size_t numAttributeSamplesPerSplit, size_t minimumSizeForSplitting);

protected:
  RandomGeneratorPtr random;

  size_t numAttributeSamplesPerSplit;
  size_t minimumSizeForSplitting;

  virtual Variable run(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode);

  BinaryDecisionTreePtr sampleTree(TypePtr inputClass, TypePtr outputClass, ContainerPtr trainingData);

  void sampleTreeRecursively(BinaryDecisionTreePtr tree, size_t nodeIndex, TypePtr inputType, TypePtr outputType, ContainerPtr trainingData, const std::vector<size_t>& variables);
  bool shouldCreateLeaf(ContainerPtr trainingData, const std::vector<size_t>& variables, TypePtr outputType, Variable& leafValue) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_EXTRA_TREE_LEARNER_H_
