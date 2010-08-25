/*-----------------------------------------.---------------------------------.
| Filename: Utilities.cpp                  | Utilities                       |
| Author  : Francis Maes                   |                                 |
| Started : 06/03/2009 17:11               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Data/Utilities.h>
#include <lbcpp/Utilities/ProgressCallback.h>
#include <lbcpp/Function/IterationFunction.h>
#include <iostream>
#include <fstream>
using namespace lbcpp;

static std::string trimDigitsLeft(const std::string& str, size_t i = 0)
{
  for (; i < str.length() && isdigit(str[i]); ++i);
  return str.substr(i);
}

static std::string trimAlphaLeft(const std::string& str, size_t i = 0)
{
  for (; i < str.length() && isalpha(str[i]); ++i);
  return str.substr(i);
}

String lbcpp::getTypeName(const std::type_info& info)
{
  std::string res = info.name();
#ifdef JUCE_WIN32
  size_t n = res.find("::");
  return res.substr(n == std::string::npos ? strlen("class ") : n + 2).c_str();
#else // linux or macos x
  bool hasNamespace = res[0] == 'N';
  if (hasNamespace)
    res = trimAlphaLeft(trimDigitsLeft(res, 1));
  res = trimDigitsLeft(res);
  if (hasNamespace)
    res = res.substr(0, res.length() - 1);
  return res.c_str();
#endif
}

/*
** ProgressCallback
*/
class ConsoleProgressCallback : public ProgressCallback
{
public:
  virtual void progressStart(const String& description)
    {std::cout << "=============== " << (const char* )description << " ===============" << std::endl;}
    
  // return false to stop the task
  virtual bool progressStep(const String& description, double iteration, double totalIterations = 0)
  {
    std::cout << "Step '" << (const char* )description << "' iteration = " << iteration;
    if (totalIterations)
      std::cout << " / " << totalIterations;
    std::cout << std::endl;
    return true;
  }
    
  virtual void progressEnd()
    {std::cout << "===========================================" << std::endl;}
};

ProgressCallbackPtr lbcpp::consoleProgressCallback()
{
  static ProgressCallbackPtr consoleProgressCallback = new ConsoleProgressCallback();
  return consoleProgressCallback;
}

/*
** ErrorHandler
*/
class DefaultErrorHandler : public ErrorHandler
{
public:
  virtual void errorMessage(const String& where, const String& what)
    {std::cerr << "Error in '" << (const char* )where << "': " << (const char* )what << "." << std::endl; jassert(false);}
    
  virtual void warningMessage(const String& where, const String& what)
    {std::cerr << "Warning in '" << (const char* )where << "': " << (const char* )what << "." << std::endl;}
};

static DefaultErrorHandler defaultErrorHandler;

ErrorHandler* ErrorHandler::instance = &defaultErrorHandler;

void ErrorHandler::setInstance(ErrorHandler& handler)
{
  instance = &handler;
}
