/*-----------------------------------------.---------------------------------.
| Filename: BinaryClassificationInference.h| Binary Classification Inference |
| Author  : Francis Maes                   |  classes                        |
| Started : 26/05/2010 16:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_
# define LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_

# include <lbcpp/Inference/DecoratorInference.h>

namespace lbcpp
{

class BinaryClassificationInference : public StaticDecoratorInference
{
public:
  BinaryClassificationInference(const String& name, InferencePtr scoreInference)
    : StaticDecoratorInference(name, scoreInference)
    {setBatchLearner(onlineToBatchInferenceLearner());}
  BinaryClassificationInference() {}

  virtual ScalarFunctionPtr getLoss(bool isPositive) const = 0;

  virtual TypePtr getSupervisionType() const
    {return sumType(booleanType(), probabilityType());}

  virtual TypePtr getOutputType(TypePtr ) const
    {return probabilityType();}

  virtual void setName(const String& name)
  {
    DecoratorInference::setName(name); 
    decorated->setName(name + T(" score"));
  }

  virtual DecoratorInferenceStatePtr prepareInference(InferenceContextPtr context, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    DecoratorInferenceStatePtr res = new DecoratorInferenceState(input, supervision);
    ScalarFunctionPtr lossFunction;
    if (supervision)
    {
      double supervisionValue = 0.0;
      if (supervision.isBoolean())
        supervisionValue = supervision.getBoolean() ? 1.0 : -1.0;
      else if (supervision.inheritsFrom(probabilityType()))
        supervisionValue = supervision.getDouble() * 2.0 - 1.0;
      else
        jassert(false);

      bool isPositive = supervisionValue > 0.0;
      ScalarFunctionPtr* f = isPositive ? &positiveLoss : &negativeLoss;
      if (!*f)
        *f = getLoss(isPositive);
      lossFunction = *f;

      if (supervisionValue < 0)
        supervisionValue = -supervisionValue;
      if (supervisionValue != 1.0)
        lossFunction = lossFunction->multiplyByScalar(supervisionValue);
    }

    res->setSubInference(decorated, input, lossFunction);
    return res;
  }
   
  virtual Variable finalizeInference(InferenceContextPtr context, DecoratorInferenceStatePtr finalState, ReturnCode& returnCode)
  {
    static const double temperature = 1.0;
    Variable subInferenceOutput = finalState->getSubOutput();
    if (!subInferenceOutput)
      return Variable();
    double score = subInferenceOutput.getDouble();
    return Variable(1.0 / (1.0 + exp(-score * temperature)), probabilityType());
  }

protected:
  ScalarFunctionPtr negativeLoss;
  ScalarFunctionPtr positiveLoss;
};

class BinaryLinearSVMInference : public BinaryClassificationInference
{
public:
  BinaryLinearSVMInference(InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}
  BinaryLinearSVMInference(InferencePtr scoreInference)
    : BinaryClassificationInference(scoreInference->getName(), scoreInference)
    {}
  BinaryLinearSVMInference() {}

  virtual ScalarFunctionPtr getLoss(bool isPositive) const
    {return hingeLoss(isPositive ? 1 : 0);}
};

class BinaryLogisticRegressionInference : public BinaryClassificationInference
{
public:
  BinaryLogisticRegressionInference(InferenceOnlineLearnerPtr learner, const String& name)
    : BinaryClassificationInference(name, linearScalarInference(name))
    {decorated->setOnlineLearner(learner);}
  BinaryLogisticRegressionInference() {}

  virtual ScalarFunctionPtr getLoss(bool isPositive) const
    {return logBinomialLoss(isPositive ? 1 : 0);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_BINARY_CLASSIFICATION_H_