/*-----------------------------------------.---------------------------------.
| Filename: XmlSerialisation.cpp           | Xml Import/Export               |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include <lbcpp/Core/XmlSerialisation.h>
#include <lbcpp/Core/DynamicObject.h>
#include <lbcpp/Execution/ExecutionContext.h>
using namespace lbcpp;

/*
** XmlExporter
*/
XmlExporter::XmlExporter(ExecutionContext& context, const String& rootTag, int version)
  : context(context)
{
  root = new juce::XmlElement(rootTag);
  if (version)
    root->setAttribute(T("version"), version);
  currentStack.push_back(root);
}

XmlExporter::~XmlExporter()
  {delete root;}

bool XmlExporter::saveToFile(const File& file)
{
  flushSave();
  if (file.exists())
  {
    if (file.existsAsFile())
    {
      if (!file.deleteFile())
      {
        context.errorCallback(T("XmlExporter::saveToFile"), T("Could not delete file ") + file.getFullPathName());
        return false;
      }
    }
    else
    {
      context.errorCallback(T("XmlExporter::saveToFile"), file.getFullPathName() + T(" is a directory"));
      return false;
    }
  }
  
  if (!root)
  {
    context.errorCallback(T("XmlExporter::saveToFile"), T("No root xml element in file ") + file.getFullPathName());
    return false;
  }
  bool ok = root->writeToFile(file, String::empty);
  if (!ok)
    context.errorCallback(T("XmlExporter::saveToFile"), T("Could not write file ") + file.getFullPathName());
  return ok;
}

String XmlExporter::toString()
{
  flushSave();
  if (!root)
  {
    context.errorCallback(T("XmlExporter::toString"), T("No root xml element"));
    return String::empty;
  }
  return root->createDocument(String::empty);
}

juce::XmlElement* XmlExporter::getCurrentElement()
  {return currentStack.back();}

void XmlExporter::enter(const String& tagName, const String& name)
{
  juce::XmlElement* elt = new juce::XmlElement(tagName);
  currentStack.push_back(elt);
  writeName(name);
}

void XmlExporter::saveVariable(const String& name, const Variable& variable, TypePtr expectedType)
{
  enter(T("variable"), name);
  writeVariable(variable, expectedType);
  leave();
}

void XmlExporter::saveElement(size_t index, const Variable& variable, TypePtr expectedType)
{
  enter(T("element"));
  setAttribute(T("index"), String((int)index));
  writeVariable(variable, expectedType);
  leave();
}

void XmlExporter::saveDynamicSharedObject(const String& identifier, const ObjectPtr& object, TypePtr expectedType)
{
  writeObject(object, expectedType, true, identifier);
/*
  enter(T("shared"));
  setAttribute(T("dynamic"), T("yes"));
  setAttribute(T("identifier"), identifier);
  leave();*/
}

void XmlExporter::leave()
{
  jassert(currentStack.size() > 1);
  juce::XmlElement* elt = getCurrentElement();
  currentStack.pop_back();
  getCurrentElement()->addChildElement(elt);
}

void XmlExporter::addTextElement(const String& text)
{
  enum {lineLength = 60};
  int n = text.length();
  int b = 0;
  while (b < n)
  {
    int e = -1;
    if (b + lineLength < n)
      e = text.indexOfAnyOf(T(" \t\n\r"), b + lineLength);
    if (e < 0)
      e = n;
    getCurrentElement()->addTextElement(text.substring(b, e));
    b = e;
    if (b < n)
      getCurrentElement()->addTextElement(T("\n"));
  }
}

void XmlExporter::writeName(const String& name)
{
  juce::XmlElement* elt = getCurrentElement();
  if (name.isNotEmpty())
    elt->setAttribute(T("name"), name);
}

void XmlExporter::writeType(TypePtr type)
{
  jassert(type);
  
  if (type->isNamedType())
    setAttribute(T("type"), type->getName().replaceCharacters(T("<>"), T("[]")));
  else
  {
    enter(T("type"));
    writeVariable(type, TypePtr());
    leave();
  }
}

