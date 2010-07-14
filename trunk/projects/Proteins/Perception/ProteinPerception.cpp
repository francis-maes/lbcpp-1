/*-----------------------------------------.---------------------------------.
| Filename: ProteinPerception.cpp          | Protein Perception              |
| Author  : Francis Maes                   |                                 |
| Started : 14/07/2010 19:14               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "ProteinPerception.h"
using namespace lbcpp;

// todo: Class::declare(new IntegerType(T("SequenceSeparationDistance"), integerType()));
TypePtr sequenceSeparationDistanceType()
  {return integerType();} // FIXME

class ProteinLengthPerception : public Perception
{
public:
  virtual TypePtr getInputType() const
    {return proteinClass();}

  virtual size_t getNumOutputVariables() const
    {return 1;}

  virtual TypePtr getOutputVariableType(size_t index) const
    {return sequenceSeparationDistanceType();}

  virtual String getOutputVariableName(size_t index) const
    {return T("proteinLength");}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr callback) const
    {callback->sense(0, input.getObjectAndCast<Protein>()->getLength());}
};

PerceptionPtr lbcpp::proteinLengthPerception()
  {return new ProteinLengthPerception();}

#if 0

class PerceptionModifier : public DecoratorPerception
{
public:
  PerceptionModifier(PerceptionPtr target = PerceptionPtr()) : DecoratorPerception(target)
    {callback.setStaticallyAllocated(); callback.owner = this;}

  virtual TypePtr getModifiedType(size_t perceptionIndex, TypePtr perceptionType) const
    {return perceptionType;}

  virtual Variable getModifiedValue(size_t perceptionIndex, const Variable& value) const
    {return value;}

  virtual std::pair<PerceptionPtr, Variable> getModifiedValue(size_t perceptionIndex, PerceptionPtr subPerception, const Variable& input) const
    {return std::make_pair(subPerception, input);}
  
  virtual TypePtr getOutputVariableType(size_t index) const
    {return getModifiedType(index, decorated->getOutputVariableType(index));}

  virtual void computePerception(const Variable& input, PerceptionCallbackPtr targetCallback) const
  {
    ReferenceCountedObjectPtr<SubstituteCallback> callback(const_cast<SubstituteCallback* >(&callback));
    callback->target = targetCallback;
    decorated->computePerception(input, callback);
  }

private:
  struct SubstituteCallback : public PerceptionCallback
  {
    PerceptionModifier* owner;
    PerceptionCallbackPtr target;
    
    virtual void sense(size_t variableNumber, const Variable& value)
      {target->sense(variableNumber, owner->getModifiedValue(variableNumber, value));}

    virtual void sense(size_t variableNumber, PerceptionPtr subPerception, const Variable& input)
    {
      std::pair<PerceptionPtr, Variable> p = owner->getModifiedValue(variableNumber, subPerception, input);
      target->sense(variableNumber, p.first, p.second);
    }
  };

  SubstituteCallback callback;
};

class PerceptionToFeaturesModifier : public PerceptionModifier
{
public:
  virtual TypePtr getModifiedType(size_t perceptionIndex, TypePtr perceptionType) const
  {
    EnumerationPtr enumeration = perceptionType.dynamicCast<Enumeration>();
    if (enumeration)
    {
      // ... 
    }
    return perceptionType;
  }

  virtual Variable getModifiedValue(size_t perceptionIndex, const Variable& value) const
    {return value;}

  virtual std::pair<PerceptionPtr, Variable> getModifiedValue(size_t perceptionIndex, PerceptionPtr subPerception, const Variable& input) const
    {return std::make_pair(subPerception, input);}
};
#endif // 0
