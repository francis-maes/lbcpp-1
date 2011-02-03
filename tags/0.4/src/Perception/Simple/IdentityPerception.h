/*-----------------------------------------.---------------------------------.
| Filename: IdentityPerception.h           | Identity Perception             |
| Author  : Francis Maes                   |                                 |
| Started : 13/07/2010 16:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_
# define LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_

# include <lbcpp/Data/Stream.h>
# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{

class IdentityPerception : public Perception
{
public:
  IdentityPerception(TypePtr type)
    : type(type) {computeOutputType();}
  IdentityPerception() {}

  virtual TypePtr getInputType() const
    {return type;}

  virtual String toString() const
    {return classNameToOutputClassName(type->getName());}

  virtual TypePtr getOutputType() const
    {return type;}

  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}

  virtual void computeOutputType()
  {
    size_t n = type->getNumMemberVariables();
    reserveOutputVariables(n);
    for (size_t i = 0; i < n; ++i)
      addOutputVariable(type->getMemberVariableName(i), type->getMemberVariableType(i));
    Perception::computeOutputType();
  }

  virtual void computePerception(ExecutionContext& context, const Variable& input, PerceptionCallbackPtr callback) const
  {
    jassert(input.isObject());
    ObjectPtr inputObject = input.getObject();
    if (inputObject)
    {
      VariableIterator* iterator = inputObject->createVariablesIterator();
      if (iterator)
      {
        for ( ; iterator->exists(); iterator->next())
        {
          size_t index;
          Variable value = iterator->getCurrentVariable(index);
          callback->sense(index, value);
        }
        delete iterator;
      }
      else
      {
        size_t n = type->getNumMemberVariables();
        for (size_t i = 0; i < n; ++i)
        {
          Variable variable = inputObject->getVariable(i);
          if (variable.exists())
            callback->sense(i, variable);
        }
      }
    }
  }

  lbcpp_UseDebuggingNewOperator

protected:
  friend class IdentityPerceptionClass;

  TypePtr type;
};

}; /* namespace lbcpp */

#endif // !LBCPP_FUNCTION_PERCEPTION_IDENTITY_H_
