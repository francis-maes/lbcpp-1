/*-----------------------------------------.---------------------------------.
| Filename: Numerical.h                    | Numerical Learning              |
| Author  : Francis Maes                   |                                 |
| Started : 15/02/2011 19:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_NUMERICAL_H_
# define LBCPP_LEARNING_NUMERICAL_H_

# include "LearnableFunction.h"
# include "../Core/Frame.h"
# include "../Data/DoubleVector.h"
# include "../Function/StoppingCriterion.h"
# include "../Function/IterationFunction.h"

namespace lbcpp
{

class NumericalLearnableFunction : public LearnableFunction
{
public:
  const DoubleVectorPtr& getParameters() const
    {return parameters.staticCast<DoubleVector>();}

  DoubleVectorPtr& getParameters()
    {return *(DoubleVectorPtr* )&parameters;}

  // returns false if no supervision is available
  virtual bool computeAndAddGradient(const Variable* inputs, const Variable& output, double& exampleLossValue, DoubleVectorPtr& target, double weight) const = 0;

  lbcpp_UseDebuggingNewOperator
};

typedef ReferenceCountedObjectPtr<NumericalLearnableFunction> NumericalLearnableFunctionPtr;

extern NumericalLearnableFunctionPtr linearLearnableFunction();

extern OnlineLearnerPtr stochasticGDOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate = true);
extern OnlineLearnerPtr perEpisodeGDOnlineLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate = true);

class StochasticGDParameters : public LearnerParameters
{
public:
  StochasticGDParameters( IterationFunctionPtr learningRate = constantIterationFunction(0.1),
                          StoppingCriterionPtr stoppingCriterion = maxIterationsWithoutImprovementStoppingCriterion(10),
                          size_t maxIterations = 100,
                          EvaluatorPtr evaluator = EvaluatorPtr(),
                          bool doPerEpisodeUpdates = false,
                          bool normalizeLearningRate = true,
                          bool restoreBestParameters = true,
                          bool randomizeExamples = true);

  virtual BatchLearnerPtr createBatchLearner() const;
  virtual OnlineLearnerPtr createOnlineLearner() const;

  const EvaluatorPtr& getEvaluator() const
    {return evaluator;}

  void setEvaluator(EvaluatorPtr evaluator)
    {this->evaluator = evaluator;}

protected:
  friend class StochasticGDParametersClass;

  IterationFunctionPtr learningRate;
  StoppingCriterionPtr stoppingCriterion;
  size_t maxIterations;
  EvaluatorPtr evaluator;
  bool doPerEpisodeUpdates;
  bool normalizeLearningRate;
  bool restoreBestParameters;
  bool randomizeExamples;
  // todo: regularizer
};

typedef ReferenceCountedObjectPtr<StochasticGDParameters> StochasticGDParametersPtr;

class SupervisedNumericalFunction : public FrameBasedFunction
{
public:
  SupervisedNumericalFunction(LearnerParametersPtr learnerParameters)
    : learnerParameters(learnerParameters) {}

  virtual size_t getNumRequiredInputs() const
    {return 2;}

  virtual String getOutputPostFix() const
    {return T("Prediction");}

protected:
  friend class SupervisedNumericalFunctionClass;

  LearnerParametersPtr learnerParameters;
};

typedef ReferenceCountedObjectPtr<SupervisedNumericalFunction> SupervisedNumericalFunctionPtr;

extern SupervisedNumericalFunctionPtr linearRegressor(LearnerParametersPtr parameters);


}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_NUMERICAL_H_
