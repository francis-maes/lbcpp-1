/*-----------------------------------------.---------------------------------.
| Filename: LearnableFunction.h            | Base classes for Learnable      |
| Author  : Francis Maes                   |  Functions                      |
| Started : 15/02/2011 18:59               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LEARNING_LEARNABLE_FUNCTION_H_
# define LBCPP_LEARNING_LEARNABLE_FUNCTION_H_

# include <lbcpp/Function/Function.h>
# include "OnlineLearner.h"
# include "BatchLearner.h"

namespace lbcpp
{

class LearnableFunction : public Function
{
public:
  const ClassPtr& getParametersClass() const
    {return parametersClass;}

  const ObjectPtr& getParameters() const
    {return parameters;}

  ObjectPtr& getParameters()
    {return parameters;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LearnableFunctionClass;

  ObjectPtr parameters;
  ClassPtr parametersClass;
};

class LearnerParameters : public Object
{
public:
  virtual BatchLearnerPtr createBatchLearner() const
    {return BatchLearnerPtr();}

  virtual OnlineLearnerPtr createOnlineLearner() const
    {return OnlineLearnerPtr();}
};

typedef ReferenceCountedObjectPtr<LearnerParameters> LearnerParametersPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LEARNING_LEARNABLE_FUNCTION_H_
