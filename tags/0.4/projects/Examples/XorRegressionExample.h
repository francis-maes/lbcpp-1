/*-----------------------------------------.---------------------------------.
| Filename: XorRegressionExample.h         | Illustrates a simple regression |
| Author  : Francis Maes                   |                                 |
| Started : 18/10/2010 19:16               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXAMPLES_XOR_REGRESSION_H_
# define LBCPP_EXAMPLES_XOR_REGRESSION_H_

# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Function/StoppingCriterion.h>
# include <lbcpp/Function/Evaluator.h>

namespace lbcpp
{

class XorPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return pairClass(doubleType, doubleType);}

  virtual void computeOutputType()
  {
    addOutputVariable(T("unit"), doubleType);
    addOutputVariable(T("x1"), doubleType);
    addOutputVariable(T("x2"), doubleType);
    addOutputVariable(T("x1.x2"), doubleType);
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    const PairPtr& pair = input.getObjectAndCast<Pair>();
    double x1 = pair->getFirst().getDouble();
    double x2 = pair->getSecond().getDouble();
    callback->sense(0, 1.0);
    callback->sense(1, x1);
    callback->sense(2, x2);
    callback->sense(3, x1 * x2);
  }
};

class XorRegressionExample : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
   // create linear regressor
    PerceptionPtr perception = new XorPerception();
    InferenceOnlineLearnerPtr learner = gradientDescentOnlineLearner(
            perStep, constantIterationFunction(0.1), true, // learning steps
            never, ScalarObjectFunctionPtr()); // regularizer
    learner->getLastLearner()->setNextLearner(stoppingCriterionOnlineLearner(maxIterationsStoppingCriterion(100), true)); // stopping criterion
    NumericalSupervisedInferencePtr regressor = squareRegressionInference(T("XOR-Regressor"), perception);
    regressor->setStochasticLearner(learner);
   
    // make training set
    TypePtr inputType = perception->getInputType();
    VectorPtr trainingSet = vector(pairClass(inputType, doubleType));
    trainingSet->append(Variable::pair(Variable::pair(0.0, 0.0), 1.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 0.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(0.0, 1.0), 0.0));
    trainingSet->append(Variable::pair(Variable::pair(1.0, 1.0), 1.0));

    // create context and train
    regressor->train(context, trainingSet, ContainerPtr(), T("Training"));

    // evaluate
    EvaluatorPtr evaluator = regressionErrorEvaluator(T("XOR-error"));
    regressor->evaluate(context, trainingSet, evaluator, T("Evaluating"));
    //std::cout << "Evaluation: " << evaluator->toString() << std::endl;

    // test evaluator on one example
    Variable myPrediction = regressor->computeFunction(context, Variable::pair(1.0, 0.0));
    //std::cout << "MyPrediction: " << myPrediction << std::endl;
    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_EXAMPLES_XOR_REGRESSION_H_