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
| Filename: FeatureVisitor.h               | Feature visitor                 |
| Author  : Francis Maes                   |                                 |
| Started : 22/03/2010 16:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_FEATURE_VISITOR_H_
# define LBCPP_FEATURE_VISITOR_H_

# include "FeatureGenerator.h"

namespace lbcpp
{

/**
** @class FeatureVisitor
** @brief FeatureGenerator visitor.
*/
class FeatureVisitor : public Object
{
public:
  /**
  ** This function is executed each time the visitor enters into a new
  ** FeatureGenerator scope.
  **
  ** @param dictionary : feature dictionary of the current FeatureGenerator.
  ** @param index : current scope index.
  **
  ** @return a boolean.
  */
  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index) = 0;

  /**
  ** Adds a new feature. A pair <@em name, @a value> where @em name is
  ** composed by a scope prefix and the given feature name.
  **
  ** Example: we are in the scope "contact" and we want to add a
  ** feature "firstname" with the value "Paul". The complete variable
  ** name will be: "contact.firstname" and its value "Paul".
  **
  ** @param dictionary : feature dictionary of the current FeatureGenerator.
  ** @param index : feature index.
  ** @param value : feature value.
  */
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value) = 0;

  /**
  ** Calls another FeatureGenerator and puts its result into a sub-scope
  **
  ** @param dictionary : feature dictionary of the current FeatureGenerator.
  ** @param scopeIndex : scope index.
  ** @param featureGenerator : feature generator to call.
  */
  virtual void featureCall(FeatureDictionaryPtr dictionary, size_t scopeIndex, FeatureGeneratorPtr featureGenerator);

  /**
  ** Calls another FeatureGenerator.
  **
  ** @param featureGenerator : feature generator to call.
  */
  virtual void featureCall(FeatureGeneratorPtr featureGenerator);

  /**
  ** This function is executed each time the visitor leaves a scope.
  */
  virtual void featureLeave() = 0;
};

class PathBasedFeatureVisitor : public FeatureVisitor
{
public:
  virtual void featureSense(const std::vector<size_t>& path, const String& name, double value) = 0;

  virtual bool featureEnter(FeatureDictionaryPtr dictionary, size_t index);  
  virtual void featureSense(FeatureDictionaryPtr dictionary, size_t index, double value);
  virtual void featureLeave();

private:
  std::vector<size_t> currentPath;
  std::vector<String> currentName;

  static String appendName(const String& path, const String& name);
};

}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_H_
