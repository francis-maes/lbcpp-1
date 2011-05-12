/*
** This file is part of the LBC++ library - "Learning Based C++"
** Copyright (C) 2010 by Francis Maes, francis.maes@lip6.fr.
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
| Filename: Stream.h                       | Variable Streams                |
| Author  : Francis Maes                   |                                 |
| Started : 10/07/2010 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#ifndef LBCPP_VARIABLE_STREAM_H_
# define LBCPP_VARIABLE_STREAM_H_

# include "predeclarations.h"
# include "../Core/Function.h"

namespace lbcpp
{

/**
** @class Stream
** @brief Variable stream.
**
** This class represents a finite or infinite stream of variables.
** It is used as a base class for parsers and generators of data.
**
** Stream is particurlaly usefull for large-scale learning
** where the whole dataset does not fit in memory.
** With an Stream, it is possible to apply learning,
** by loading the training examples one at a time.
**
** In order to fit all usages, users can load the whole content
** of an Stream into memory, to have a random access on
** its elements.
*/
class Stream : public Object
{
public:
  Stream(ExecutionContext& context) : context(context) {}
  Stream() : context(*(ExecutionContext* )0) {}

  /**
  ** Returns the type of the elements contained by this stream.
  **
  ** If elements from multiple types are mixed in this stream, this
  ** functions returns the highest common base-type of elements.
  **
  ** @return the type of elements contained by this stream
  */
  virtual TypePtr getElementsType() const = 0;

  /**
  ** Rewind the stream.
  **
  ** @return false if rewind is not supported on this stream.
  */
  virtual bool rewind()
    {return false;}

  /**
  ** Checks if the stream is exhausted.
  **
  ** @return true if the stream is exhausted.
  */
  virtual bool isExhausted() const
    {return true;}

  virtual ProgressionStatePtr getCurrentPosition() const = 0;

  /**
  ** Loads and returns the next element from the stream.
  **
  ** @return a Variable containing the next database element.
  */
  virtual Variable next() = 0;


  /**
  ** Calls next() up to @a maximumCount times and ignores the loaded
  ** objects.
  **
  ** If @a maximumCount == 0, the function calls next() until
  ** the end of the stream. Otherwise, if the end of the stream occurs
  ** before @a maximumCount calls to next(), the function stops. Note
  ** that this function is essentially provided for debugging purpose.
  **
  **
  ** @param maximumCount : iteration steps.
  ** @return False if any errors occurs.
  */
  bool iterate(size_t maximumCount = 0);

  /**
  ** Loads \a maximumCount items (maximum) from the stream and stores
  ** them into memory.
  **
  ** If @a maximumCount == 0, all the items will be loaded.
  **
  ** @param maximumCount : number of item to load.
  **
  ** @return a Vector containing loaded items.
  ** @see Container
  */
  VectorPtr load(size_t maximumCount = 0, bool doProgression = true);

  /**
  ** Applies an Function to this stream.
  **
  ** This function creates a new stream derivated from this one.
  ** Each time next() is called on this stream, a new element
  ** is loaded from this one and @a function is applied on
  ** this element.
  **
  ** @param function : function to apply on elements.
  ** @return a new object stream instance referring to this one.
  ** @see Function
  */
  StreamPtr apply(FunctionPtr function) const;

protected:
  ExecutionContext& context;
};

StreamPtr directoryFileStream(ExecutionContext& context, const File& directory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);
StreamPtr directoryPairFileStream(ExecutionContext& context, const File& mainDirectory, const File& secondDirectory, const String& wildCardPattern = T("*"), bool searchFilesRecursively = false);

/**
 ** @class TextObjectParser
 ** @brief Text object parser.
 **
 ** Base class for parsing text files line by line.
 */
class TextParser : public Stream
{
public:
  /**
   ** Constructor.
   **
   ** @param filename : file name.
   **
   ** @return a TextObjectParser.
   */
  TextParser(ExecutionContext& context, const File& file);
  
  /**
   ** Constructor.
   **
   ** @param newInputStream : an input stream.
   ** The TextObjectParser is responsible for deleting this
   ** stream, when no more used.
   **
   ** @return a TextObjectParser.
   */
  TextParser(ExecutionContext& context, InputStream* newInputStream);
  
  TextParser() {}

  /**
   ** Destructor.
   */
  virtual ~TextParser();
  
  virtual bool rewind()
    {return istr->setPosition(0);}
  
  virtual bool isExhausted() const
    {return istr == NULL;}

  virtual ProgressionStatePtr getCurrentPosition() const;
  
  /**
   ** This function is called at the begging of parsing.
   **
   */
  virtual void parseBegin()
    {}
  
  /**
   ** This function is called to process one line during parsing.
   **
   ** If an object has been fully loaded thanks to this line,
   ** call setResult() to complete the parsing of this object.
   **
   ** @param line : text line.
   **
   ** @return False if parsing of the line failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   ** @see setResult
   */
  virtual bool parseLine(const String& line) = 0;
  
  /**
   ** This function is called at the end of the parsing.
   **
   ** This function may call setResult(), if an object
   ** has been fully loaded thanks to the end of parsing.
   **
   ** @return False if end of parsing failed. In this case
   **  inherited class are responsible for throwing an error
   **  to the ErrorManager.
   ** @see setResult
   */
  virtual bool parseEnd()
    {return true;}
  
