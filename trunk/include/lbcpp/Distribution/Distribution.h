/*-----------------------------------------.---------------------------------.
| Filename: Distribution.h      | Probability Distributions       |
| Author  : Francis Maes                   |                                 |
| Started : 06/07/2010 15:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
# define LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_

# include "../Data/RandomGenerator.h"

namespace lbcpp
{

class Distribution : public Object
{
public:
  Distribution(ClassPtr thisClass) : Object(thisClass) {}
  Distribution() : Object() {}
  
  virtual TypePtr getElementsType() const = 0;

  virtual double computeEntropy() const = 0;
  virtual double computeProbability(const Variable& value) const = 0;
  virtual Variable sample(RandomGeneratorPtr random) const = 0;
  virtual Variable sampleBest(RandomGeneratorPtr random) const = 0;
};

typedef ReferenceCountedObjectPtr<Distribution> DistributionPtr;

extern ClassPtr distributionClass(TypePtr elementsType);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_PROBABILITY_DISTRIBUTION_H_