void XmlExporter::writeVariable(const Variable& variable, TypePtr expectedType)
{
  juce::XmlElement* elt = getCurrentElement();
  
  if (variable.isMissingValue())
  {
    writeType(variable.getType());
    elt->setAttribute(T("missing"), T("true"));
  }
  else if (variable.isObject())
  {
    const ObjectPtr& object = variable.getObject();
    TypePtr typeValue = object.dynamicCast<Type>();
    if (typeValue && typeValue->isNamedType())
      addTextElement(typeValue->getName()); // named type
    else
      writeObject(object, expectedType); // traditional object
  }
  else
  {
    TypePtr type = variable.getType();
    if (type != expectedType)
      writeType(type);
    if (!type->isMissingValue(variable.value))
      type->saveToXml(*this, variable.value);
  }
}

void XmlExporter::writeObject(const ObjectPtr& object, TypePtr expectedType, bool isDynamic, const String& identifier)
{
  jassert(object->getReferenceCount());
  juce::XmlElement* currentElement = getCurrentElement();

  size_t index;
  SavedObjectsMap::const_iterator it = savedObjectsMap.find(object);
  if (it == savedObjectsMap.end())
  {
    // create index
    index = savedObjects.size();
    savedObjectsMap[object] = index;

    // create SavedObject
    SavedObject savedObject;
    savedObject.object = object;
    savedObject.elt = new juce::XmlElement(T("NOTAGYET"));
    savedObject.identifier = identifier;
    savedObjects.push_back(savedObject);

    // save object
    currentlySavedObjects.insert(index);
    currentStack.push_back(savedObject.elt);
    if (expectedType != object->getClass())
      writeType(object->getClass());
    if (!isDynamic)
      object->saveToXml(*this);
    currentStack.pop_back();
    currentlySavedObjects.erase(index);

    savedObjects[index].references.push_back(currentElement);
  }
  else
  {
    index = it->second;
    std::vector<juce::XmlElement* >& references = savedObjects[index].references;
    references.push_back(currentElement);
    if (references.size() == 2)
    {
      sharedObjectsIndices.insert(it->second);
      savedObjects[index].ordered = false;
    }
  }

  for (std::set<size_t>::const_iterator it = currentlySavedObjects.begin(); it != currentlySavedObjects.end(); ++it)
    savedObjects[*it].dependencies.insert(index);
}

void XmlExporter::makeSharedObjectsSaveOrder(const std::set<size_t>& sharedObjectsIndices, std::vector<size_t>& res)
{
  std::set<size_t> todo = sharedObjectsIndices;

  res.reserve(todo.size());
  size_t n = todo.size();
  for (size_t iteration = 0; todo.size() && iteration < n; ++iteration)
  {
    std::set<size_t>::const_iterator nxt;
    for (std::set<size_t>::const_iterator it = todo.begin(); it != todo.end(); it = nxt)
    {
      nxt = it; ++nxt;

      bool areAllDependenciesDone = true;
      const std::set<size_t>& dependencies = savedObjects[*it].dependencies;
      for (std::set<size_t>::const_iterator it2 = dependencies.begin(); it2 != dependencies.end(); ++it2)
        if (!savedObjects[*it2].ordered)
        {
          areAllDependenciesDone = false;
          break;
        }
      if (areAllDependenciesDone)
      {
        res.push_back(*it);
        savedObjects[*it].ordered = true;
        todo.erase(it);
      }
    }
  }
  
  if (todo.empty())
    return;
  context.errorCallback(T("XmlExporter::makeSharedObjectsSaveOrder"), T("Cyclic dependancy between shared objects"));
}

void XmlExporter::resolveSingleObjectReference(SavedObject& savedObject)
{
  juce::XmlElement* reference = savedObject.references[0];
  reference->moveChildrenFrom(*savedObject.elt); 
  for (int i = 0; i < savedObject.elt->getNumAttributes(); ++i)
    reference->setAttribute(savedObject.elt->getAttributeName(i), savedObject.elt->getAttributeValue(i));
  deleteAndZero(savedObject.elt);
}

