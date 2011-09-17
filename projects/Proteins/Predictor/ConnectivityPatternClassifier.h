/*-----------------------------------------.---------------------------------.
| Filename: ConnectivityPatternClassifier.h| Connectivity Pattern Classifier |
| Author  : Julien Becker                   |                                 |
| Started : 16/06/2011 13:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_
# define LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_

# include "../../src/Learning/Numerical/LinearLearningMachine.h"
# include "../../src/Function/Evaluator/Utilities.h"
# include "../Evaluator/DisulfideBondEvaluator.h"
# include "../Data/ProteinFunctions.h"

namespace lbcpp
{

extern BatchLearnerPtr addConnectivityPatternBiasBatchLearner();

extern ClassPtr addConnectivityPatternBiasLearnableFunctionClass;

class AddConnectivityPatternBiasLearnableFunction : public Function
{
public:
  AddConnectivityPatternBiasLearnableFunction()
    : bias(0.0)
    {thisClass = addConnectivityPatternBiasLearnableFunctionClass;}
  
  virtual size_t getNumRequiredInputs() const
    {return 2;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return index ? anyType : (TypePtr)containerClass(doubleType);}

  virtual TypePtr initializeFunction(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, String& outputName, String& outputShortName)
  {
    outputName = T("biased");
    outputShortName = T("b");
    setBatchLearner(addConnectivityPatternBiasBatchLearner());
    return inputVariables[0]->getType();
  }

  virtual Variable computeFunction(ExecutionContext& context, const Variable* inputs) const
  {
    const ContainerPtr& prediction = inputs[0].getObjectAndCast<Container>(context);
    jassert(prediction);
    
    const size_t n = prediction->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      const Variable value = prediction->getElement(i);
      if (!value.exists())
        continue;
      prediction->setElement(i, Variable(value.getDouble() + bias, doubleType));
    }

    return prediction;
  }

  void setBias(double bias)
    {this->bias = bias;}

protected:
  friend class AddConnectivityPatternBiasLearnableFunctionClass;

  double bias;
};

class AddConnectivityPatternBiasBatchLearner : public BatchLearner
{
public:
  virtual TypePtr getRequiredFunctionType() const
    {return addConnectivityPatternBiasLearnableFunctionClass;}
  
  virtual bool train(ExecutionContext& context, const FunctionPtr& f, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData) const
  {
    const ReferenceCountedObjectPtr<AddConnectivityPatternBiasLearnableFunction>& function = f.staticCast<AddConnectivityPatternBiasLearnableFunction>();
    
    ROCScoreObject roc;
    insertToROCScoreObject(context, trainingData, roc);
    //insertToROCScoreObject(context, validationData, roc);
    
    if (!roc.getSampleCount())
    {
      context.errorCallback(T("No training examples"));
      return false;
    }
    // Get threshold
    std::vector<double> thresholds;
    roc.getAllThresholds(thresholds);
    
    double bestScore = DBL_MAX;
    double bestThreshold = 0.0;
    double previousThreshold = thresholds.size() ? thresholds[0] - 1.0 : 0;
    size_t numTestedThresholds = 0;
    // Find the best threshold the 0/1 Loss score
    for (size_t i = 0; i < thresholds.size(); ++i)
    {
      if (thresholds[i] - previousThreshold <= 1e-3)
        continue;
      else
        previousThreshold = thresholds[i];
      ++numTestedThresholds;
      context.enterScope(T("Threshold ") + String((int)i));

      SupervisedEvaluatorPtr evaluator = new DisulfidePatternEvaluator(new GreedyDisulfidePatternBuilder(6, thresholds[i], doubleType), thresholds[i]);
      ScoreObjectPtr scoreObject = evaluator->createEmptyScoreObject(context, FunctionPtr());

      insertToEvaluatorScoreObject(context, trainingData, evaluator, scoreObject);
      //insertToEvaluatorScoreObject(context, validationData, evaluator, scoreObject);

      evaluator->finalizeScoreObject(scoreObject, FunctionPtr());
      double score = scoreObject->getScoreToMinimize();
      context.resultCallback(T("threshold"), thresholds[i]);
      context.resultCallback(T("score"), score);
      if (score < bestScore)
      {
        bestScore = score;
        bestThreshold = thresholds[i];
      }
      context.leaveScope();
    }

    context.resultCallback(T("Best threshold"), bestThreshold);
    context.resultCallback(T("Best threshold score"), bestScore);
    context.resultCallback(T("Num. tested thresholds"), numTestedThresholds);
    function->setBias(-bestThreshold);
    return true;
  }
  
protected:  
  void insertToROCScoreObject(ExecutionContext& context, const std::vector<ObjectPtr>& data, ROCScoreObject& roc) const
  {
    for (size_t i = 0; i < data.size(); ++i)
    {
      const ObjectPtr& example = data[i];

      ContainerPtr supervision = example->getVariable(1).getObjectAndCast<Container>(context);
      ContainerPtr prediction = example->getVariable(0).getObjectAndCast<Container>(context);
      jassert(supervision && prediction);

      const size_t n = supervision->getNumElements();
      for (size_t j = 0; j < n; ++j)
      {
        bool isPositive;
        if (convertSupervisionVariableToBoolean(supervision->getElement(j), isPositive))
          roc.addPrediction(context, prediction->getElement(j).getDouble(), isPositive);
      }
    }
  }
  
  void insertToEvaluatorScoreObject(ExecutionContext& context, const std::vector<ObjectPtr>& data, const SupervisedEvaluatorPtr& evaluator, const ScoreObjectPtr& scoreObject) const
  {
    for (size_t i = 0; i < data.size(); ++i)
    {
      const ObjectPtr& example = data[i];
      evaluator->addPrediction(context, example->getVariable(0), example->getVariable(1), scoreObject);
    }
  }
};

extern ClassPtr connectivityPatternClassifierClass;

class ConnectivityPatternClassifier : public CompositeFunction
{
public:
  ConnectivityPatternClassifier(FunctionPtr classifier, bool isSymmetricMatrix)
    : classifier(classifier), isSymmetricMatrix(isSymmetricMatrix)
    {thisClass = connectivityPatternClassifierClass;}

  ConnectivityPatternClassifier() : isSymmetricMatrix(false) {}

  virtual String getOutputPostFix() const
    {return T("PatternPrediction");}

  virtual void buildFunction(CompositeFunctionBuilder& builder)
  {
    size_t input;
    size_t supervision;
    if (isSymmetricMatrix)
    {
      input = builder.addInput(symmetricMatrixClass(doubleVectorClass()));
      supervision = builder.addInput(symmetricMatrixClass(probabilityType));
    }
    else
    {
      input = builder.addInput(matrixClass(doubleVectorClass()));
      supervision = builder.addInput(matrixClass(probabilityType));
    }

    size_t prediction = builder.addFunction(mapNFunction(classifier), input, supervision);

    prediction = builder.addFunction(new AddConnectivityPatternBiasLearnableFunction(), prediction, supervision);
    builder.addFunction(mapNFunction(signedScalarToProbabilityFunction()), prediction);
  }

protected:
  friend class ConnectivityPatternClassifierClass;

  FunctionPtr classifier;
  bool isSymmetricMatrix;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONNECTIVITY_PATTERN_CLASSIFIER_H_