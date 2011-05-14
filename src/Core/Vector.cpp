/*-----------------------------------------.---------------------------------.
| Filename: Vector.cpp                     | Vector of variables             |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:51               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/Vector.h>
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Data/DoubleVector.h>
#include <lbcpp/Distribution/Distribution.h>
using namespace lbcpp;

/*
** Vector
*/
bool Vector::checkType(const Variable& value) const
  {return checkInheritance(value, getElementsType());}

String Vector::toString() const
{
  TypePtr type = getElementsType();
  size_t n = getNumElements();
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String value;
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getElement(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += enumeration->getElement(variable.getInteger())->getOneLetterCode();
    }
    return value;
  }

  if (type->inheritsFrom(doubleType))
  {
    String value;
    for (size_t i = 0; i < n; ++i)
    {
      Variable variable = getElement(i);
      if (variable.isMissingValue())
        value += '_';
      else
        value += String(variable.getDouble());
      if (i < n - 1)
        value += ", ";
    }
    return value;
  }

  return Container::toString();
}

bool Vector::loadFromXml(XmlImporter& importer)
{
  int size = importer.getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    importer.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  resize(size);
  return Container::loadFromXml(importer);
}

bool Vector::loadFromString(ExecutionContext& context, const String& stringValue)
{
  TypePtr elementsType = getElementsType();
  StringArray tokens;
  tokens.addTokens(stringValue, T(","), T("\""));
  resize(tokens.size());
  for (int i = 0; i < tokens.size(); ++i)
  {
    Variable variable = Variable::createFromString(context, elementsType, tokens[i]);
    if (!variable.exists())
      return false;
    setElement(i, variable);
  }
  return true;
}

void Vector::clone(ExecutionContext& context, const ObjectPtr& target) const
{
  VectorPtr targetVector = target.staticCast<Vector>();
  targetVector->resize(getNumElements());
  Container::clone(context, targetVector);
}

/*
** GenericVector
*/
GenericVector::GenericVector(TypePtr elementsType, size_t initialSize)
  : Vector(genericVectorClass(elementsType))
{
  jassert(elementsType != topLevelType);
  if (initialSize)
    values.resize(initialSize, elementsType->getMissingValue());
}

size_t GenericVector::getNumElements() const
  {return values.size();}

Variable GenericVector::getElement(size_t index) const
{
  jassert(index < values.size());
  TypePtr elementsType = getElementsType();
  if (elementsType.isInstanceOf<Class>())
  {
    ObjectPtr res = values[index].getObjectPointer();
    return res ? Variable(res) : Variable::missingValue(elementsType);
  }
  else
    return Variable::copyFrom(elementsType, values[index]);
}

void GenericVector::setElement(size_t index, const Variable& value)
{
  if (checkType(value))
  {
    jassert(index < values.size());
    value.copyTo(values[index]);
  }
}

void GenericVector::clear()
{
  TypePtr type = getElementsType();
  for (size_t i = 0; i < values.size(); ++i)
    type->destroy(values[i]);
  values.clear();
}

void GenericVector::reserve(size_t size)
  {values.reserve(size);}

void GenericVector::resize(size_t size)
  {values.resize(size, getElementsType()->getMissingValue());}

void GenericVector::prepend(const Variable& value)
{
  if (checkType(value))
  {
    values.insert(values.begin(), getElementsType()->getMissingValue());
    value.copyTo(values.front());
  }
}

void GenericVector::append(const Variable& value)
{
  if (checkType(value))
  {
    values.push_back(getElementsType()->getMissingValue());
    value.copyTo(values.back());
  }
}

void GenericVector::remove(size_t index)
{
  jassert(index < values.size());
  TypePtr type = getElementsType();
  type->destroy(values[index]);
  values.erase(values.begin() + index);
}

void GenericVector::saveToXml(XmlExporter& exporter) const
{
  size_t n = getNumElements();

  TypePtr type = getElementsType();
  exporter.setAttribute(T("size"), (int)n);

  // enumeration vectors: as text
  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if ((enumeration && enumeration->hasOneLetterCodes()) || type->inheritsFrom(doubleType))
  {
    exporter.addTextElement(toString());
    return;
  }

  // long builtin-type vectors: binary encoding
  if (n > 1000 && (type->inheritsFrom(integerType) || type->inheritsFrom(doubleType) || type->inheritsFrom(booleanType)))
  {
    juce::MemoryBlock block(&values[0], (int)(sizeof (VariableValue) * values.size()));
    exporter.setAttribute(T("binary"), T("true"));
    exporter.addTextElement(block.toBase64Encoding());
    return;
  }

  // other vectors: encore into XML child elements (default implementation)
  Container::saveToXml(exporter);
}