void XmlExporter::resolveSharedObjectReferences(SavedObject& savedObject)
{
  jassert(savedObject.references.size() > 1);
  for (size_t i = 0; i < savedObject.references.size(); ++i)
    savedObject.references[i]->setAttribute(T("reference"), savedObject.identifier);
}

void XmlExporter::flushSave()
{
  // make shared object order
  std::vector<size_t> sharedObjectsOrder;
  makeSharedObjectsSaveOrder(sharedObjectsIndices, sharedObjectsOrder);
  jassert(sharedObjectsOrder.size() == sharedObjectsIndices.size());

  // save all shared objects and solve their references
  std::set<String> sharedObjectIdentifiers;
  for (int i = (int)sharedObjectsOrder.size() - 1; i >= 0; --i)
  {
    SavedObject& savedObject = savedObjects[sharedObjectsOrder[i]];
    savedObject.elt->setTagName(T("shared"));
    if (savedObject.identifier.isEmpty())
      savedObject.identifier = makeUniqueIdentifier(savedObject.object, sharedObjectIdentifiers);
    savedObject.elt->setAttribute(T("identifier"), savedObject.identifier);
    savedObject.elt->setAttribute(T("type"), savedObject.object->getClassName()); 
    root->insertChildElement(savedObject.elt, 0);
    resolveSharedObjectReferences(savedObject);
    savedObject.elt = NULL;
  }
  
  // resolve all remaining single-references
  for (size_t i = 0; i < savedObjects.size(); ++i)
  {
    SavedObject& savedObject = savedObjects[i];
    if (savedObject.references.size() == 1)
      resolveSingleObjectReference(savedObject);
  }
}

String XmlExporter::makeUniqueIdentifier(ObjectPtr object, std::set<String>& identifiers)
{
  String cn = object->getClassName();
  for (int i = 1; true; ++i)
  {
    String res = cn + String(i);
    if (identifiers.find(res) == identifiers.end())
    {
      identifiers.insert(res);
      return res;
    }
  }
  return String::empty;
}
/*
** XmlImporter
*/
XmlImporter::XmlImporter(ExecutionContext& context, const File& file)
  : context(context), root(NULL)
{
  if (file.isDirectory())
  {
    context.errorCallback(T("Variable::createFromFile"), file.getFullPathName() + T(" is a directory"));
    return;
  }
  
  if (!file.existsAsFile())
  {
    context.errorCallback(T("Variable::createFromFile"), file.getFullPathName() + T(" does not exists"));
    return;
  }

  juce::XmlDocument document(file);
  
  root = document.getDocumentElement();
  String lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(T("Variable::createFromFile"),
      lastParseError.isEmpty() ? T("Could not parse file ") + file.getFullPathName() : lastParseError);
    return;
  }
  else
    enter(root);

  if (lastParseError.isNotEmpty())
    context.warningCallback(T("Variable::createFromFile"), lastParseError);
}

XmlImporter::XmlImporter(ExecutionContext& context, juce::XmlDocument& document)
  : context(context), root(document.getDocumentElement())
{
  String lastParseError = document.getLastParseError();
  if (!root)
  {
    context.errorCallback(T("NetworkClient::messageReceived"), 
                          lastParseError.isEmpty() ? T("Could not parse message") : lastParseError);
    return;
  }
  else
    enter(root);
  
  if (lastParseError.isNotEmpty())
    context.warningCallback(T("NetworkClient::messageReceived"), lastParseError);
}

void XmlImporter::errorMessage(const String& where, const String& what) const
  {context.errorCallback(where, what);}

void XmlImporter::warningMessage(const String& where, const String& what) const
  {context.warningCallback(where, what);}

Variable XmlImporter::load()
{
  if (root->getTagName() == T("lbcpp"))
  {
    loadSharedObjects();
    return loadVariable(root->getChildByName(T("variable")), TypePtr());
  }
  else
    return loadVariable(root, TypePtr());
}

