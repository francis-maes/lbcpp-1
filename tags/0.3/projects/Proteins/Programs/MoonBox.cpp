/*
 *  MoonBox.lcpp
 *  LBCpp
 *
 *  Created by Becker Julien on 15/07/10.
 *  Copyright 2010 University of Liège. All rights reserved.
 *
 */

#include <lbcpp/lbcpp.h>

#include "Programs/ArgumentSet.h"

#include "Data/Protein.h" 

#include "Inference/ProteinInferenceFactory.h"
#include "Inference/ProteinInference.h"
#include "Perception/PerceptionToFeatures.h"
#include "Evaluator/ProteinEvaluator.h"


using namespace lbcpp;

extern void declareLBCppCoreClasses();
extern void declareProteinClasses();

struct DefaultParameters;

class ExtraTreeProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    return res ? res->flatten() : PerceptionPtr();
  }
  
  virtual InferencePtr createBinaryClassifier(const String& targetName, TypePtr inputType) const
  {return binaryClassificationExtraTreeInference(targetName, inputType, 2, 3);}
  
  virtual InferencePtr createMultiClassClassifier(const String& targetName, TypePtr inputType, EnumerationPtr classes) const
  {return classificationExtraTreeInference(targetName, inputType, classes, 2, 3);}
};

class NumericalProteinInferenceFactory : public ProteinInferenceFactory
{
public:
  virtual PerceptionPtr createPerception(const String& targetName, bool is1DTarget, bool is2DTarget) const
  {
    PerceptionPtr res = ProteinInferenceFactory::createPerception(targetName, is1DTarget, is2DTarget);
    return res ? PerceptionPtr(new ConvertToFeaturesPerception(res)) : PerceptionPtr();
  }
  
public:
  virtual InferencePtr createBinaryClassifier(const String& targetName, TypePtr inputType) const
  {return binaryLinearSVMInference(createOnlineLearner(targetName + T(" Learner")), targetName + T(" Classifier"));}
  
  virtual InferencePtr createMultiClassClassifier(const String& targetName, TypePtr inputType, EnumerationPtr classes) const
  {
    InferencePtr binaryClassifier = createBinaryClassifier(targetName, inputType);
    InferencePtr res = oneAgainstAllClassificationInference(targetName, classes, binaryClassifier);
    //res->setBatchLearner(onlineToBatchInferenceLearner());
    return res;
  }
  
protected:
  InferenceOnlineLearnerPtr createOnlineLearner(const String& targetName, double initialLearningRate = 1.0) const
  {
    StoppingCriterionPtr stoppingCriterion = maxIterationsStoppingCriterion(3);/*logicalOr(
                                                                                maxIterationsStoppingCriterion(100),  
                                                                                maxIterationsWithoutImprovementStoppingCriterion(1));*/
    
    if (targetName.startsWith(T("contactMap")))
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::perEpisode,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 100000), true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, sumOfSquaresFunction(0.0),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
    else
      return gradientDescentInferenceOnlineLearner(
                                                   InferenceOnlineLearner::never,                                                 // randomization
                                                   InferenceOnlineLearner::perStep, invLinearIterationFunction(initialLearningRate, 10000), true, // learning steps
                                                   InferenceOnlineLearner::perStepMiniBatch20, sumOfSquaresFunction(0.0),         // regularizer
                                                   InferenceOnlineLearner::perPass, stoppingCriterion, true);                     // stopping criterion
  }
};

///////////////////////////////////////// 

class MyInferenceCallback : public InferenceCallback
{
public:
  MyInferenceCallback(InferencePtr inference, ContainerPtr trainingData, ContainerPtr testingData)
  : inference(inference), trainingData(trainingData), testingData(testingData) {}
  
  virtual void preInferenceCallback(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      // top-level learning is beginning
      startingTime = Time::getMillisecondCounter();
      iterationNumber = 0;
    }
    
