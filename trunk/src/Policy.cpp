/*-----------------------------------------.---------------------------------.
| Filename: Policy.cpp                     | Policies                        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 20:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include <cralgo/cralgo.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

inline impl::DynamicToStaticPolicy dynamicToStatic(const Policy* policy)
  {return impl::dynamicToStatic(PolicyPtr(const_cast<Policy* >(policy)));}

inline impl::DynamicToStaticPolicy dynamicToStatic(const PolicyPtr policy)
  {return impl::dynamicToStatic(policy);}

// 

PolicyPtr Policy::createRandom()
  {return impl::staticToDynamic(impl::RandomPolicy());}

PolicyPtr Policy::createGreedy(ActionValueFunctionPtr actionValues)
  {return impl::staticToDynamic(impl::GreedyPolicy(actionValues));}

PolicyPtr Policy::createQLearning(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::QLearningPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount, false));
}

PolicyPtr Policy::createSarsaZero(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::QLearningPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount, true));
}

PolicyPtr Policy::createMonteCarloControl(PolicyPtr explorationPolicy, RegressorPtr regressor, double discount)
{
  return impl::staticToDynamic(impl::MonteCarloControlPolicy<impl::DynamicToStaticPolicy>(
        dynamicToStatic(explorationPolicy), regressor, discount));
}

PolicyPtr Policy::createClassificationExampleCreator(PolicyPtr explorationPolicy,
          ClassifierPtr classifier, ActionValueFunctionPtr supervisor)
{
  return impl::staticToDynamic(impl::ClassificationExampleCreatorPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(explorationPolicy), classifier, supervisor));
}


PolicyPtr Policy::createRankingExampleCreator(PolicyPtr explorationPolicy,
          RankerPtr ranker, ActionValueFunctionPtr supervisor)
{
  return impl::staticToDynamic(impl::RankingExampleCreatorPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(explorationPolicy), ranker, supervisor));
}


PolicyPtr Policy::verbose(std::ostream& ostr, size_t verbosity) const
{
  return impl::staticToDynamic(impl::VerbosePolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this), ostr, verbosity));
}

PolicyPtr Policy::epsilonGreedy(IterationFunctionPtr epsilon) const
  {return impl::staticToDynamic(impl::EpsilonGreedyPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this), epsilon));}
  
PolicyPtr Policy::addComputeStatistics() const
  {return impl::staticToDynamic(impl::ComputeStatisticsPolicy<impl::DynamicToStaticPolicy>(
            dynamicToStatic(this)));}
