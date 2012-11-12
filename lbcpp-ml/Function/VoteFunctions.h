/*-----------------------------------------.---------------------------------.
| Filename: VoteFunctions.h                | Voting Functions                |
| Author  : Francis Maes                   |                                 |
| Started : 22/01/2012 21:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_FUNCTION_VOTE_H_
# define LBCPP_ML_FUNCTION_VOTE_H_

# include <lbcpp-ml/Function.h>
# include <lbcpp-ml/Expression.h>

namespace lbcpp
{

class VoteFunction : public Function
{
public:
  VoteFunction(TypePtr outputType)
    : outputType(outputType) {}
  VoteFunction() {}

  virtual size_t getNumInputs() const
    {return 1;}

  virtual bool doAcceptInputType(size_t index, const TypePtr& type) const
    {return type == newBooleanClass || type == newProbabilityClass;}

  virtual TypePtr initialize(const TypePtr* inputTypes)
    {return outputType;}

  virtual ObjectPtr computeVote(double input) const = 0;

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (!inputs[0])
      return ObjectPtr();
    return computeVote(NewDouble::get(inputs[0]));
  }

  virtual DataVectorPtr compute(ExecutionContext& context, const std::vector<DataVectorPtr>& inputs, TypePtr outputType) const
  {
    DataVectorPtr weakPredictions = inputs[0];
    if (weakPredictions->getElementsType() == newBooleanClass)
    {
      ObjectPtr negativeVote = computeVote(0.0);
      ObjectPtr positiveVote = computeVote(1.0);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (DataVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        unsigned char c = it.getRawBoolean();
        if (c < 2)
          res->setElement(index, c == 1 ? positiveVote : negativeVote);
        ++index;
      }
      return new DataVector(weakPredictions->getIndices(), res);
    }
    else
    {
      jassert(weakPredictions->getElementsType() == newProbabilityClass);
      VectorPtr res = lbcpp::vector(outputType, weakPredictions->size());
      size_t index = 0;
      for (DataVector::const_iterator it = weakPredictions->begin(); it != weakPredictions->end(); ++it)
      {
        double d = it.getRawDouble();
        if (d != doubleMissingValue)
          res->setElement(index, computeVote(d));
        ++index;
      }
      return new DataVector(weakPredictions->getIndices(), res);
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  TypePtr outputType;
};

class ScalarVoteFunction : public VoteFunction
{
public:
  ScalarVoteFunction(double vote = 0.0)
    : VoteFunction(newDoubleClass), vote(vote) {}

  virtual ObjectPtr computeVote(double input) const
    {return new NewDouble((input * 2 - 1) * vote);}

  virtual ObjectPtr compute(ExecutionContext& context, const ObjectPtr* inputs) const
  {
    if (inputs[0])
      return computeVote(NewDouble::get(inputs[0]));
    else
      return new NewDouble();
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + ObjectPtr(new NewDouble(vote))->toShortString() + ")";}

protected:
  friend class ScalarVoteFunctionClass;

  double vote;
};

class VectorVoteFunction : public VoteFunction
{
public:
  VectorVoteFunction(const DenseDoubleVectorPtr& vote)
    : VoteFunction(vote->getClass()), vote(vote) {}
  VectorVoteFunction() {}

  virtual ObjectPtr computeVote(double input) const
  {
    DenseDoubleVectorPtr res = vote->cloneAndCast<DenseDoubleVector>();
    res->multiplyByScalar(input * 2 - 1);
    return res;
  }

  virtual String makeNodeName(const std::vector<ExpressionPtr>& inputs) const
    {return "vote(" + inputs[0]->toShortString() + ", " + vote->toShortString() + ")";}

protected:
  friend class VectorVoteFunctionClass;

  DenseDoubleVectorPtr vote;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_FUNCTION_VOTE_H_
