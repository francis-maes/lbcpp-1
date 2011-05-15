/*-----------------------------------------.---------------------------------.
| Filename: SequentialOptimizer.h          | SequentialOptimizer             |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 29 avr. 2011  22:51:54         |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_
# define LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_

# include "precompiled.h"
# include "../ProteinOptimizer.h"
# include "SimulatedAnnealingOptimizer.h"
# include "../Sampler.h"
# include "../ProteinMover.h"

namespace lbcpp
{
class ProteinSequentialOptimizer;
typedef ReferenceCountedObjectPtr<ProteinSequentialOptimizer> ProteinSequentialOptimizerPtr;

class ProteinSequentialOptimizer: public ProteinOptimizer
{
public:
  ProteinSequentialOptimizer() :
    ProteinOptimizer()
  {
  }

  ProteinSequentialOptimizer(String name) :
    ProteinOptimizer(name, -1, -1, juce::File())
  {
    if (name.isEmpty())
      this->name = String("Default");
    else
      this->name = name;
  }

  core::pose::PoseOP apply(core::pose::PoseOP& pose, ExecutionContext& context,
      RandomGeneratorPtr& random)
  {
    SamplerPtr sampler;
    return sequentialOptimization(pose, sampler, context, random);
  }

  core::pose::PoseOP apply(core::pose::PoseOP& pose, SamplerPtr& sampler,
      ExecutionContext& context, RandomGeneratorPtr& random)
  {
    return sequentialOptimization(pose, sampler, context, random);
  }

  /*
   * Performs sequential simulation on the pose object. This function adds a residue
   * at each iteration and then performs optimization on the resulting protein object.
   * The purpose is to fold the protein as it was being cronstructed.
   * @param pose the initial conformation
   * @param mover pointer to a mover that perturbs the object at each iteration.
   * @return the new conformation
   */
  core::pose::PoseOP sequentialOptimization(core::pose::PoseOP& pose,
      SamplerPtr& sampler, ExecutionContext& context, RandomGeneratorPtr& random)
  {
    core::pose::PoseOP acc = new core::pose::Pose();

    double initialTemperature = 4.0;
    double finalTemperature = 0.01;
    int numberDecreasingSteps = 100;
    int timesReinitialization = 5;
    int maxSteps = 0;
    int factor = 5000;
    bool store = getVerbosity();
    setVerbosity(false);

    for (size_t i = 1; i <= pose->n_residue(); i++)
    {
      maxSteps = (int)std::max((int)((factor * log((double)i)) + 1), numberDecreasingSteps);

      ProteinSimulatedAnnealingOptimizerPtr optimizer = new ProteinSimulatedAnnealingOptimizer(
          initialTemperature, finalTemperature, numberDecreasingSteps, maxSteps,
          timesReinitialization, name);

      acc->append_residue_by_bond(pose->residue(i), true);

      // phipsisampler
      CompositeSamplerPtr ppsres = simpleResidueSampler(pose->n_residue());
      ContinuousSamplerPtr ppsphi = gaussianSampler(0, 25);
      ContinuousSamplerPtr ppspsi = gaussianSampler(0, 25);
      CompositeSamplerPtr phipsi = objectCompositeSampler(phiPsiMoverClass, ppsres, ppsphi, ppspsi);
      // shearsampler
      CompositeSamplerPtr sres = simpleResidueSampler(pose->n_residue());
      ContinuousSamplerPtr sphi = gaussianSampler(0, 25);
      ContinuousSamplerPtr spsi = gaussianSampler(0, 25);
      CompositeSamplerPtr shear = objectCompositeSampler(shearMoverClass, sres, sphi, spsi);
      // rigidbody
      CompositeSamplerPtr rbres = residuePairSampler(pose->n_residue());
      ContinuousSamplerPtr rbmagn = gaussianSampler(0.5, 0.25);
      ContinuousSamplerPtr rbamp = gaussianSampler(0, 25);
      CompositeSamplerPtr rigidbody = objectCompositeSampler(rigidBodyMoverClass, rbres, rbmagn,
          rbamp);
      std::vector<SamplerPtr> samplers;
      samplers.push_back(phipsi);
      samplers.push_back(shear);
      samplers.push_back(rigidbody);
      ClassPtr actionClass = denseDoubleVectorClass(positiveIntegerEnumerationEnumeration);
      DenseDoubleVectorPtr proba = new DenseDoubleVector(actionClass, 3, 0.33);
      SamplerPtr samp = mixtureSampler(proba, samplers).staticCast<Sampler>();

      core::pose::PoseOP tempPose;
//      core::pose::PoseOP tempPose = optimizer->simulatedAnnealingOptimization(acc, samp.staticCast<
//          Sampler> (), context, random, initialTemperature, finalTemperature,
//          numberDecreasingSteps, maxSteps, timesReinitialization);

      if (tempPose.get() == NULL)
      {
        return NULL;
      }
      (*acc) = (*tempPose);
    }
    std::cout << "test 5" << std::endl;
    setVerbosity(store);
    return acc;
  }
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_PROTEIN_SEQUENTIAL_OPTIMIZER_H_