bool XmlImporter::loadSharedObjects()
{
  bool res = true;
  forEachXmlChildElementWithTagName(*getCurrentElement(), child, T("shared"))
  {
    Variable variable = loadVariable(child, TypePtr());
    if (!variable.exists())
    {
      res = false;
      continue;
    }

    if (!variable.isObject())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared variable is not an object"));
      res = false;
      continue;
    }
    String identifier = child->getStringAttribute(T("identifier"));
    if (identifier.isEmpty())
    {
      errorMessage(T("XmlImporter::loadRecursively"), T("Shared object has no identifier"));
      res = false;
      continue;
    }
    sharedObjectsStack.back()[identifier] = variable.getObject();
  }
  return res;
}

TypePtr XmlImporter::loadType(TypePtr expectedType)
{
  juce::XmlElement* elt = getCurrentElement();
  String typeName = elt->getStringAttribute(T("type"), String::empty).replaceCharacters(T("[]"), T("<>"));
  if (typeName.isNotEmpty())
    return typeManager().getType(context, typeName);
  else
  {
    juce::XmlElement* child = elt->getChildByName(T("type"));
    if (child)
    {
      TypePtr res;
      enter(child);
      if (hasAttribute(T("reference")))
        res = getReferencedObject().staticCast<Type>().get();
      else
        res = Type::loadUnnamedTypeFromXml(*this);
      leave();
      return res;
    }
    else
    {
      if (!expectedType)
        context.errorCallback(T("XmlImporter::loadType"), T("Could not find type"));
      return expectedType;
    }
  }
}

ObjectPtr XmlImporter::getReferencedObject() const
{
  const SharedObjectMap& sharedObjects = sharedObjectsStack.back();
  String ref = getStringAttribute(T("reference"));
  SharedObjectMap::const_iterator it = sharedObjects.find(ref);
  if (it == sharedObjects.end())
  {
    errorMessage(T("XmlImporter::getReferencedObject"), T("Could not find shared object reference ") + ref.quoted());
    return ObjectPtr();
  }
  jassert(it->second);
  return it->second;
}

Variable XmlImporter::loadVariable(TypePtr expectedType)
{
  TypePtr type = loadType(expectedType);
  if (!type)
    return Variable();
  
  if (getStringAttribute(T("missing")) == T("true"))
    return Variable::missingValue(type);

  if (hasAttribute(T("reference")))
    return getReferencedObject();
  else
    return Variable::createFromXml(type, *this);
}

Variable XmlImporter::loadVariable(juce::XmlElement* elt, TypePtr expectedType)
{
  enter(elt);
  Variable res = loadSharedObjects() ? loadVariable(expectedType) : Variable();
  leave();
  return res;
}

void XmlImporter::enter(juce::XmlElement* child)
{
  jassert(child);
  stack.push_back(child);
  sharedObjectsStack.push_back(sharedObjectsStack.size() ? sharedObjectsStack.back() : SharedObjectMap());
}

bool XmlImporter::enter(const String& childTagName)
{
  juce::XmlElement* child = getCurrentElement()->getChildByName(childTagName);
  if (!child)
  {
    context.errorCallback(T("XmlImporter::enter"), T("Could not find child ") + childTagName.quoted());
    return false;
  }
  enter(child);
  return true;
}

