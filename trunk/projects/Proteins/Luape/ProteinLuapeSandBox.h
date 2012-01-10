/*-----------------------------------------.---------------------------------.
| Filename: ProteinLuapeSandBox.h          | Protein Luape SandBox           |
| Author  : Francis Maes                   |                                 |
| Started : 13/12/2011 11:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_LUAPE_SAND_BOX_H_
# define LBCPP_PROTEINS_LUAPE_SAND_BOX_H_

# include <lbcpp/Execution/WorkUnit.h>
# include "LuapeProteinPredictorParameters.h"
# include "../Predictor/ProteinPredictor.h"
# include "../Evaluator/ProteinEvaluator.h"

namespace lbcpp
{

class ProteinLuapeSandBox : public WorkUnit
{
public:
  ProteinLuapeSandBox() : maxProteinCount(0), treeDepth(1), complexity(5), relativeBudget(10.0), miniBatchRelativeSize(0.0), numIterations(1000) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins = loadProteinPairs(context, trainingInputDirectory, trainingSupervisionDirectory, "training");
    ContainerPtr testingProteins;// = loadProteinPairs(context, testingInputDirectory, testingSupervisionDirectory, "testing");
    if (!trainingProteins)// || !testingProteins)
      return false;

#if 0
    LargeProteinParametersPtr parameter = LargeProteinParameters::createTestObject(20);
    LargeProteinPredictorParametersPtr predictor = new LargeProteinPredictorParameters(parameter);
    /*predictor->learningMachineName = T("ExtraTrees");
    predictor->x3Trees = 100;
    predictor->x3Attributes = 0;
    predictor->x3Splits = 1;*/
    predictor->learningMachineName = "kNN";
    predictor->knnNeighbors = 5;
#endif
     
    ProteinPredictorParametersPtr predictor = new LuapeProteinPredictorParameters(treeDepth, complexity, relativeBudget, miniBatchRelativeSize, numIterations, false);

    ProteinPredictorPtr iteration = new ProteinPredictor(predictor);
    //iteration->addTarget(dsbTarget);
    //iteration->addTarget(sa20Target);
    iteration->addTarget(ss3Target);
    //iteration->addTarget(ss8Target);
    //iteration->addTarget(stalTarget);
    //iteration->addTarget(drTarget);

    if (!iteration->train(context, trainingProteins, testingProteins, T("Training")))
      return Variable::missingValue(doubleType);

    ProteinEvaluatorPtr evaluator = createEvaluator(true);    
    iteration->evaluate(context, trainingProteins, evaluator, T("Evaluate on training proteins"));

    evaluator = createEvaluator(true);
    CompositeScoreObjectPtr scores = iteration->evaluate(context, testingProteins, evaluator, T("Evaluate on test proteins"));
    return evaluator->getScoreObjectOfTarget(scores, dsbTarget)->getScoreToMinimize();
  }

protected:
  friend class ProteinLuapeSandBoxClass;

  File trainingInputDirectory;
  File trainingSupervisionDirectory;
  File testingInputDirectory;
  File testingSupervisionDirectory;
  size_t maxProteinCount;

  size_t treeDepth;
  size_t complexity;
  double relativeBudget;
  double miniBatchRelativeSize;
  size_t numIterations;

  ContainerPtr loadProteinPairs(ExecutionContext& context, const File& inputDirectory, const File& supervisionDirectory, const String& description)
  {
    ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, maxProteinCount, T("Loading ") + description + T(" proteins"));
    context.informationCallback(String(proteins ? (int)proteins->getNumElements() : 0) + T(" ") + description + T(" proteins"));
    return proteins;
  }

  ProteinEvaluatorPtr createEvaluator(bool isFinalEvaluation) const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("Secondary Structure"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationSensitivityAndSpecificityScore, isFinalEvaluation), 1), T("Disulfide Bonds (Sens. and Spec)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(rocAnalysisEvaluator(binaryClassificationMCCScore, isFinalEvaluation), 1), T("Disulfide Bonds (MCC)"));
    evaluator->addEvaluator(dsbTarget, symmetricMatrixSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationAccuracyScore), 1), T("Disulfide Bonds (Raw)"));
    evaluator->addEvaluator(dsbTarget, new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, 0.0), 0.0), T("Disulfide Bonds (Greedy L=6)"));    
    return evaluator;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEINS_LUAPE_SAND_BOX_H_