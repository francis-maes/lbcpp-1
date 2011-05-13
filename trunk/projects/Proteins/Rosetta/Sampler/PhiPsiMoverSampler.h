/*-----------------------------------------.---------------------------------.
| Filename: PhiPsiMoverSampler.h           | PhiPsiMoverSampler              |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:28:05           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/PhiPsiMover.h"
# include "SimpleResidueSampler.h"

namespace lbcpp
{

class PhiPsiMoverSampler;
typedef ReferenceCountedObjectPtr<PhiPsiMoverSampler> PhiPsiMoverSamplerPtr;

class PhiPsiMoverSampler : public CompositeSampler
{
public:
  PhiPsiMoverSampler()
    : CompositeSampler(3)
  {
  }

  PhiPsiMoverSampler(size_t numResidue, double meanPhi, double stdPhi, double meanPsi,
      double stdPsi)
    : CompositeSampler(3)
  {
    // select residue
    samplers[0] = new SimpleResidueSampler(numResidue, juce::jmax(1, (int)(0.02 * numResidue)));
    //select phi
    samplers[1] = gaussianSampler(meanPhi, stdPhi);
    // select psi
    samplers[2] = gaussianSampler(meanPsi, stdPsi);
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    size_t residue = samplers[0]->sample(context, random, inputs).getInteger();
    // TODO rajouter la dependance entre le residu et les angles
    double phi = samplers[1]->sample(context, random, inputs).getDouble();
    double psi = samplers[2]->sample(context, random, inputs).getDouble();
    PhiPsiMoverPtr mover = new PhiPsiMover(residue, phi, psi);
    return Variable(mover);
  }

  /**
   * dataset = first : PhiPsiMoverPtr observed
   *           second : not yet used
   */
  virtual void learn(ExecutionContext& context, const std::vector<std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() < 2)
      return;

    std::vector<std::pair<Variable, Variable> > datasetResidue;
    std::vector<std::pair<Variable, Variable> > datasetPhi;
    std::vector<std::pair<Variable, Variable> > datasetPsi;
    for (size_t i = 0; i < dataset.size(); i++)
    {
      PhiPsiMoverPtr mover = dataset[i].first.getObjectAndCast<PhiPsiMover> ();
      datasetResidue.push_back(std::pair<Variable, Variable>(Variable(mover->getResidueIndex()),
          Variable()));
      datasetPhi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPhi()), Variable()));
      datasetPsi.push_back(
          std::pair<Variable, Variable>(Variable(mover->getDeltaPsi()), Variable()));
    }
    samplers[0]->learn(context, datasetResidue);
    samplers[1]->learn(context, datasetPhi);
    samplers[2]->learn(context, datasetPsi);
  }

protected:
  friend class PhiPsiMoverSamplerClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PHI_PSI_MOVER_SAMPLER_H_