void XmlImporter::leave()
{
  jassert(stack.size());
  stack.pop_back();
  jassert(sharedObjectsStack.size());
  sharedObjectsStack.pop_back();
}
/*
bool XmlExporter::CompareObjectsDeterministically::operator()(const ObjectPtr& object1, const ObjectPtr& object2) const
{
  if (!object2)
    return false;
  if (!object1)
    return true;
  if (object1->getClass() != object2->getClass())
  {
    jassert(object1->getClass()->getName() != object2->getClass()->getName());
    return object1->getClass()->getName() < object2->getClass()->getName();
  }
  ClassPtr cl = object1->getClass();
  size_t n = cl->getNumMemberVariables();
  for (size_t i = 0; i < n; ++i)
  {
    Variable v1 = object1->getVariable(i);
    Variable v2 = object2->getVariable(i);
    if (v1 == v2)
      continue;
    if (!v2.exists())
      return false;
    if (!v1.exists())
      return true;
    if (v1.isObject())
    {
      bool res1 = (*this)(v1.getObject(), v2.getObject());
      bool res2 = (*this)(v2.getObject(), v1.getObject());
      jassert(!res1 || !res2);
      if (res1 != res2)
        return res1;
      else
      {
        // objects are stricly identic
        return v1.getObject() < v2.getObject();
      }
    }
  }
  return false;
}
*/

/*
** XmlElement
*/
XmlElementPtr XmlElement::createFromXml(juce::XmlElement* element, bool deleteElementOnceConverted)
{
  XmlElementPtr res(new XmlElement(element->getTagName()));
  res->attributes.resize(element->getNumAttributes());
  for (size_t i = 0; i < res->attributes.size(); ++i)
    res->attributes[i] = std::make_pair(element->getAttributeName(i), element->getAttributeValue(i));
  if (element->isTextElement())
    res->text = element->getText();
  res->childElements.reserve(element->getNumChildElements());
  forEachXmlChildElement(*element, child)
    res->childElements.push_back(XmlElement::createFromXml(child, false));
  if (deleteElementOnceConverted)
    delete element;
  return res;
}

juce::XmlElement* XmlElement::createJuceXmlElement() const
{
  if (tagName.isEmpty())
    return juce::XmlElement::createTextElement(text);
  else
  {
    juce::XmlElement* res = new juce::XmlElement(tagName);
    for (size_t i = 0; i < attributes.size(); ++i)
      res->setAttribute(attributes[i].first, attributes[i].second);
    for (size_t i = 0; i < childElements.size(); ++i)
      res->addChildElement(childElements[i]->createJuceXmlElement());
    return res;
  }
}

XmlElementPtr XmlElement::getChildByName(const String& name) const
{
  for (size_t i = 0; i < childElements.size(); ++i)
    if (childElements[i]->getTagName() == name)
      return childElements[i];
  return XmlElementPtr();
}

void XmlElement::setAttribute(const String& name, const String& value)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes[i].second = value;
      return;
    }
  attributes.push_back(std::make_pair(name, value));
}

String XmlElement::getStringAttribute(const String& name, const String& defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second;
  return defaultValue;
}

int XmlElement::getIntAttribute(const String& name, int defaultValue) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return attributes[i].second.getIntValue();
  return defaultValue;
}

bool XmlElement::hasAttribute(const String& name) const
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
      return true;
  return false;
}

void XmlElement::removeAttribute(const String& name)
{
  for (size_t i = 0; i < attributes.size(); ++i)
    if (attributes[i].first == name)
    {
      attributes.erase(attributes.begin() + i);
      return;
    }
}

String XmlElement::getAllSubText() const
{
  String res;
  for (size_t i = 0; i < childElements.size(); ++i)
    if (childElements[i]->isTextElement())
      res += childElements[i]->getText();
  return res;
}

bool XmlElement::loadFromJuceXmlElement(juce::XmlElement* element)
{
  tagName = element->getTagName();
  
  attributes.clear();
  attributes.resize(element->getNumAttributes());
  for (size_t i = 0; i < attributes.size(); ++i)
    attributes[i] = std::make_pair(element->getAttributeName(i), element->getAttributeValue(i));
  if (element->isTextElement())
    text = element->getText();
  
  childElements.clear();
  childElements.reserve(element->getNumChildElements());
  forEachXmlChildElement(*element, child)
    childElements.push_back(XmlElement::createFromXml(child, false));
  return true;
}

bool XmlElement::loadFromXml(XmlImporter& importer)
{
  juce::XmlElement* element = importer.getCurrentElement()->getFirstChildElement();
  return loadFromJuceXmlElement(element);
}