/*-----------------------------------------.---------------------------------.
| Filename: IterativeBracketingOptimizer.h | Iterative Bracketing Optimizer  |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 23:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_
# define LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_

# include <lbcpp/Optimizer/Optimizer.h>
# include <lbcpp/ProbabilityDistribution/MultiVariateProbabilityDistribution.h>

namespace lbcpp
{

class IterativeBracketingOptimizer : public Optimizer
{
public:
  virtual Variable computeFunction(ExecutionContext& context, const Variable& i) const
  {
    const OptimizerInputPtr& input = i.getObjectAndCast<OptimizerInput>();
    const ObjectiveFunctionPtr& objective = input->getObjective();
    MultiVariateProbabilityDistributionPtr apriori = input->getAprioriDistribution().dynamicCast<MultiVariateProbabilityDistribution>();
    jassert(apriori);

    // FIXME !

    return Variable();
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_ITERATIVE_BRACKETING_H_
