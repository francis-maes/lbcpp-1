/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2009 by Francis Maes, francis.maes@lip6.fr.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*-----------------------------------------.---------------------------------.
| Filename: XmlSerialisation.h             | Xml Importer/Exporter           |
| Author  : Francis Maes                   |                                 |
| Started : 26/08/2010 17:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CORE_XML_SERIALISATION_H_
# define LBCPP_CORE_XML_SERIALISATION_H_

# include "Variable.h"

namespace lbcpp
{

class XmlExporter
{
public:
  XmlExporter(ExecutionContext& context, const String& rootTag = T("lbcpp"), int version = 100);
  ~XmlExporter();

  bool saveToFile(const File& file);
  String toString();

  XmlElement* getCurrentElement();

  void saveVariable(const String& name, const Variable& variable, TypePtr expectedType);
  void saveElement(size_t index, const Variable& variable, TypePtr expectedType);

  void enter(const String& tagName, const String& name = String::empty);
  void writeType(TypePtr type);
  void leave();

  void addTextElement(const String& text)
    {getCurrentElement()->addTextElement(text);}

  template<class TT>
  void setAttribute(const String& name, const TT& value)
    {getCurrentElement()->setAttribute(name, value);}

  void addChildElement(XmlElement* elt)
    {getCurrentElement()->addChildElement(elt);}

  void flushSave();

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  XmlElement* root;
  std::vector<XmlElement* > currentStack;

  struct SavedObject
  {
    SavedObject(const SavedObject& other)
      : identifier(other.identifier), object(other.object), elt(other.elt),
        references(other.references), dependencies(other.dependencies), ordered(other.ordered) {}
    SavedObject() : elt(NULL), ordered(true) {}

    String identifier;
    ObjectPtr object;
    XmlElement* elt;
    std::vector<XmlElement* > references; // XmlElements refering to this object
    std::set<size_t> dependencies; // dependencies on other savedObjects 
    bool ordered;
  };

  std::vector<SavedObject> savedObjects;
  typedef std::map<ObjectPtr, size_t> SavedObjectsMap;
  SavedObjectsMap savedObjectsMap;
  std::set<size_t> sharedObjectsIndices;
  std::set<size_t> currentlySavedObjects;

  void makeSharedObjectsSaveOrder(const std::set<size_t>& sharedObjectsIndices, std::vector<size_t>& res);

  void resolveSingleObjectReference(SavedObject& savedObject);
  void resolveSharedObjectReferences(SavedObject& savedObject);

  static String makeUniqueIdentifier(ObjectPtr object, std::set<String>& identifiers);

  void writeName(const String& name);
  void writeVariable(const Variable& variable, TypePtr expectedType);
  void writeObject(const ObjectPtr& object, TypePtr expectedType);
};

class XmlImporter
{
public:
  XmlImporter(ExecutionContext& context, const File& file);
  XmlImporter(ExecutionContext& context, juce::XmlDocument& document);
  ~XmlImporter()
    {if (root) delete root;}

  bool isOpened() const
    {return root != NULL;}

  void errorMessage(const String& where, const String& what) const;
  void warningMessage(const String& where, const String& what) const;

  typedef std::map<String, ObjectPtr> SharedObjectMap;

  Variable load();

  XmlElement* getCurrentElement() const
    {return stack.back();}

  String getAllSubText() const
    {return getCurrentElement()->getAllSubText();}

  bool hasAttribute(const String& attributeName) const
    {return getCurrentElement()->hasAttribute(attributeName);}

  bool getBoolAttribute(const String& attributeName, bool defaultResult = 0) const
    {return getCurrentElement()->getBoolAttribute(attributeName, defaultResult);}

  int getIntAttribute(const String& attributeName, int defaultResult = 0) const
    {return getCurrentElement()->getIntAttribute(attributeName, defaultResult);}

  double getDoubleAttribute(const String& attributeName, double defaultResult = 0.0) const
    {return getCurrentElement()->getDoubleAttribute(attributeName, defaultResult);}

  String getStringAttribute(const String& attributeName, const String& defaultResult = String::empty) const
    {return getCurrentElement()->getStringAttribute(attributeName, defaultResult);}

  Variable loadVariable(XmlElement* child, TypePtr expectedType);

  void enter(XmlElement* child);
  bool enter(const String& childTagName);
  TypePtr loadType(TypePtr expectedType);
  void leave();

  ExecutionContext& getContext()
    {return context;}

private:
  ExecutionContext& context;
  XmlElement* root;

  std::vector<XmlElement* > stack;
  std::vector<SharedObjectMap> sharedObjectsStack;

  bool loadSharedObjects();
  Variable loadVariable(TypePtr expectedType);
  ObjectPtr getReferencedObject() const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_CORE_XML_SERIALISATION_H_
