/*-----------------------------------------.---------------------------------.
| Filename: RegressionExamplesParser.h     | Default parser for              |
| Author  : Francis Maes                   |   regression examples           |
| Started : 22/06/2009 18:22               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_
# define LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_

# include <lbcpp/ObjectStream.h>

namespace lbcpp
{

class RegressionExamplesParser : public LearningDataObjectParser
{
public:
  RegressionExamplesParser(const std::string& filename, FeatureDictionaryPtr features)
    : LearningDataObjectParser(filename, features) {}

  virtual std::string getContentClassName() const
    {return "RegressionExample";}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    double y;
    if (!TextObjectParser::parse(columns[0], y))
      return false;
    SparseVectorPtr x;
    if (!parseFeatureList(columns, 1, x))
      return false;
    setResult(new RegressionExample(x, y));
    return true;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_OBJECT_STREAM_REGRESSION_EXAMPLES_PARSER_H_