bool GenericVector::loadFromXml(XmlImporter& importer)
{
  TypePtr type = getElementsType();
  jassert(type);
  int size = importer.getIntAttribute(T("size"), -1);
  if (size < 0)
  {
    importer.errorMessage(T("Vector::loadFromXml"), T("Invalid size: ") + String(size));
    return false;
  }
  values.resize(size, type->getMissingValue());

  if (importer.getBoolAttribute(T("binary")))
  {
    if (!type->inheritsFrom(integerType) && !type->inheritsFrom(doubleType) && !type->inheritsFrom(booleanType))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Unexpected type for binary encoding"));
      return false;
    }

    juce::MemoryBlock block;
    if (!block.fromBase64Encoding(importer.getAllSubText().trim()))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Could not decode base 64"));
      return false;
    }

    if (block.getSize() != (int)(sizeof (VariableValue) * values.size()))
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Invalid data size: found ") + String(block.getSize())
        + T(" expected ") + String((int)(sizeof (VariableValue) * values.size())));
      return false;
    }
    memcpy(&values[0], block.getData(), block.getSize());
    return true;
  }

  EnumerationPtr enumeration = type.dynamicCast<Enumeration>();
  if (enumeration && enumeration->hasOneLetterCodes())
  {
    String text = importer.getAllSubText().trim();
    if (text.length() != size)
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(text.length()));
      return false;
    }
    
    for (size_t i = 0; i < values.size(); ++i)
    {
      int j = enumeration->findElementByOneLetterCode(text[(int)i]);
      if (j >= 0)
        values[i] = VariableValue(j);
      else
      {
        if (text[(int)i] != '_')
          importer.warningMessage(T("Vector::loadFromXml"), String(T("Could not recognize one letter code '")) + text[(int)i] + T("'"));
        values[i] = enumeration->getMissingValue();
      }
    }
    return true;
  }

  if (type->inheritsFrom(doubleType))
  {
    String text = importer.getAllSubText().trim();
    StringArray tokens;
    tokens.addTokens(text, T(" \t\r\n"), NULL);
    if (tokens.size() != size)
    {
      importer.errorMessage(T("Vector::loadFromXml"), T("Size does not match. Expected ") + String(size) + T(", found ") + String(tokens.size()));
      return false;
    }
    for (size_t i = 0; i < values.size(); ++i)
      if (tokens[(int)i] != T("_"))
      {
        Variable value = Variable::createFromString(importer.getContext(), doubleType, tokens[(int)i]);
        if (!value.exists())
          return false;
        values[i] = value.getDouble();
      }
    return true;
  }

  // default implementation  
  return Container::loadFromXml(importer);
}

/*
** BooleanVector
*/
BooleanVector::BooleanVector(size_t initialSize)
  : Vector(booleanVectorClass)
{
  if (initialSize)
    v.resize(initialSize, false);
}

String BooleanVector::toString() const
{
  String res = T("[");
  for (size_t i = 0; i < v.size(); ++i)
    res += v[i] ? '+' : '-';
  res += T("]");
  return res;
}

size_t BooleanVector::getNumElements() const
  {return v.size();}

Variable BooleanVector::getElement(size_t index) const
  {jassert(index < v.size()); return v[index];}

void BooleanVector::setElement(size_t index, const Variable& value)
{
  if (checkInheritance(value, booleanType))
    v[index] = value.getBoolean();
}

void BooleanVector::reserve(size_t size)
  {v.reserve(size);}

void BooleanVector::resize(size_t size)
  {v.resize(size);}

void BooleanVector::clear()
  {v.clear();}

void BooleanVector::prepend(const Variable& value)
  {v.insert(v.begin(), value.getBoolean());}

void BooleanVector::append(const Variable& value)
  {v.push_back(value.getBoolean());}

void BooleanVector::remove(size_t index)
  {v.erase(v.begin() + index);}

