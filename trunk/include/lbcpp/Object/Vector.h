/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VECTOR_H_
# define LBCPP_OBJECT_VECTOR_H_

# include "VariableContainer.h"

namespace lbcpp
{

class Vector : public VariableContainer
{
public:
  Vector(TypePtr elementsType, size_t initialSize = 0);
  Vector() {}

  virtual ~Vector()
    {clear();}

  virtual TypePtr getStaticType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumVariables() const;
  virtual Variable getVariable(size_t index) const;
  virtual void setVariable(size_t index, const Variable& value);
  virtual void saveToXml(XmlElement* xml) const;
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback);

  void reserve(size_t size)
    {values.reserve(size);}

  void clear();
  void append(const Variable& value);

protected:
  std::vector<VariableValue> values;
  
  bool checkType(const Variable& value) const;
};

typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

extern ClassPtr vectorClass(TypePtr elementsType);

class BooleanVector : public VariableContainer
{
public:
  BooleanVector(size_t initialSize = 0)
    : v(initialSize, false) {}

  virtual String toString() const
  {
    String res = T("[");
    for (size_t i = 0; i < v.size(); ++i)
      res += v[i] ? '+' : '-';
    res += T("]");
    return res;
  }

  virtual TypePtr getStaticType() const
    {return booleanType();}

  virtual size_t getNumVariables() const
    {return v.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < v.size()); return v[index];}

  virtual void setVariable(size_t index, const Variable& value)
  {
    if (checkInheritance(value, booleanType()))
      v[index] = value.getBoolean();
  }

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value;}

  bool get(size_t index) const
    {jassert(index < v.size()); return v[index];}

protected:
  std::vector<bool> v;
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

template<class ImplementationType, class ObjectType>
class BuiltinVector : public VariableContainer
{
public:
  BuiltinVector(size_t initialSize, const ImplementationType& defaultValue)
    : values(initialSize, defaultValue) {}
  BuiltinVector(const std::vector<ImplementationType>& values)
    : values(values) {}
  BuiltinVector() {}

  virtual TypePtr getStaticType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumVariables() const
    {return values.size();}

  virtual Variable getVariable(size_t index) const
  {
    if (values[index].exists())
      return ObjectPtr(new ObjectType(values[index]));
    else
      return Variable::missingValue(getStaticType());
  }

  virtual void setVariable(size_t index, const Variable& value)
  {
    ReferenceCountedObjectPtr<ObjectType> v = value.getObjectAndCast<ObjectType>();
    if (v)
      values[index] = ImplementationType();
    else
      values[index] = v->getValue();
  }

  virtual void saveToXml(XmlElement* xml) const
    {jassert(false);}
  virtual bool loadFromXml(XmlElement* xml, ErrorHandler& callback)
    {jassert(false); return false;}

  void reserve(size_t size)
    {values.reserve(size);}

  void clear()
    {values.clear();}

  void append(const ImplementationType& value)
    {values.push_back(value);}

protected:
  std::vector<ImplementationType> values;
};

class DynamicObject : public VariableContainer
{
public:
  DynamicObject(ClassPtr type = objectClass()) : type(type) {}

  virtual ClassPtr getClass() const
    {return type;}

  virtual size_t getNumVariables() const
    {return variables.size();}

  virtual Variable getVariable(size_t index) const
    {jassert(index < variables.size()); return variables[index];}

  virtual void setVariable(size_t index, const Variable& value)
    {jassert(index < variables.size()); variables[index] = value;}

  void reserveVariables(size_t size)
    {variables.reserve(size);}

  void clearVariables()
    {variables.clear();}

  void appendVariable(const Variable& value)
    {variables.push_back(value);}

private:
  ClassPtr type;
  std::vector<Variable> variables;
};

typedef ReferenceCountedObjectPtr<DynamicObject> DynamicObjectPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_