    String inferenceClassName = stack->getCurrentInference()->getClassName();
    if (inferenceClassName.contains(T("Learner")) && input.size() == 2)
    {
      TypePtr trainingExamplesType = input[1].getObjectAndCast<Container>()->getElementsType();
      jassert(trainingExamplesType->getNumTemplateArguments() == 2);
      String inputTypeName = trainingExamplesType->getTemplateArgument(0)->getName();
      std::cout << "=== Learning " << input[0].getObject()->getName() << " with " << input[1].size() << " " << inputTypeName << "(s) ===" << std::endl;
      //std::cout << "  learner: " << inferenceClassName << " static type: " << input[1].getTypeName() << std::endl
      //  << "  first example type: " << input[1][0].getTypeName() << std::endl << std::endl;
    }
  }
  
  virtual void postInferenceCallback(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
  {
    String inferenceClassName = stack->getCurrentInference()->getClassName();
    
    if (inferenceClassName == T("RunSequentialInferenceStepOnExamples"))
    {
      // end of learning iteration
      std::cout << std::endl
      << "=====================================================" << std::endl
      << "================ EVALUATION =========================  " << (Time::getMillisecondCounter() - startingTime) / 1000 << " s" << std::endl
      << "=====================================================" << std::endl;
      
      InferenceContextPtr validationContext = singleThreadedInferenceContext();
      ProteinEvaluatorPtr evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, trainingData, evaluator);
      processResults(evaluator, true);
      
      evaluator = new ProteinEvaluator();
      validationContext->evaluate(inference, testingData, evaluator);
      processResults(evaluator, false);
      
      std::cout << "=====================================================" << std::endl << std::endl;
    }
    else if (stack->getDepth() == 1)
    {
      std::cout << "Bye." << std::endl;
    }
  }
  
  void processResults(ProteinEvaluatorPtr evaluator, bool isTrainingData)
  {std::cout << " == " << (isTrainingData ? "Training" : "Testing") << " Scores == " << std::endl << evaluator->toString() << std::endl;}
  
private:
  InferencePtr inference;
  ContainerPtr trainingData, testingData;
  size_t iterationNumber;
  juce::uint32 startingTime;
};

/*------------------------------------------------------------------------------
 | Main Function
 -----------------------------------------------------------------------------*/

struct DefaultParameters
{
  static bool   useExtraTrees;
  static double learningRate;
  static size_t learningRateUpdate;
  static bool   useConstantLearning;
  static double regularizer;
  static size_t stoppingIteration;
  static bool   forceUse;
};

bool   DefaultParameters::useExtraTrees       = false;
double DefaultParameters::learningRate        = 1.;
size_t DefaultParameters::learningRateUpdate  = 10000;
bool   DefaultParameters::useConstantLearning = false;
double DefaultParameters::regularizer         = 0.;
size_t DefaultParameters::stoppingIteration   = 20;
bool   DefaultParameters::forceUse            = false;


