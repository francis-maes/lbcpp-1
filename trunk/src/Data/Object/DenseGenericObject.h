/*-----------------------------------------.---------------------------------.
| Filename: DenseGenericObject.h           | Dense Generic Object            |
| Author  : Francis Maes                   |                                 |
| Started : 05/10/2010 23:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
# define LBCPP_DATA_OBJECT_DENSE_GENERIC_H_

# include <lbcpp/Data/DynamicObject.h>

namespace lbcpp
{

class DenseGenericObject : public DynamicObject
{
public:
  DenseGenericObject(TypePtr thisType)
    : DynamicObject(thisType) {}
  virtual ~DenseGenericObject()
  {
    for (size_t i = 0; i < variableValues.size(); ++i)
      thisClass->getObjectVariableType(i)->destroy(variableValues[i]);
  }

  VariableValue& getVariableValueReference(size_t index)
  {
    jassert(index < thisClass->getObjectNumVariables());
    if (variableValues.size() <= index)
    {
      size_t i = variableValues.size();
      variableValues.resize(index + 1);
      while (i < variableValues.size())
      {
        variableValues[i] = thisClass->getObjectVariableType(i)->getMissingValue();
        ++i;
      }
    }
    return variableValues[index];
  }

  virtual Variable getVariableImpl(size_t index) const
  {
    TypePtr type = thisClass->getObjectVariableType(index);
    if (index < variableValues.size())
      return Variable::copyFrom(type, variableValues[index]);
    else
      return Variable::missingValue(type);
  }

  virtual void setVariableImpl(size_t index, const Variable& value)
    {value.copyTo(getVariableValueReference(index));}

  virtual VariableIterator* createVariablesIterator() const;

private:
  friend class DenseGenericObjectVariableIterator;

  std::vector<VariableValue> variableValues;
};

typedef ReferenceCountedObjectPtr<DenseGenericObject> DenseGenericObjectPtr;

class DenseGenericObjectVariableIterator : public Object::VariableIterator
{
public:
  DenseGenericObjectVariableIterator(DenseGenericObjectPtr object)
    : object(object), current(0), n(object->variableValues.size()) {moveToNextActiveVariable();}

  virtual bool exists() const
    {return current < n;}
  
  virtual Variable getCurrentVariable(size_t& index) const
    {jassert(current < n); index = current; return currentValue;}

  virtual void next()
  {
    jassert(current < n);
    ++current;
    moveToNextActiveVariable();
  }

private:
  DenseGenericObjectPtr object;
  size_t current;
  Variable currentValue;
  size_t n;

  void moveToNextActiveVariable()
  {
    while (current < n)
    {
      TypePtr type = object->thisClass->getObjectVariableType(current);
      if (!type->isMissingValue(object->variableValues[current]))
      {
        currentValue = Variable::copyFrom(type, object->variableValues[current]);
        break;
      }
      ++current;
    }
  }
};

inline Object::VariableIterator* DenseGenericObject::createVariablesIterator() const
  {return new DenseGenericObjectVariableIterator(refCountedPointerFromThis(this));}

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_OBJECT_DENSE_GENERIC_H_
