/*-----------------------------------------.---------------------------------.
| Filename: ARFFDataParser.h               | ARFF Data Parser                |
| Author  : Julien Becker                  |                                 |
| Started : 10/07/2010 15:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_
# define LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_

# include <lbcpp/Data/Stream.h>

namespace lbcpp
{

class ARFFDataParser : public TextParser
{
public:
  ARFFDataParser(ExecutionContext& context, const File& file)
    : TextParser(context, file), shouldReadData(false) {}

  ARFFDataParser(ExecutionContext& context, InputStream* newInputStream)
    : TextParser(context, newInputStream), shouldReadData(false) {}

  ARFFDataParser() {}
  
  virtual bool parseLine(const String& line);
  
protected:
  bool shouldReadData;
  std::vector<TypePtr> attributesType;
  TypePtr supervisionType;

  bool parseAttributeLine(const String& line);
  bool parseEnumerationAttributeLine(const String& line);
  virtual bool checkSupervisionType() const = 0;
  bool parseDataLine(const String& line);
  bool parseSparseDataLine(const String& line);
  virtual Variable finalizeData(Variable data) const
    {return data;}
};

class RegressionARFFDataParser : public ARFFDataParser
{
public:
  RegressionARFFDataParser(ExecutionContext& context, const File& file)
    : ARFFDataParser(context, file)
    {elementsType = pairClass(variableVectorClass, doubleType);}

  RegressionARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}
  
protected:
  TypePtr elementsType;
  
  virtual bool checkSupervisionType() const
    {return context.checkInheritance(supervisionType, doubleType);}
};

class BinaryClassificationARFFDataParser : public ARFFDataParser
{
public:
  BinaryClassificationARFFDataParser(ExecutionContext& context, const File& file)
    : ARFFDataParser(context, file)
    {elementsType = pairClass(variableVectorClass, booleanType);}

  BinaryClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  
  virtual bool checkSupervisionType() const
  {
    if (!context.checkInheritance(supervisionType, enumValueType))
      return false;
    
    EnumerationPtr enumType = supervisionType.dynamicCast<Enumeration>();
    if (enumType->getNumElements() != 2)
    {
      context.errorCallback(T("BinaryClassificationARFFDataParser::checkSupervisionType"), T("Too many possible output"));
      return false;
    }
    return true;
  }
  
  virtual Variable finalizeData(Variable data) const
  {
    PairPtr p = data.getObjectAndCast<Pair>();
    p->setSecond(p->getSecond().getInteger() == 1);
    return p;
  }
};

class ClassificationARFFDataParser : public ARFFDataParser
{
public:
  ClassificationARFFDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr labels)
    : ARFFDataParser(context, file), labels(labels)
    {elementsType = pairClass(variableVectorClass, labels);}

  ClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  DefaultEnumerationPtr labels;
  
  virtual bool checkSupervisionType() const
  {
    if (!context.checkInheritance(supervisionType, enumValueType))
      return false;
    // copy supervision enumeration to labels
    EnumerationPtr enumType = supervisionType.dynamicCast<Enumeration>();
    ClassificationARFFDataParser* thisPtr = const_cast<ClassificationARFFDataParser*>(this);
    for (size_t i = 0; i < enumType->getNumElements(); ++i)
      thisPtr->labels->addElement(context, enumType->getElement(i)->getName());
    thisPtr->supervisionType = labels;
    return true;
  }
};

class MultiLabelClassificationARFFDataParser : public ARFFDataParser
{
public:
  MultiLabelClassificationARFFDataParser(ExecutionContext& context, const File& file, DefaultEnumerationPtr labels)
    : ARFFDataParser(context, file), labels(labels)
    {elementsType = pairClass(variableVectorClass, sparseDoubleVectorClass(labels, probabilityType));}

  MultiLabelClassificationARFFDataParser() {}
  
  virtual TypePtr getElementsType() const
    {return elementsType;}

protected:
  TypePtr elementsType;
  DefaultEnumerationPtr labels;
  
  virtual bool checkSupervisionType() const
  {
    if (!context.checkInheritance(supervisionType, enumValueType))
      return false;
    // copy supervision enumeration to labels
    EnumerationPtr enumType = supervisionType.dynamicCast<Enumeration>();
    MultiLabelClassificationARFFDataParser* thisPtr = const_cast<MultiLabelClassificationARFFDataParser*>(this);
    for (size_t i = 0; i < enumType->getNumElements(); ++i)
      thisPtr->labels->addElement(context, enumType->getElement(i)->getName());
    const_cast<MultiLabelClassificationARFFDataParser*>(this)->supervisionType = labels;
    return true;
  }
};

};

#endif // !LBCPP_VARIABLE_STREAM_ARFF_DATA_PARSER_H_