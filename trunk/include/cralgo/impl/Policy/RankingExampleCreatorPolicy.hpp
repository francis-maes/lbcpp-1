/*-----------------------------------------.---------------------------------.
| Filename: RankingExampleCreatorPolicy.hpp| A policy that creates           |
| Author  : Francis Maes                   |   ranking examples              |
| Started : 13/03/2009 18:30               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_
# define CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_

# include "PolicyStatic.hpp"
# include "../../LearningMachine.h"

namespace cralgo {
namespace impl {

template<class DecoratedType>
struct RankingExampleCreatorPolicy
  : public EpisodicDecoratorPolicy<RankingExampleCreatorPolicy<DecoratedType> , DecoratedType>
{
  typedef RankingExampleCreatorPolicy<DecoratedType> ExactType;
  typedef EpisodicDecoratorPolicy<ExactType, DecoratedType> BaseClass;
  
  RankingExampleCreatorPolicy(const DecoratedType& explorationPolicy, RankerPtr ranker, ActionValueFunctionPtr supervisor = ActionValueFunctionPtr())
    : BaseClass(explorationPolicy), ranker(ranker), supervisor(supervisor) {}
    
  RankerPtr ranker;
  ActionValueFunctionPtr supervisor;

  void episodeEnter(CRAlgorithmPtr crAlgorithm)
    {ranker->trainStochasticBegin();}

  VariablePtr policyChoose(ChoosePtr choose)
  {
    std::vector<FeatureGeneratorPtr> alternatives;
    choose->computeActionFeatures(alternatives);

    std::vector<double> costs;
    choose->computeActionValues(costs);
    
    // transform action values into (regret) costs
    double bestValue = -DBL_MAX;
    for (size_t i = 0; i < costs.size(); ++i)
      if (costs[i] > bestValue)
        bestValue = costs[i];
    for (size_t i = 0; i < costs.size(); ++i)
      costs[i] = bestValue - costs[i];
    
    ranker->trainStochasticExample(RankingExample(alternatives, costs));
    return BaseClass::policyChoose(choose);
  }
  
  void episodeLeave()
    {ranker->trainStochasticEnd();}
};

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_POLICY_RANKING_EXAMPLE_CREATOR_H_
