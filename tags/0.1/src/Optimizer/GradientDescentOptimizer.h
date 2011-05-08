/*-----------------------------------------.---------------------------------.
| Filename: GradientDescentOptimizer.h     | Gradient Descent Optimizer      |
| Author  : Francis Maes                   |                                 |
| Started : 20/03/2009 16:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_
# define LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_

# include <lbcpp/Optimizer.h>
# include <lbcpp/IterationFunction.h>

namespace lbcpp
{

class GradientDescentOptimizer : public VectorOptimizer
{
public:
  GradientDescentOptimizer(IterationFunctionPtr stepSize)
    : stepSize(stepSize) {}
  GradientDescentOptimizer() {}

  virtual std::string toString() const
    {return "GradientDescentOptimizer(" + lbcpp::toString(stepSize) + ")";}
  
  virtual OptimizerState step()
  {
    assert(stepSize);
    DenseVectorPtr denseParameters = parameters->toDenseVector();
    denseParameters->addWeighted(gradient, -(stepSize->compute(iteration)));
    setParameters(denseParameters);
    return optimizerContinue;
  }

  virtual void save(std::ostream& ostr) const
    {write(ostr, stepSize);}

  virtual bool load(std::istream& istr)
    {return read(istr, stepSize);}
  
private:
  IterationFunctionPtr stepSize;
};

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_GRADIENT_DESCENT_H_