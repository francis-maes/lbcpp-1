/*-----------------------------------------.---------------------------------.
| Filename: EvaluatorOnlineLearner.h       | Evaluator Online Learner        |
| Author  : Francis Maes                   |                                 |
| Started : 16/02/2011 15:37               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_
# define LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_

# include <lbcpp/Learning/OnlineLearner.h>
# include <lbcpp/Function/OldEvaluator.h>

namespace lbcpp
{

class EvaluatorOnlineLearner : public OnlineLearner
{
public:
  EvaluatorOnlineLearner(OldEvaluatorPtr evaluator = OldEvaluatorPtr())
    : evaluator(evaluator), context(NULL), trainingData(NULL), validationData(NULL) {}

  virtual void startLearning(ExecutionContext& context, const FunctionPtr& function, size_t maxIterations, const std::vector<ObjectPtr>& trainingData, const std::vector<ObjectPtr>& validationData)
  {
    this->context = &context;
    this->function = function;
    this->trainingData = trainingData.size() ? &trainingData : NULL;
    this->validationData = validationData.size() ? &validationData : NULL;
  }

  virtual bool finishLearningIteration(size_t iteration, double& objectiveValueToMinimize)
  {
    if (trainingData)
    {
      OldEvaluatorPtr evaluator = evaluate(*trainingData, T("Train"));
      objectiveValueToMinimize = -evaluator->getDefaultScore();
    }

    if (validationData)
    {
      OldEvaluatorPtr evaluator = evaluate(*trainingData, T("Validation"));
      objectiveValueToMinimize = -evaluator->getDefaultScore();
    }
    return false;
  }

  virtual void finishLearning()
    {function = FunctionPtr();}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class EvaluatorOnlineLearnerClass;

  OldEvaluatorPtr evaluator;

  ExecutionContext* context;
  FunctionPtr function;
  const std::vector<ObjectPtr>* trainingData;
  const std::vector<ObjectPtr>* validationData;

  OldEvaluatorPtr evaluate(const std::vector<ObjectPtr>& data, const String& name)
  {
    OldEvaluatorPtr evaluator = this->evaluator->cloneAndCast<OldEvaluator>();
    function->evaluate(*context, data, evaluator);

    std::vector< std::pair<String, double> > results;
    evaluator->getScores(results);
    for (size_t i = 0; i < results.size(); ++i)
      context->resultCallback(name + T(" ") + results[i].first, results[i].second);

    return evaluator;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_ONLINE_LEARNER_EVALUATOR_H_
