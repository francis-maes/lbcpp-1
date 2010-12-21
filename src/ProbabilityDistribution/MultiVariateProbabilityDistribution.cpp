/*-----------------------------------------.---------------------------------.
| Filename: MultiVariateProbabilityDis..cpp| MultiVariate Probability        |
| Author  : Francis Maes                   |  Distributions                  |
| Started : 22/12/2010 00:36               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/ProbabilityDistribution/MultiVariateProbabilityDistribution.h>
using namespace lbcpp;

/*
** IndependentMultiVariateProbabilityDistribution
*/
IndependentMultiVariateProbabilityDistribution::IndependentMultiVariateProbabilityDistribution(ClassPtr variableClass)
{
}

double IndependentMultiVariateProbabilityDistribution::computeEntropy() const
  {jassert(false); return 0.0;} // not implemented

double IndependentMultiVariateProbabilityDistribution::compute(ExecutionContext& context, const Variable& value) const
{
  const ObjectPtr& object = value.getObject();
  jassert(object->getNumVariables() == distributions.size());
  double res = 1.0;
  for (size_t i = 0; i < distributions.size(); ++i)
    res *= distributions[i]->compute(context, object->getVariable(i));
  return res;
}

Variable IndependentMultiVariateProbabilityDistribution::sample(RandomGeneratorPtr random) const
{
  ClassPtr vclass = getVariableClass();
  ObjectPtr res = Object::create(vclass);
  for (size_t i = 0; i < distributions.size(); ++i)
    res->setVariable(defaultExecutionContext(), i, distributions[i]->sample(random));
  return res;
}
