/*-----------------------------------------.---------------------------------.
| Filename: StringDescriptionVisitor.hpp   | A visitor to describe vectors   |
| Author  : Francis Maes                   |  as strings                     |
| Started : 27/02/2009 22:23               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_
# define LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_

# include "FeatureVisitorStatic.hpp"

namespace lbcpp {
namespace impl {

class StringDescriptionVisitor : public FeatureVisitor<StringDescriptionVisitor>
{
public: 
  StringDescriptionVisitor() : indent(0) {}

  bool featureEnter(lbcpp::FeatureDictionaryPtr dictionary, size_t number)
  {
   //res += "DICO = " + dictionary->getName() + " (" + lbcpp::toString((int)dictionary.get()) + ")\n";
    flushCurrentFeatures();
    addIndent(indent);
    res += dictionary->getScopes()->getString(number) + "\n";
    ++indent;
    return true;
  }

  void featureSense(lbcpp::FeatureDictionaryPtr dictionary, size_t number, double value = 1.0)
  {
    if (currentFeatures.size())
      currentFeatures += ", ";
    currentFeatures += dictionary->getFeatures()->getString(number) + " = " + lbcpp::toString(value);
  }
  
  void featureLeave()
  {
    flushCurrentFeatures();
    --indent;
  }

  void flushCurrentFeatures()
  {
    if (currentFeatures.size())
    {
      addIndent(indent);
      res += currentFeatures + "\n";
      currentFeatures = "";
    }
  }

  std::string getResult() const
  {
    if (currentFeatures.size() && !res.size())
      return "{" + currentFeatures + "}";
    if (res.size() && !currentFeatures.size())
      return res;
    if (!res.size() && !currentFeatures.size())
      return "<empty>";
    return res + "\n" + currentFeatures;
  }

private:
  std::string currentFeatures;
  std::string res;
  int indent;

  inline void addIndent(int indent)
    {for (int i = 0; i < indent; ++i) res += "  ";}
};

}; /* namespace impl */
}; /* namespace lbcpp */

#endif // !LBCPP_FEATURE_VISITOR_STRING_DESCRIPTION_HPP_