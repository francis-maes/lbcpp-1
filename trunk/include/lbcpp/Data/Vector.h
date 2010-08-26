/*-----------------------------------------.---------------------------------.
| Filename: Vector.h                       | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:41               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_VECTOR_H_
# define LBCPP_OBJECT_VECTOR_H_

# include "Container.h"

namespace lbcpp
{

class Vector : public Container
{
public:
  Vector(ClassPtr thisClass) : Container(thisClass) {}
  Vector() {}

  /*
  ** Vector
  */
  virtual void clear() = 0;
  virtual void reserve(size_t size) = 0;
  virtual void resize(size_t size) = 0;
  virtual void prepend(const Variable& value) = 0;
  virtual void append(const Variable& value) = 0;
  virtual void remove(size_t index) = 0;

  /*
  ** Object
  */
  virtual String toString() const;
  virtual bool loadFromXml(XmlElement* xml, MessageCallback& callback);

protected:
  bool checkType(const Variable& value) const;
};

typedef ReferenceCountedObjectPtr<Vector> VectorPtr;

class GenericVector : public Vector
{
public:
  GenericVector(TypePtr elementsType, size_t initialSize);
  GenericVector() {}
  virtual ~GenericVector()
    {clear();}

  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);
  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  /*
  ** Object
  */
  virtual void saveToXml(XmlExporter& exporter) const;
  virtual bool loadFromXml(XmlElement* xml, MessageCallback& callback);

protected:
  std::vector<VariableValue> values;
};

class BooleanVector : public Vector
{
public:
  BooleanVector(size_t initialSize);
  BooleanVector() {}

  void set(size_t index, bool value)
    {jassert(index < v.size()); v[index] = value;}

  bool get(size_t index) const
    {jassert(index < v.size()); return v[index];}


  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual TypePtr getElementsType() const
    {return booleanType();}

  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  /*
  ** Object
  */
  virtual String toString() const;

protected:
  std::vector<bool> v;
};

typedef ReferenceCountedObjectPtr<BooleanVector> BooleanVectorPtr;

class ObjectVector : public Vector
{
public:
  ObjectVector(TypePtr elementsType, size_t initialSize);
  ObjectVector() {}
 
  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

  ObjectPtr get(size_t index) const
    {jassert(index < objects.size()); return objects[index];}

  template<class T>
  ReferenceCountedObjectPtr<T> getAndCast(size_t index) const
    {return checkCast<T>(T("ObjectVector::getAndCast"), get(index));}

  void set(size_t index, ObjectPtr object)
    {objects[index] = object;}

protected:
  std::vector<ObjectPtr> objects;
};

typedef ReferenceCountedObjectPtr<ObjectVector> ObjectVectorPtr;

class VariableVector : public Vector
{
public:
  VariableVector(size_t initialSize);
  VariableVector() {}

  Variable& getElement(size_t index)
    {jassert(index < variables.size()); return variables[index];}

  /*
  ** Vector
  */
  virtual void clear();
  virtual void reserve(size_t size);
  virtual void resize(size_t size);

  virtual void prepend(const Variable& value);
  virtual void append(const Variable& value);
  virtual void remove(size_t index);

  /*
  ** Container
  */
  virtual TypePtr getElementsType() const;
  virtual size_t getNumElements() const;
  virtual Variable getElement(size_t index) const;
  virtual void setElement(size_t index, const Variable& value);

protected:
  std::vector<Variable> variables;
};

typedef ReferenceCountedObjectPtr<VariableVector> VariableVectorPtr;

template<class ImplementationType, class ObjectType>
class BuiltinVector : public Vector
{
public:
  BuiltinVector(ClassPtr thisClass, size_t initialSize, const ImplementationType& defaultValue)
    : Vector(thisClass), values(initialSize, defaultValue) {}
  BuiltinVector(ClassPtr thisClass, const std::vector<ImplementationType>& values)
    : Vector(thisClass), values(values) {}
  BuiltinVector() {}

  virtual TypePtr getElementsType() const
    {return thisClass->getTemplateArgument(0);}

  virtual size_t getNumElements() const
    {return values.size();}

  virtual Variable getElement(size_t index) const
  {
    if (values[index].exists())
      return ObjectPtr(new ObjectType(values[index]));
    else
      return Variable::missingValue(getElementsType());
  }

  virtual void setElement(size_t index, const Variable& value)
    {values[index] = getImplementation(value);}

  virtual void saveToXml(XmlExporter& exporter) const
    {jassert(false);}

  virtual bool loadFromXml(XmlElement* xml, MessageCallback& callback)
    {jassert(false); return false;}

  virtual void reserve(size_t size)
    {values.reserve(size);}

  virtual void resize(size_t size)
    {values.resize(size, ImplementationType());}

  virtual void clear()
    {values.clear();}

  virtual void prepend(const Variable& value)
    {values.insert(values.begin(), getImplementation(value));}

  virtual void append(const Variable& value)
    {values.push_back(getImplementation(value));}

  virtual void remove(size_t index)
    {values.erase(values.begin() + index);}

  void append(const ImplementationType& value)
    {values.push_back(value);}

protected:
  std::vector<ImplementationType> values;

  static ImplementationType getImplementation(const Variable& value)
  {
    ReferenceCountedObjectPtr<ObjectType> v = value.getObjectAndCast<ObjectType>();
    return v ? ImplementationType() : v->getValue();
  }
};

extern ClassPtr vectorClass(TypePtr elementsType);
extern ClassPtr genericVectorClass(TypePtr elementsType);
extern ClassPtr objectVectorClass(TypePtr elementsType);
extern ClassPtr booleanVectorClass();
extern ClassPtr variableVectorClass();

extern VectorPtr vector(TypePtr elementsType, size_t initialSize = 0);
extern VectorPtr genericVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr booleanVector(size_t initialSize);
extern VectorPtr objectVector(TypePtr elementsType, size_t initialSize);
extern VectorPtr variableVector(size_t initialSize);

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_VECTOR_H_
