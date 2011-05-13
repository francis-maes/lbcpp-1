/*-----------------------------------------.---------------------------------.
| Filename: RigidBodyGeneralMoverSampler.h | RigidBodyGeneralMoverSampler    |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 1 mai 2011  16:29:04           |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_
# define LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_

# include "precompiled.h"
# include "../Sampler.h"
# include "../ProteinMover/RigidBodyGeneralMover.h"
# include "DualResidueSampler.h"

namespace lbcpp
{

class RigidBodyGeneralMoverSampler;
typedef ReferenceCountedObjectPtr<RigidBodyGeneralMoverSampler> RigidBodyGeneralMoverSamplerPtr;

class RigidBodyGeneralMoverSampler : public CompositeSampler
{
public:
  RigidBodyGeneralMoverSampler()
    : CompositeSampler(3), numResidue(0)
  {
  }

  RigidBodyGeneralMoverSampler(size_t numResidue, double meanMagnitude, double stdMagnitude,
      double meanAmplitude, double stdAmplitude)
    : CompositeSampler(3), numResidue(numResidue)
  {
    // select residue
    sons[0] = Variable(new DualResidueSampler(numResidue, 2));
    // select magnitude
    sons[1] = Variable(new GaussianContinuousSampler(meanMagnitude, stdMagnitude));
    // select amplitude
    sons[2] = Variable(new GaussianContinuousSampler(meanAmplitude, stdAmplitude));
  }

  RigidBodyGeneralMoverSampler(const RigidBodyGeneralMoverSampler& sampler)
    : CompositeSampler(3), numResidue(sampler.numResidue)
  {
    sons[0] = Variable(new DualResidueSampler(
        *sampler.sons[0].getObjectAndCast<DualResidueSampler> ()));
    sons[1] = Variable(new GaussianContinuousSampler(*sampler.sons[1].getObjectAndCast<
        GaussianContinuousSampler> ()));
    sons[2] = Variable(new GaussianContinuousSampler(*sampler.sons[2].getObjectAndCast<
        GaussianContinuousSampler> ()));
  }

  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random,
      const Variable* inputs = NULL) const
  {
    MatrixPtr residues = sons[0].getObjectAndCast<DualResidueSampler> ()->sample(context, random,
        inputs).getObjectAndCast<Matrix> ();
    size_t firstResidue = (size_t)(residues->getElement(0, 0).getDouble());
    size_t secondResidue = (size_t)(residues->getElement(1, 0).getDouble());

    double magnitude = sons[1].getObjectAndCast<GaussianContinuousSampler> ()->sample(context,
        random, inputs).getDouble();
    double amplitude = sons[2].getObjectAndCast<GaussianContinuousSampler> ()->sample(context,
        random, inputs).getDouble();
    RigidBodyGeneralMoverPtr mover = new RigidBodyGeneralMover(firstResidue, secondResidue,
        magnitude, amplitude);
    return Variable(mover);
  }

  /**
   * dataset = first : RigidBodyGeneralMoverPtr observed
   *           second : not yet used
   */
  virtual void learn(ExecutionContext& context, const RandomGeneratorPtr& random, const std::vector<
      std::pair<Variable, Variable> >& dataset)
  {
    if (dataset.size() < 2)
      return;

    std::vector<std::pair<Variable, Variable> > datasetResidues;
    std::vector<std::pair<Variable, Variable> > datasetMagnitude;
    std::vector<std::pair<Variable, Variable> > datasetAmplitude;
    for (size_t i = 0; i < dataset.size(); i++)
    {
      RigidBodyGeneralMoverPtr mover = dataset[i].first.getObjectAndCast<RigidBodyGeneralMover> ();
      MatrixPtr tempResidue = new DoubleMatrix(2, 1);
      tempResidue->setElement(0, 0, Variable((double)mover->getIndexResidueOne()));
      tempResidue->setElement(1, 0, Variable((double)mover->getIndexResidueTwo()));
      datasetResidues.push_back(std::pair<Variable, Variable>(Variable(tempResidue), Variable()));
      datasetMagnitude.push_back(std::pair<Variable, Variable>(Variable(mover->getMagnitude()),
          Variable()));
      datasetAmplitude.push_back(std::pair<Variable, Variable>(Variable(mover->getAmplitude()),
          Variable()));
    }
    sons[0].getObjectAndCast<DualResidueSampler> ()->learn(context, random, datasetResidues);
    sons[1].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetMagnitude);
    sons[2].getObjectAndCast<GaussianContinuousSampler> ()->learn(context, random, datasetAmplitude);
  }

protected:
  friend class RigidBodyGeneralMoverSamplerClass;
  size_t numResidue;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_RIGID_BODY_GENERAL_MOVER_SAMPLER_H_