  /**
   ** Loads the next object from the stream.
   **
   ** When called the first time next() calls parseBegin().
   ** In order to load the next object from the stream, next()
   ** calls parseLine() for each line to parse until
   ** a result is set through a call to setResult(). This result
   ** is then returned. When reaching end-of-file, this
   ** function calls parseEnd().
   **
   ** @return a pointer on the next parsed object or Variable()
   ** if there are no more object in the stream.
   */
  virtual Variable next();
  
protected:
  /**
   ** currentObject setter.
   **
   ** This function may be called by parseLine() and by parseEnd()
   ** to transmit the lastly parsed object, which will be returned
   ** by next().
   **
   ** @param object : object pointer.
   */
  void setResult(const Variable& result)
    {currentResult = result;}
  
  /**
   ** Tokenizes a line.
   **
   ** @param line : example text line.
   ** @param columns : item container.
   ** @param separators : item text separators (" " and "\t" by default).
   */
  static void tokenize(const String& line,
                       std::vector< String >& columns,
                       const juce::tchar* separators = T(" \t"));
  
private:
  Variable currentResult;      /*!< The current Variable. */
  InputStream* istr;           /*!< A pointer to the current stream. */
};
  

/**
** @class LearningDataTextParser
** @brief Base class for data parsers using libSVM inspired formats.
**
** @see classificationExamplesParser, regressionExamplesParser
**
*/
class LearningDataTextParser : public TextParser
{
public:
  LearningDataTextParser(ExecutionContext& context, const File& file)
    : TextParser(context, file) {}

  /**
  ** Constructor
  **
  ** @param newInputStream : new input stream. This object is
  ** responsible for deleting the input stream, when no longer used.
  ** @param features : feature dictionary.
  **
  ** @return a @a LearningDataObjectParser.
  */
  LearningDataTextParser(ExecutionContext& context, InputStream* newInputStream)
    : TextParser(context, newInputStream) {}

  LearningDataTextParser() {}
  /**
  ** This function is called to parse an empty line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseEmptyLine()
    {return true;}

  /**
  ** This function is called to parse a line containing data.
  **
  ** @param columns : tokenized data line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseDataLine(const std::vector<String>& columns)
    {return false;}

  /**
  ** This function is called to parse a comment line (starting by '#').
  **
  ** @param comment : the comment (the line without '#')
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseCommentLine(const String& comment)
    {return true;}

  /**
  ** This function is called when parsing finishes.
  **
  ** By default, end-of-file is considered as an empty line.
  **
  ** @return False if parsing failed. In this case
  **  inherited class are responsible for throwing an error
  **  to the ErrorManager.
  */
  virtual bool parseEnd()
    {return parseEmptyLine();}

  /**
  ** Parses one line.
  **
  ** The following rules are applied:
  ** - if the line is empty, parseLine() calls parseEmptyLine()
  ** - if the line starts by '#', it calls parseCommentLine()
  ** - otherwise, it tokenizes the line and calls parseDataLine()
  **
  ** @param line : text line.
  **
  ** @return a boolean.
  ** @see LearningDataObjectParser::parseEmptyLine
  ** @see LearningDataObjectParser::parseCommentLine
  ** @see LearningDataObjectParser::parseDataLine
  */
  virtual bool parseLine(const String& line);

protected:
  /**
  ** Parses a list of features.
  **
  ** Feature lists have the following grammar:
  ** \verbatim featureList ::= feature featureList | feature \endverbatim
  **
  ** @param columns : the columns to parse.
  ** @param firstColumn : start column number.
  */
  ObjectPtr parseFeatureList(DynamicClassPtr cl, const std::vector<String>& columns, size_t firstIndex) const;


  /**
  ** Parses a feature
  **
  ** Features have the following grammar:
  ** \verbatim feature ::= featureId : featureValue | featureId \endverbatim
  **
  ** @param str : the string to parse.
  ** @param featureId : feature ID container.
  ** @param featureValue : feature value container.
  **
  ** @return False if any error occurs.
  */
  static bool parseFeature(const String& str, String& featureId, double& featureValue);

  /**
  ** Parses a feature identifier
  **
  ** Feature identifiers have the following grammar:
  ** \verbatim featureId ::= name . featureId  | name \endverbatim
  **
  ** @param identifier : the string to parse.
  ** @param path : feature identifier path container.
  **
  ** @return True.
  */
  static bool parseFeatureIdentifier(const String& identifier, std::vector<String>& path)
    {tokenize(identifier, path, T(".")); return true;}
};

typedef ReferenceCountedObjectPtr<LearningDataTextParser> LearningDataTextParserPtr;

extern LearningDataTextParserPtr classificationDataTextParser(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr outputLabels);
extern LearningDataTextParserPtr multiLabelClassificationDataTextParser(ExecutionContext& context, const File& file, DynamicClassPtr inputClass, DefaultEnumerationPtr outputLabels);

}; /* namespace lbcpp */

#endif // !LBCPP_VARIABLE_STREAM_H_