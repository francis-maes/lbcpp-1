/*-----------------------------------------.---------------------------------.
| Filename: GradientBasedLearner.cpp       | Gradient based learner          |
| Author  : Francis Maes                   |                                 |
| Started : 08/03/2009 23:15               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <cralgo/GradientBasedLearner.h>
#include <cralgo/impl/impl.h>
using namespace cralgo;

class ConstantIterationFunction : public IterationFunction
{
public:
  ConstantIterationFunction(double value) : value(value) {}
  
  virtual double compute(size_t iteration) const
    {return value;}
    
private:
  double value;
};

IterationFunctionPtr IterationFunction::createConstant(double value)
  {return new ConstantIterationFunction(value);}

class GradientDescentLearner : public GradientBasedLearner
{
public:
  GradientDescentLearner(IterationFunctionPtr learningRate, bool normalizeLearningRate)
    : learningRate(learningRate), normalizeLearningRate(normalizeLearningRate), epoch(0) {}
    
  virtual void trainStochasticExample(DenseVectorPtr parameters, ScalarVectorFunctionPtr exampleLoss, ScalarVectorFunctionPtr regularizer)
  {
    parameters->addWeighted(exampleLoss->computeGradient(parameters), -computeAlpha());
    ++epoch;
  }
  
  virtual void trainStochasticEnd(DenseVectorPtr parameters, ScalarVectorFunctionPtr regularizer)
  {
    // apply regularizer
    if (regularizer)
      parameters->addWeighted(regularizer->computeGradient(parameters), -computeAlpha());
  }

  virtual void trainBatch(DenseVectorPtr parameters, ScalarVectorFunctionPtr objective, size_t numExamples)
  {
    for (int i = 0; i < 100; ++i)
    {
      std::cout << "Iteration " << i << " objective = " << objective->compute(parameters) << std::endl; 
      parameters->addWeighted(objective->computeGradient(parameters), -computeAlpha() * numExamples);
      epoch += numExamples;
    }
  }
  
protected:
  IterationFunctionPtr learningRate;
  bool normalizeLearningRate;
  size_t epoch;
  
  double computeAlpha() const
  {
    double meanNumberOfFeatures = 0.0; // FIXME
    return (learningRate ? learningRate->compute(epoch) : 1.0) * 
        (normalizeLearningRate && meanNumberOfFeatures ? 1.0 / meanNumberOfFeatures : 1.0);
  }
};

GradientBasedLearnerPtr GradientBasedLearner::createGradientDescent(IterationFunctionPtr learningRate, bool normalizeLearningRate)
  {return GradientBasedLearnerPtr(new GradientDescentLearner(learningRate, normalizeLearningRate));}
