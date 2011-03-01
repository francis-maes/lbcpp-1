/*-----------------------------------------.---------------------------------.
| Filename: Optimizer.h                    | Optimizers                      |
| Author  : Francis Maes                   |                                 |
| Started : 21/12/2010 23:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_H_
# define LBCPP_OPTIMIZER_H_

# include "OptimizerInput.h"

namespace lbcpp
{

class Optimizer : public Function
{
public:
  virtual size_t getNumRequiredInputs() const 
    {return getMinimumNumRequiredInputs();}
  virtual size_t getMinimumNumRequiredInputs() const 
    {return 2;}
  virtual size_t getMaximumNumRequiredInputs() const
    {return 3;}
  
  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
  {
    switch (index) {
      case 0:
        return (TypePtr) functionClass;
      case 1:
        return (TypePtr) distributionClass(anyType);  // TODO
      case 2:
        if (numInputs != 3) 
        {
          jassert(false);
          return anyType;
        } else
          return variableType;
      default:
        jassert(false);
        return anyType;
    }
  }
  
  /*virtual TypePtr getInputType() const
    {return optimizerInputClass;}

  virtual TypePtr getOutputType(TypePtr ) const
    {return variableType;}*/
};

typedef ReferenceCountedObjectPtr<Optimizer> OptimizerPtr;

extern OptimizerPtr uniformSampleAndPickBestOptimizer(size_t numSamples);
extern OptimizerPtr iterativeBracketingOptimizer(size_t numPasses, double reductionFactor, const OptimizerPtr& baseOptimizer);

}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_H_
