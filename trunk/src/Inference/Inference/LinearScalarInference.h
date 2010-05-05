/*-----------------------------------------.---------------------------------.
| Filename: LinearScalarInference.h        | Linear Scalar Inference         |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 14:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_LINEAR_SCALAR_H_
# define LBCPP_INFERENCE_LINEAR_SCALAR_H_

# include <lbcpp/Inference/InferenceBaseClasses.h>
# include <lbcpp/FeatureGenerator/FeatureGenerator.h>

namespace lbcpp
{

// Input: Features
// Output: Scalar
// Supervision: ScalarFunction
class LinearScalarInference : public LearnableAtomicInference
{
public:
  LinearScalarInference(const String& name)
    : LearnableAtomicInference(name), dotProductCache(NULL) {}

  virtual ~LinearScalarInference()
    {clearDotProductCache();}

  virtual void beginRunSession()
    {clearDotProductCache(); dotProductCache = new FeatureGenerator::DotProductCache();}

  virtual void endRunSession()
    {clearDotProductCache();}
  
  virtual void validateParametersChange()
    {clearDotProductCache();}

  void clearDotProductCache()
  {
    if (dotProductCache)
    {
      delete dotProductCache;
      dotProductCache = NULL;
    }
  }

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    FeatureGeneratorPtr features = input.dynamicCast<FeatureGenerator>();
    jassert(features);
    DenseVectorPtr parameters = getOrCreateParameters(features->getDictionary());
    return new Scalar(features->dotProduct(parameters, dotProductCache));
  }

private:
  FeatureGenerator::DotProductCache* dotProductCache;
};

typedef ReferenceCountedObjectPtr<LinearScalarInference> LinearScalarInferencePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_LINEAR_SCALAR_H_
