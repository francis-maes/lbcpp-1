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

#ifndef LBCPP_DATA_XML_SERIALISATION_H_
# define LBCPP_DATA_XML_SERIALISATION_H_

# include "Variable.h"

namespace lbcpp
{

class XmlExporter
{
public:
  XmlExporter(const String& rootTag = T("lbcpp"), int version = 100);
  ~XmlExporter();

  bool saveToFile(const File& file, MessageCallback& callback);

  XmlElement* getCurrentElement();

  void saveVariable(const String& name, const Variable& variable);
  void saveElement(size_t index, const Variable& variable);

  void enter(const String& tagName, const String& name = String::empty);
  void leave();

  void addTextElement(const String& text)
    {getCurrentElement()->addTextElement(text);}

  template<class TT>
  void setAttribute(const String& name, const TT& value)
    {getCurrentElement()->setAttribute(name, value);}

  void addChildElement(XmlElement* elt)
    {getCurrentElement()->addChildElement(elt);}

  void flushSave();

private:
  XmlElement* root;
  std::vector<XmlElement* > currentStack;

  struct SavedObject
  {
    String identifier;
    ObjectPtr object;
    XmlElement* elt;
    std::set<XmlElement* > references;
  };

  typedef std::map<ObjectPtr, SavedObject> SavedObjectMap;
  SavedObjectMap savedObjects;
  std::map<XmlElement* , SavedObject* > unresolvedLinks;
  std::set<XmlElement* > solvedLinks;
  
  static String makeUniqueIdentifier(ObjectPtr object, std::set<String>& identifiers);

  void resolveLink(XmlElement* xml, std::map<ObjectPtr, int>& referencedObjects, std::set<String>& identifiers);
  void resolveChildLinks(XmlElement* xml, std::map<ObjectPtr, int>& referencedObjects, std::set<String>& identifiers);

  void writeName(const String& name);
  void writeVariable(const Variable& variable);
  void writeObject(ObjectPtr object);
};

}; /* namespace lbcpp */

#endif // !LBCPP_DATA_XML_SERIALISATION_H_
