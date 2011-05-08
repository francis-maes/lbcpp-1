/*-----------------------------------------.---------------------------------.
| Filename: ApplyFunctionContainer.h       | Apply a function in a lazy way  |
| Author  : Francis Maes                   |                                 |
| Started : 23/08/2010 13:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_
# define LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_

# include <lbcpp/Core/Container.h>
# include <lbcpp/Core/Function.h>

namespace lbcpp
{

class ApplyFunctionContainer : public DecoratorContainer
{
public:
  ApplyFunctionContainer(ContainerPtr target, FunctionPtr function)
    : DecoratorContainer(target), function(function)
    {checkInheritance(target->getElementsType(), function->getRequiredInputType(0, 1));}

  ApplyFunctionContainer() {}
    
  virtual TypePtr getElementsType() const
    {return function->getOutputType();}

  virtual Variable getElement(size_t index) const
    {return function->compute(defaultExecutionContext(), target->getElement(index));}

  virtual void setElement(size_t index, const Variable& value)
    {jassert(false);}

private:
  friend class ApplyFunctionContainerClass;

  FunctionPtr function;
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_CONTAINER_APPLY_FUNCTION_H_