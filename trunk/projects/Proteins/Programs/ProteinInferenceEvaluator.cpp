/*-----------------------------------------.---------------------------------.
| Filename: ProteinInferenceEvaluator.cpp  | Evaluator                       |
| Author  : Francis Maes                   |                                 |
| Started : 26/04/2010 16:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/Evaluation/ProteinEvaluationCallback.h"
#include "Protein/Inference/ProteinInference.h"
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& fileOrDirectory, size_t maxCount = 0)
{
  if (fileOrDirectory.isDirectory())
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory, T("*.protein"))
      ->load(maxCount)
      ->randomize()
      ->apply(new ProteinToInputOutputPairFunction());

    ObjectContainerPtr resPair = directoryObjectStream(fileOrDirectory, T("*.proteinPair"))
      ->load(maxCount - res->size())
      ->randomize()
      ->apply(new ComputeMissingFieldsOfProteinPairFunction());

    return res;
  }

  if (fileOrDirectory.getFileExtension() == T(".protein")) 
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory.getParentDirectory(), fileOrDirectory.getFileName())
    ->load()
    ->apply(new ProteinToInputOutputPairFunction());
    if (res->size())
      return res;
  }
  
  if (fileOrDirectory.getFileExtension() == T(".proteinPair"))
  {
    ObjectContainerPtr res = directoryObjectStream(fileOrDirectory.getParentDirectory(), fileOrDirectory.getFileName())
    ->load()
    ->apply(new ComputeMissingFieldsOfProteinPairFunction());
    if (res->size())
      return res;
  }
  return ObjectContainerPtr();
}

InferencePtr addBreakToInference(InferencePtr inference, InferencePtr lastStepBeforeBreak)
  {return callbackBasedDecoratorInference(inference->getName() + T(" breaked"), inference, cancelAfterStepCallback(lastStepBeforeBreak));}

class SaveOutputInferenceCallback : public InferenceCallback
{
public:
  SaveOutputInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      File f = directory.getChildFile(output->getName() + T(".") + extension);
      std::cout << "Save " << f.getFileName() << "." << std::endl;

      output->saveToFile(f);
    }
  }

private:
  File directory;
  String extension;
};

class SaveProteinPairInferenceCallback : public InferenceCallback
{
public:
  SaveProteinPairInferenceCallback(const File& directory, const String& extension)
    : directory(directory), extension(extension) {}

  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
    {
      File f = directory.getChildFile(output->getName() + T(".") + extension);
      std::cout << "Save Pair " << f.getFileName() << "." << std::endl;

      ProteinPtr inputProtein = input.dynamicCast<Protein>()->clone();
      jassert(inputProtein);

      ProteinPtr outputProtein = output.dynamicCast<Protein>();
      jassert(outputProtein);
      

      std::vector<String> keys = outputProtein->getKeys();
      for (size_t i = 0; i < keys.size(); ++i)
        inputProtein->setObject(outputProtein->getObject(keys[i]));

      inputProtein->computeMissingFields();
      
      ProteinPtr supervisionProtein = supervision.dynamicCast<Protein>();
      jassert(supervisionProtein);
      supervisionProtein->computeMissingFields();

      ObjectPairPtr(new ObjectPair(inputProtein, supervision))->saveToFile(f);
    }
  }

private:
  File directory;
  String extension;
};

class PrintDotForEachExampleInferenceCallback : public InferenceCallback
{
public:
  virtual void postInferenceCallback(InferenceStackPtr stack, ObjectPtr input, ObjectPtr supervision, ObjectPtr& output, ReturnCode& returnCode)
  {
    if (stack->getDepth() == 1)
      std::cout << "." << std::flush;
  }
};

int main(int argc, char** argv)
{
  declareProteinClasses();

  if (argc < 4)
  {
    std::cerr << "Usage: " << argv[0] << " modelDirectory proteinFileOrDirectory mode [param]" << std::endl;
    std::cerr << "Possible values for 'mode': All StepByStep AllSave or AllSavePair" << std::endl;
    std::cerr << "Param is only for mode AllSave and AllSavePair and is the output directory" << std::endl;
    return 1;
  }

  File cwd = File::getCurrentWorkingDirectory();
  File modelDirectory = cwd.getChildFile(argv[1]);
  File proteinsFileOrDirectory = cwd.getChildFile(argv[2]);
  String mode = argv[3];

  if (!proteinsFileOrDirectory.exists())
  {
    std::cerr << proteinsFileOrDirectory.getFullPathName() << " does not exists." << std::endl;
    return 1;
  }

  File outputDirectory;
  if (mode == T("AllSave") || mode == T("AllSavePair"))
  {
    if (argc > 4)
      outputDirectory = cwd.getChildFile(argv[4]);
    else
    {
      std::cerr << "Missing 'output directory' argument" << std::endl;
      return 1;
    }
    if (!outputDirectory.exists() && !outputDirectory.createDirectory())
    {
      std::cerr << "Could not create output directory " << outputDirectory.getFullPathName() << std::endl;
      return 1;
    }
  }

  std::cout << "Loading data... " << std::flush;
  ObjectContainerPtr proteins = loadProteins(proteinsFileOrDirectory);
  if (!proteins)
    return 2;
  std::cout << proteins->size() << " protein(s)." << std::endl;

  std::cout << "Loading inference... " << std::flush;
  ProteinInferencePtr inference = new ProteinInference();
  inference->loadSubInferencesFromDirectory(modelDirectory);
  if (!inference->getNumSubInferences())
  {
    std::cerr << "Could not find any inference step in directory " << modelDirectory.getFullPathName() << std::endl;
    return 3;
  }
  std::cout << inference->getNumSubInferences() << " step(s)." << std::endl;

  InferenceContextPtr inferenceContext = singleThreadedInferenceContext();
  ProteinEvaluationCallbackPtr evaluationCallback = new ProteinEvaluationCallback();
  inferenceContext->appendCallback(evaluationCallback);
  inferenceContext->appendCallback(new PrintDotForEachExampleInferenceCallback());
  InferenceResultCachePtr cache = new InferenceResultCache();

  if (mode == T("All") || mode == T("AllSave") || mode == T("AllSavePair"))
  {
    if (mode == T("AllSave"))
      inferenceContext->appendCallback(new SaveOutputInferenceCallback(outputDirectory, T("protein")));
    if (mode == T("AllSavePair"))
      inferenceContext->appendCallback(new SaveProteinPairInferenceCallback(outputDirectory, T("proteinPair")));
    std::cout << "Making predictions..." << std::endl;
    inferenceContext->runWithSupervisedExamples(inference, proteins);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  else if (mode == T("StepByStep"))
  {
    inferenceContext->appendCallback(cacheInferenceCallback(cache, inference));
    std::cout << std::endl;
    for (size_t i = 0; i < inference->getNumSubInferences(); ++i)
    {
      InferencePtr decoratedInference;
      if (i < inference->getNumSubInferences() - 1)
      {
        std::cout << "Making predictions for steps 1.." << (i+1) << std::endl;
        decoratedInference = addBreakToInference(inference, inference->getSubInference(i));
      }
      else
      {
        std::cout << "Making predictions for all steps" << std::endl;
        decoratedInference = inference;
      }
      
      inferenceContext->runWithSupervisedExamples(decoratedInference, proteins);
      std::cout << evaluationCallback->toString() << std::endl << std::endl;
    }
  }
  else if (mode == T("AllSave") || mode == T("AllSavePair"))
  {
    std::cout << "Making predictions..." << std::endl;
    inferenceContext->runWithSupervisedExamples(inference, proteins);
    std::cout << evaluationCallback->toString() << std::endl << std::endl;
  }
  else
  {
    std::cerr << "Unrecognized mode: " << mode.quoted() << std::endl;
    return 1;
  }
  return 0;
}