/*
** ObjectVector
*/
ObjectVector::ObjectVector(TypePtr elementsType, size_t initialSize)
  : Vector(objectVectorClass(elementsType)), objects(new std::vector<ObjectPtr>(initialSize)), ownObjects(true)
{
}

ObjectVector::ObjectVector(ClassPtr thisClass)
  : Vector(thisClass), objects(new std::vector<ObjectPtr>()), ownObjects(true)
{
}

ObjectVector::ObjectVector(const std::vector<ObjectPtr>& reference, TypePtr elementsType)
  : Vector(objectVectorClass(elementsType ? elementsType : (TypePtr)(reference.size() ? reference[0]->getClass() : objectClass))),
    objects(const_cast<std::vector<ObjectPtr>* >(&reference)), ownObjects(false)
{
}

ObjectVector::ObjectVector(std::vector<ObjectPtr>& reference, TypePtr elementsType)
  : Vector(objectVectorClass(elementsType ? elementsType : (TypePtr)(reference.size() ? reference[0]->getClass() : objectClass))),
    objects(&reference), ownObjects(false)
{
}

ObjectVector::ObjectVector() : objects(NULL), ownObjects(false)
{
}

ObjectVector::~ObjectVector()
{
  if (ownObjects)
  {
    jassert(objects);
    delete objects;
  }
}

void ObjectVector::clear()
  {objects->clear();}

void ObjectVector::reserve(size_t size)
{
  if (!objects)
  {
    objects = new std::vector<ObjectPtr>();
    ownObjects = true;
  }
  objects->reserve(size);
}

void ObjectVector::resize(size_t size)
{
  if (objects)
    objects->resize(size);
  else
  {
    objects = new std::vector<ObjectPtr>(size);
    ownObjects = true;
  }
}

void ObjectVector::prepend(const Variable& value)
  {objects->insert(objects->begin(), value.getObject());}

void ObjectVector::append(const Variable& value)
  {objects->push_back(value.getObject());}

void ObjectVector::remove(size_t index)
  {objects->erase(objects->begin() + index);}

size_t ObjectVector::getNumElements() const
  {return objects->size();}

Variable ObjectVector::getElement(size_t index) const
{
  jassert(index < objects->size());
  const ObjectPtr& res = (*objects)[index];
  TypePtr elementsType = res ? (TypePtr)res->getClass() : getElementsType();
  return Variable(res, elementsType);
}

void ObjectVector::setElement(size_t index, const Variable& value)
  {jassert(index < objects->size()); (*objects)[index] = value.getObject();}


/*
** VariableVector
*/
VariableVector::VariableVector(size_t initialSize)
  : Vector(variableVectorClass), variables(initialSize)
{
}

void VariableVector::clear()
  {variables.clear();}

void VariableVector::reserve(size_t size)
  {variables.reserve(size);}

void VariableVector::resize(size_t size)
  {variables.resize(size);}

void VariableVector::prepend(const Variable& value)
  {variables.insert(variables.begin(), value);}

void VariableVector::append(const Variable& value)
  {variables.push_back(value);}

void VariableVector::remove(size_t index)
  {variables.erase(variables.begin() + index);}

TypePtr VariableVector::getElementsType() const
  {return anyType;}

size_t VariableVector::getNumElements() const
  {return variables.size();}

Variable VariableVector::getElement(size_t index) const
  {jassert(index < variables.size()); return variables[index];}

void VariableVector::setElement(size_t index, const Variable& value)
  {jassert(index < variables.size()); variables[index] = value;}

void VariableVector::saveToXml(XmlExporter& exporter) const
{
  size_t n = getNumElements();
  exporter.setAttribute(T("size"), (int)n);
  for (size_t i = 0; i < n; ++i)
    exporter.saveElement(i, getElement(i), anyType);
}


/*
** Vector Constructor Method
*/
VectorPtr lbcpp::vector(TypePtr elementsType, size_t initialSize)
{
  jassert(elementsType);
  if (elementsType->inheritsFrom(booleanType))
    return booleanVector(initialSize);
  else if (elementsType->inheritsFrom(objectClass))
    return objectVector(elementsType, initialSize);
  else if (elementsType->inheritsFrom(doubleType))
    return new DenseDoubleVector(denseDoubleVectorClass(positiveIntegerEnumerationEnumeration, elementsType), initialSize);
  else if (elementsType == anyType)
    return variableVector(initialSize);
  else
    return genericVector(elementsType, initialSize);
}
