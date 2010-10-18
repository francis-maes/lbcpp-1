/*-----------------------------------------.---------------------------------.
| Filename: NumericalInference.cpp         | Numerical Inference             |
| Author  : Francis Maes                   |   base class                    |
| Started : 30/09/2010 11:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "NumericalInference.h"
#include <lbcpp/Function/ScalarObjectFunction.h>
#include <lbcpp/Perception/PerceptionMaths.h>
using namespace lbcpp;

NumericalInference::NumericalInference(const String& name, PerceptionPtr perception)
  : ParameterizedInference(name), perception(perception) {}

void NumericalInference::addWeightedToParameters(const ObjectPtr& value, double weight)
{
  {
    ScopedWriteLock _(parametersLock);
    lbcpp::addWeighted(parameters, value, weight);
  }
  parametersChangedCallback();
}

void NumericalInference::applyRegularizerToParameters(ScalarObjectFunctionPtr regularizer, double weight)
{
  {
    ScopedWriteLock _(parametersLock);
    if (parameters)
      regularizer->compute(parameters, NULL, &parameters, weight);
  }
  parametersChangedCallback();
}
