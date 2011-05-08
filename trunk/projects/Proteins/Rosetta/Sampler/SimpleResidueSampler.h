/*-----------------------------------------.---------------------------------.
| Filename: SimpleResidueSampler.h         | SimpleResidueSampler            |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 7 mai 2011  15:04:24           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "ParzenContinuousSampler.h"

namespace lbcpp
{

class SimpleResidueSampler;
typedef ReferenceCountedObjectPtr<SimpleResidueSampler> SimpleResidueSamplerPtr;

class SimpleResidueSampler: public CompositeSampler
{
public:
  SimpleResidueSampler() :
    CompositeSampler(), numResidues(1), residuesDeviation(0)
  {
  }

  SimpleResidueSampler(int numResidues, int residuesDeviation = 0) :
    CompositeSampler(1), numResidues(numResidues), residuesDeviation(residuesDeviation)
  {
    ParzenContinuousSamplerPtr temp = new ParzenContinuousSampler(0.0001, -1, 2, 10.0
        / (double)numResidues, 0.5, 0.25);
    sons[0] = temp;
  }

  SimpleResidueSampler(const SimpleResidueSampler& sampler) :
    CompositeSampler(1), numResidues(sampler.numResidues), residuesDeviation(
        sampler.residuesDeviation)
  {
    ParzenContinuousSamplerPtr temp = new ParzenContinuousSampler(
        (*(sampler.sons[0].getObjectAndCast<ParzenContinuousSampler> ())));
    sons[0] = temp;
  }

  ~SimpleResidueSampler()
  {
  }

  SamplerPtr clone()
  {
    SimpleResidueSamplerPtr temp = new SimpleResidueSampler(*this);
    return temp;
  }

  Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    double rand = std::abs(
        sons[0].getObjectAndCast<Sampler> ()->sample(context, random, NULL).getDouble());
    rand = rand > 1 ? 2 - rand : rand;

    int residue = std::floor(rand * numResidues);
    if (residue == numResidues)
      residue--;
    return Variable(residue);
  }

  /**
   * dataset = first : a Variable of integer type containing the residue observed.
   *           second : not yet used.
   */
  void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() == 0)
      return;

    std::vector<std::pair<Variable, Variable> > data;

    double varianceIncrement = (double)residuesDeviation / (double)numResidues;
    for (int i = 0; i < dataset.size(); i++)
    {
      int res = dataset[i].first.getInteger();
      double value = (double)res / (double)numResidues;
      value = std::abs(value + varianceIncrement * random->sampleDoubleFromGaussian(0, 1));
      value = value > 1 ? 2 - value : value;
      data.push_back(std::pair<Variable, Variable>(Variable(value), Variable()));
    }

    sons[0].getObjectAndCast<Sampler> ()->learn(context, random, data);
  }

protected:
  friend class SimpleResidueSamplerClass;
  int numResidues;
  int residuesDeviation;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_SIMPLE_RESIDUE_SAMPLER_H_