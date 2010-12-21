/*-----------------------------------------.---------------------------------.
| Filename: SharedParallelVectorInference.h| Infer a Vector by inferring each|
| Author  : Francis Maes                   |  element parallely with a       |
| Started : 15/07/2010 14:41               |  shared elements sub-inference  |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_
# define LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_

# include <lbcpp/Inference/ParallelInference.h>
# include <lbcpp/Core/Vector.h>
# include <lbcpp/Perception/Perception.h>
# include <lbcpp/Distribution/DiscreteDistribution.h>
# include <lbcpp/Data/RandomGenerator.h>
# include <lbcpp/Core/Pair.h>

namespace lbcpp
{

class SharedParallelVectorInference : public SharedParallelInference
{
public:
  SharedParallelVectorInference(const String& name, FunctionPtr sizeFunction, InferencePtr elementInference)
    : SharedParallelInference(name, elementInference), sizeFunction(sizeFunction) {}
  SharedParallelVectorInference() {}

  virtual bool useMultiThreading() const
    {return false;}

  virtual TypePtr getInputType() const
    {return getSubInference()->getInputType()->getTemplateArgument(0);}

  virtual TypePtr getSupervisionType() const
    {return vectorClass(subInference->getSupervisionType());}

  TypePtr getOutputElementsType(TypePtr inputType) const
    {return subInference->getOutputType(pairClass(inputType, positiveIntegerType));}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return vectorClass(getOutputElementsType(inputType));}

  virtual ParallelInferenceStatePtr prepareInference(ExecutionContext& context, const Variable& input, const Variable& supervision) const
  {
    size_t n = (size_t)sizeFunction->computeFunction(context, input).getInteger();
    
    VectorPtr supervisionVector = supervision.exists() ? supervision.getObjectAndCast<Vector>(context) : VectorPtr();

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    TypePtr subInputType = pairClass(input.getType(), positiveIntegerType);
    res->reserve(n);
    for (size_t i = 0; i < n; ++i)
    {
      Variable elementSupervision;
      if (supervisionVector)
        elementSupervision = supervisionVector->getElement(i);
      res->addSubInference(subInference, Variable::pair(input, i, subInputType), elementSupervision);
    }
    return res;
  }

  virtual Variable finalizeInference(ExecutionContext& context, ParallelInferenceStatePtr state) const
  {
    size_t n = state->getNumSubInferences();
    VectorPtr res = vector(getOutputElementsType(state->getInput().getType()), n);
    bool atLeastOnePrediction = false;
    for (size_t i = 0; i < n; ++i)
    {
      Variable result = state->getSubOutput(i);
      if (result.exists())
      {
        if (result.isObject())
        {
          EnumerationDistributionPtr distribution = result.dynamicCast<EnumerationDistribution>();
          if (distribution)
            result = distribution->sample(RandomGenerator::getInstance());
        }
        atLeastOnePrediction = true;
        res->setElement(i, result);
      }
    }
    return atLeastOnePrediction ? res : Variable::missingValue(res->getClass());
  }

protected:
  friend class SharedParallelVectorInferenceClass;

  FunctionPtr sizeFunction;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_REDUCTION_SHARED_PARALLEL_VECTOR_H_
