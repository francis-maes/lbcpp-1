#ifndef LBCPP_BINARIZER_PROGRAM_PROGRAM_H_
# define LBCPP_BINARIZER_PROGRAM_PROGRAM_H_

# include <lbcpp/lbcpp.h>
# include "MNISTImage.h"
# include "MatlabFileParser.h"

namespace lbcpp
{

class SaveMNISTImageAsBinaryFunction : public Function
{
public:
  SaveMNISTImageAsBinaryFunction(const File& output)
  {
    if (output.exists())
      output.deleteFile();
    o = output.createOutputStream();
  }
  
  ~SaveMNISTImageAsBinaryFunction()
    {delete o;}
  
  virtual TypePtr getInputType() const
    {return mnistImageClass;}
  
  virtual TypePtr getOutputType(TypePtr inputType) const
    {return inputType;}
  
  virtual Variable computeFunction(const Variable& input, MessageCallback& callback) const
  {
    jassert(input.isObject());
    MNISTImagePtr image = input.getObjectAndCast<MNISTImage>();
    jassert(image);
    Variable digit = image->getDigit();
    o->writeBool(!digit.isMissingValue());
    const std::vector<double>& pixels = image->getPixels();
    for (size_t i = 0; i < MNISTImage::numPixels; ++i)
      o->writeDouble(pixels[i]);
    if (!digit.isMissingValue())
      o->writeInt(digit.getInteger());
    return input;
  }
  
private:
  OutputStream* o;
};
  
class BinarizerProgram : public Program
{
public:
  virtual String toString() const
    {return T("Convert text file to binary file.");}
  
  virtual int runProgram(MessageCallback& callback)
  {
    jassert(inputFile != File::nonexistent);
    if (outputFile == File::nonexistent)
      outputFile = File::getCurrentWorkingDirectory().getChildFile(T("data.bin"));
    
    ContainerPtr data = StreamPtr(new MatlabFileParser(inputFile))->load()->apply(new SaveMNISTImageAsBinaryFunction(outputFile), false);
    return 0;
  }
  
protected:
  friend class BinarizerProgramClass;
  
  File inputFile;
  File outputFile;
};

};

#endif // !LBCPP_BINARIZER_PROGRAM_PROGRAM_H_