int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();
  
  enum {numFolds = 5};
  /*
  ** Parameters initialization
  */
  File proteinsDirectory(T("/Users/jbecker/Documents/Workspace/CASP9/SmallPDB/test_version"));
  File testingProteinsDirectory;
  int numProteinsToLoad = 0;
  std::vector<String> targets;
  String output(T("result"));
  
  bool isTestVersion = false;
  String multiTaskFeatures;
  bool isExperimentalMode = false;
  
  ArgumentSet arguments;
  /* Input-Output */
  arguments.insert(new FileArgument(T("ProteinsDirectory"), proteinsDirectory, true, true));
  arguments.insert(new FileArgument(T("TestingProteinsDirectory"), testingProteinsDirectory, true, true));
  arguments.insert(new IntegerArgument(T("NumProteinsToLoad"), numProteinsToLoad));
  arguments.insert(new TargetExpressionArgument(T("Targets"), targets), true);
  arguments.insert(new StringArgument(T("Output"), output));
  /* Learning Parameters */
  arguments.insert(new BooleanArgument(T("useExtraTrees"), DefaultParameters::useExtraTrees));
  arguments.insert(new DoubleArgument(T("LearningRate"), DefaultParameters::learningRate));
  arguments.insert(new IntegerArgument(T("LearningStep"), (int&)DefaultParameters::learningRateUpdate));
  arguments.insert(new DoubleArgument(T("Regularizer"), DefaultParameters::regularizer));
  arguments.insert(new IntegerArgument(T("StoppingIteration"), (int&)DefaultParameters::stoppingIteration));
  /* Perception Parameters */
  // ...
  /* Modes */
  arguments.insert(new BooleanArgument(T("IsTestVersion"), isTestVersion));
  arguments.insert(new BooleanArgument(T("IsExperimentalMode"), isExperimentalMode));
  
  if (!arguments.parse(argv, 1, argc-1))
  {
    std::cout << "Usage: " << argv[0] << " " << arguments.toString() << std::endl;
    return -1;
  }
  
  if (isTestVersion)
  {
    if (!numProteinsToLoad)
      numProteinsToLoad = numFolds;
    DefaultParameters::stoppingIteration = 2;
  }
  
  if (isExperimentalMode)
  {
    DefaultParameters::useConstantLearning = true;
    DefaultParameters::forceUse = true;
  }
  
  std::cout << "*---- Program Parameters -----" << std::endl;
  std::cout << arguments;
  std::cout << "*-----------------------------" << std::endl;
  
  /*
  ** Loading proteins
  */
  ContainerPtr trainingData = directoryFileStream(proteinsDirectory, T("*.xml"))
                            ->apply(loadFromFileFunction(proteinClass()))
                            ->load(numProteinsToLoad)
                            ->apply(proteinToInputOutputPairFunction())
                            ->randomize();
  
  ContainerPtr testingData;
  if (testingProteinsDirectory == File::nonexistent)
  {
    testingData = trainingData->fold(0, numFolds);
    trainingData = trainingData->invFold(0, numFolds);
  }
  else
  {
    testingData = directoryFileStream(testingProteinsDirectory, T("*.xml"))
                ->apply(loadFromFileFunction(proteinClass()))
                ->load()
                ->apply(proteinToInputOutputPairFunction());
  }
  
  std::cout << trainingData->size() << " Training Proteins & "
            << testingData->size()  << " Testing Proteins" << std::endl;
  
  
  /*
  ** Selection of the Protein Inference Factory
  */
  ProteinInferenceFactoryPtr factory;
  if (DefaultParameters::useExtraTrees)
    factory = new ExtraTreeProteinInferenceFactory();
  else
    factory = new NumericalProteinInferenceFactory();
  
  /*
  ** Creation of the inference
  */
  ProteinSequentialInferencePtr inference = new ProteinSequentialInference();
  for (size_t i = 0; i < targets.size(); ++i)
  {
    if (targets[i].contains(T("SS3")))
      inference->appendInference(factory->createInferenceStep(T("secondaryStructure")));
    if (targets[i].contains(T("SS8")))
      inference->appendInference(factory->createInferenceStep(T("dsspSecondaryStructure")));
    if (targets[i].contains(T("SA")))
      inference->appendInference(factory->createInferenceStep(T("solventAccesibilityAt20p")));
    if (targets[i].contains(T("DR")))
      inference->appendInference(factory->createInferenceStep(T("disorderRegions")));
    if (targets[i].contains(T("StAl")))
      inference->appendInference(factory->createInferenceStep(T("structuralAlphabetSequence")));
  }
  
  std::cout << "*--------- Inference ---------" << std::endl;
  Variable(inference).printRecursively(std::cout);
  std::cout << "*-----------------------------" << std::endl;

  /*
  ** Setting Callbacks
  */
  InferenceContextPtr context = singleThreadedInferenceContext();
  context->appendCallback(new MyInferenceCallback(inference, trainingData, testingData));
  
/*  EvaluationInferenceCallbackPtr evaluationCallback = new EvaluationInferenceCallback(proteinInference, trainingData, testingData);
  
  learningContext->appendCallback(new StandardOutputInferenceCallback(evaluationCallback));
  learningContext->appendCallback(new GlobalGnuPlotInferenceCallback(evaluationCallback, output, isExperimentalMode));
  learningContext->appendCallback(new GnuPlotInferenceCallback(evaluationCallback, output, T("SecondaryStructureSequence"), T("DSSPSecondaryStructureSequence")));
  learningContext->appendCallback(new GnuPlotInferenceCallback(evaluationCallback, output, T("ResidueResidueContactMatrix8Ca"), T("BackboneBondSequence")));
  learningContext->appendCallback(new GnuPlotInferenceCallback(evaluationCallback, output, T("StructuralAlphabetSequence"), T("BackboneBondSequence")));
  learningContext->appendCallback(new GnuPlotInferenceCallback(evaluationCallback, output, T("TertiaryStructure"), T("BackboneBondSequence")));
  learningContext->appendCallback(evaluationCallback); // must be the last callback
  if (saveInference)
    learningContext->appendCallback(new SaveInferenceCallback(proteinInference, output));
  */

  /*
  ** Run
  */
  context->train(inference, trainingData);
  return 0;
}