/*-----------------------------------------.---------------------------------.
| Filename: ModelBox.h                     |                                 |
| Author  : Julien Becker                  |                                 |
| Started : 09/02/2012 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef _PROTEINS_MODEL_BOX_H_
# define _PROTEINS_MODEL_BOX_H_

# include <lbcpp/Core/Function.h>
# include "../Model/SimpleProteinModel.h"
# include "../Evaluator/SegmentOverlapEvaluator.h"

namespace lbcpp
{

class SimpleModelWorkUnit : public WorkUnit
{
public:
  SimpleModelWorkUnit()
  : numFolds(5), fold(0) {}

  virtual Variable run(ExecutionContext& context)
  {
    ContainerPtr trainingProteins;
    ContainerPtr testingProteins;
    if (!loadProteins(context, trainingProteins, testingProteins))
      return false;

    ProteinTarget target = drTarget;
    SimpleProteinModelPtr m = new SimpleProteinModel(target);

    m->pssmWindowSize = 15;

    m->train(context, trainingProteins, testingProteins, T("Training Model"));

    if (outputDirectory != File::nonexistent)
      m->evaluate(context, testingProteins, saveToDirectoryEvaluator(outputDirectory, T(".xml")), T("Saving test predictions to directory"));

    ProteinEvaluatorPtr evaluator = createEvaluator(target);
    CompositeScoreObjectPtr scores = m->evaluate(context, testingProteins, evaluator, T("EvaluateTest"));

    return evaluator->getScoreToMinimize(scores);
  }

protected:
  friend class SimpleModelWorkUnitClass;

  File inputDirectory;
  File supervisionDirectory;
  File outputDirectory;

  size_t numFolds;
  size_t fold;

  bool loadProteins(ExecutionContext& context, ContainerPtr& trainingProteins, ContainerPtr& testingProteins) const
  {
    size_t numProteinsToLoad = 0;
#if JUCE_MAC && JUCE_DEBUG
    numProteinsToLoad = 100;
#endif

    if (supervisionDirectory.getChildFile(T("train/")).exists()
        && supervisionDirectory.getChildFile(T("test/")).exists())
    {
      context.informationCallback(T("Train/Test split detected !"));
      trainingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("train/")), supervisionDirectory.getChildFile(T("train/")), numProteinsToLoad, T("Loading training proteins"));
      testingProteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory.getChildFile(T("test/")), supervisionDirectory.getChildFile(T("test/")), numProteinsToLoad, T("Loading testing proteins"));
    }
    else
    {
      context.informationCallback(T("No train/test split detected. Fold: ") + String((int)fold) + T(" of ") +  String((int)numFolds));
      ContainerPtr proteins = Protein::loadProteinsFromDirectoryPair(context, inputDirectory, supervisionDirectory, numProteinsToLoad, T("Loading proteins"));
      trainingProteins = proteins->invFold(fold, numFolds);
      testingProteins = proteins->fold(fold, numFolds);
    }

    if (!trainingProteins || trainingProteins->getNumElements() == 0 ||
        !testingProteins || testingProteins->getNumElements() == 0)
    {
      context.errorCallback(T("No proteins found !"));
      return false;
    }
    return true;
  }

  EvaluatorPtr createEvaluator(ProteinTarget target) const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    if (target == ss3Target)
    {
      //evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Protein"));
      evaluator->addEvaluator(ss3Target, elementContainerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Residue"), true);
      //evaluator->addEvaluator(ss3Target, new SegmentOverlapEvaluator(secondaryStructureElementEnumeration), T("SS3-SegmentOverlap"));
    }
    else if (target == drTarget)
    {
      evaluator->addEvaluator(drTarget, elementContainerSupervisedEvaluator(binaryClassificationCurveEvaluator(binaryClassificationAreaUnderCurve, false)), T("DR-AUC"), true);
      //evaluator->addEvaluator(drTarget, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)), T("DR - MCC-Precision-Recall @ 50%"));
    }
    else
      jassertfalse;
    
    return evaluator;
  }
};

class ProteinModelLearnerFunction : public Function
{
public:
  ProteinModelLearnerFunction(const String& inputDirectory,
                              const String& supervisionDirectory,
                              bool isValidation)
    : inputDirectory(inputDirectory),
      supervisionDirectory(supervisionDirectory),
      isValidation(isValidation) {}

  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return proteinModelClass;}

  virtual TypePtr initializeFunction(ExecutionContext& context,
                                     const std::vector<VariableSignaturePtr>& inputVariables,
                                     String& outputName, String& outputShortName)
    {return scalarVariableMeanAndVarianceClass;}

  virtual Variable computeFunction(ExecutionContext& context, const Variable& input) const
  {
    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("train/")), context.getFile(supervisionDirectory).getChildFile(T("train/")), 0, T("Loading proteins"));
    
    ProteinModelPtr model = input.getObjectAndCast<ProteinModel>(context);
    context.resultCallback(T("Model"), model);

    ScalarVariableMeanAndVariancePtr res = new ScalarVariableMeanAndVariance();
    if (isValidation)
    {
      ContainerPtr testingProteins = Protein::loadProteinsFromDirectoryPair(context, context.getFile(inputDirectory).getChildFile(T("test/")), context.getFile(supervisionDirectory).getChildFile(T("test/")), 0, T("Loading proteins"));
      res->push(computeFold(context, model, trainingProteins, testingProteins));
    }
    else
    {
      for (size_t i = 0; i < 10; ++i)
        res->push(computeFold(context, model, trainingProteins->invFold(i, 10), trainingProteins->fold(i, 10)));
    }

    return res;
  }

  double computeFold(ExecutionContext& context, const ProteinModelPtr& model,
                     const ContainerPtr& trainingProteins, const ContainerPtr& testingProteins) const
  {
    ProteinModelPtr clone = model->cloneAndCast<Model>(context);

    if (!clone->train(context, trainingProteins, testingProteins, T("Training")))
      return 101.f;

    ProteinEvaluatorPtr evaluator = createEvaluator(clone->getProteinTarget());
    CompositeScoreObjectPtr scores = clone->evaluate(context, testingProteins, evaluator, T("EvaluateTest"));
    return evaluator->getScoreToMinimize(scores);
  }

protected:
  friend class ProteinModelLearnerFunctionClass;

  String inputDirectory;
  String supervisionDirectory;
  bool isValidation;

  ProteinModelLearnerFunction() {}

  EvaluatorPtr createEvaluator(ProteinTarget target) const
  {
    ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
    if (target == ss3Target)
    {
      //evaluator->addEvaluator(ss3Target, containerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Protein"));
      evaluator->addEvaluator(ss3Target, elementContainerSupervisedEvaluator(classificationEvaluator()), T("SS3-By-Residue"), true);
      //evaluator->addEvaluator(ss3Target, new SegmentOverlapEvaluator(secondaryStructureElementEnumeration), T("SS3-SegmentOverlap"));
    }
    else if (target == drTarget)
    {
      evaluator->addEvaluator(drTarget, elementContainerSupervisedEvaluator(binaryClassificationCurveEvaluator(binaryClassificationAreaUnderCurve, false)), T("DR-AUC"), true);
      //evaluator->addEvaluator(drTarget, elementContainerSupervisedEvaluator(binaryClassificationEvaluator(binaryClassificationMCCScore)), T("DR - MCC-Precision-Recall @ 50%"));
    }
    else
      jassertfalse;
    
    return evaluator;
  }
};

class ProteinModelFeatureFunctionSelectionWorkUnit : public WorkUnit
{
public:
  ProteinModelFeatureFunctionSelectionWorkUnit()
    : target(noTarget), memoryResourceInGb(0) {}
  
  virtual Variable run(ExecutionContext& context)
  {
    if (projectName == String::empty)
    {
      context.errorCallback(T("Please provide a projet name"));
      return false;
    }
    
    if (gridName == String::empty)
    {
      context.errorCallback(T("Please provide the grid identifier (user@grid)"));
      return false;
    }

    if (!memoryResourceInGb)
    {
      context.errorCallback(T("Please provide the amout of expected memory (in Gb)"));
      return false;
    }

    ExecutionContextPtr remoteContext = distributedExecutionContext(context,
                                            T("m24.big.ulg.ac.be"), 1664,
                                            projectName, T("jbecker@screen"), gridName,
                                            fixedResourceEstimator(1, memoryResourceInGb * 1024, 300), false);

    SimpleProteinModelPtr initialModel = new SimpleProteinModel(target);
    initialModel->x3Trees = 1000;
    initialModel->x3Attributes = 0;
    initialModel->x3Splits = 1;

    OptimizationProblemPtr problem = new OptimizationProblem(
        new ProteinModelLearnerFunction(inputDirectory, supervisionDirectory, false),
        initialModel, SamplerPtr(),
        new ProteinModelLearnerFunction(inputDirectory, supervisionDirectory, true));

    std::vector<StreamPtr> streams;
    SimpleProteinModel::createStreamsExceptFor(T("dr"), streams);

    OptimizerPtr optimizer = bestFirstSearchOptimizer(streams, optimizerStateFile);

    return optimizer->compute(*remoteContext.get(), problem);
  }

protected:
  friend class ProteinModelFeatureFunctionSelectionWorkUnitClass;

  String inputDirectory;
  String supervisionDirectory;
  File optimizerStateFile;
  ProteinTarget target;
  
  String projectName;
  String gridName;
  size_t memoryResourceInGb;
};

};

#endif // _PROTEINS_MODEL_BOX_H_