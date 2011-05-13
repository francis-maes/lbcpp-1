/*-----------------------------------------.---------------------------------.
| Filename: Sampler.h                      | Sampler base class              |
| Author  : Francis Maes                   |                                 |
| Started : 13/05/2011 16:28               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_SAMPLER_H_
# define LBCPP_SAMPLER_H_

# include "../Data/DoubleVector.h"
# include "../Data/RandomGenerator.h"
# include "../Core/Variable.h"
# include "../Core/Vector.h"

namespace lbcpp
{ 

/*
** Sampler Base Class
*/
class Sampler : public Object
{
public:
  virtual Variable sample(ExecutionContext& context, const RandomGeneratorPtr& random, const Variable* inputs = NULL) const = 0;

  virtual void learn(ExecutionContext& context, const std::vector<Variable>& dataset) = 0;

  lbcpp_UseDebuggingNewOperator

protected:
  friend class SamplerClass;
};
typedef ReferenceCountedObjectPtr<Sampler> SamplerPtr;

/*
** Continuous Sampler
*/
class ContinuousSampler : public Sampler
{
public:
  virtual double computeExpectation(const Variable* inputs = NULL) const
    {jassert(false); return 0.0;}

  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<ContinuousSampler> ContinuousSamplerPtr;

extern ContinuousSamplerPtr gaussianSampler(double mean = 0.0, double stddev = 1.0);

/*
** Discrete Sampler
*/
class DiscreteSampler : public Sampler
{
public:
  lbcpp_UseDebuggingNewOperator
};
typedef ReferenceCountedObjectPtr<DiscreteSampler> DiscreteSamplerPtr;

/*
** Composite Sampler
*/
class CompositeSampler : public Sampler
{
public:
  CompositeSampler(size_t numSamplers)
    : samplers(numSamplers) {}
  CompositeSampler(const std::vector<SamplerPtr>& samplers)
    : samplers(samplers) {}
  CompositeSampler() {}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class CompositeSamplerClass;

  std::vector<SamplerPtr> samplers;
};

typedef ReferenceCountedObjectPtr<CompositeSampler> CompositeSamplerPtr;

extern CompositeSamplerPtr objectCompositeSampler(ClassPtr objectClass, const std::vector<SamplerPtr>& variableSamplers);

extern CompositeSamplerPtr independentDenseDoubleVectorSampler(EnumerationPtr elementsEnumeration, SamplerPtr elementSamplerModel);
extern CompositeSamplerPtr independentDenseDoubleVectorSampler(size_t numElements, SamplerPtr elementSamplerModel);

extern CompositeSamplerPtr mixtureSampler(const DenseDoubleVectorPtr& probabilities, const std::vector<SamplerPtr>& samplers);

}; /* namespace lbcpp */

#endif // !LBCPP_SAMPLER_H_
