/*-----------------------------------------.---------------------------------.
| Filename: PerceptronLossFunction.hpp     | Perceptron Loss Function        |
| Author  : Francis Maes                   |                                 |
| Started : 11/03/2009 19:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_PERCEPTRON_LOSS_H_
# define CRALGO_IMPL_FUNCTION_PERCEPTRON_LOSS_H_

# include "LossFunctions.hpp"

namespace cralgo {
namespace impl {

// f(x) = max(0, -x)
struct PerceptronLossFunction : public ScalarFunction<PerceptronLossFunction>
{
  void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (input >= 0)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (input == 0)
    {
      if (output) *output = 0.0;
      if (derivative) {assert(derivativeDirection); *derivative = derivativeDirection <= 0 ? -1 : 0;}
    }
    else
    {
      if (output) *output = -input;
      if (derivative) *derivative = -1;
    }
  }
};
inline PerceptronLossFunction perceptronLossFunction()
  {return PerceptronLossFunction();}

STATIC_DISCRIMINANT_LOSS_FUNCTION(perceptronLoss, PerceptronLoss, PerceptronLossFunction);

}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_PERCEPTRON_LOSS_H_
