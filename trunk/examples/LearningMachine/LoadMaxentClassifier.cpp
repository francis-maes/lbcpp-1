/*-----------------------------------------.---------------------------------.
| Filename: LoadMaxentClassifier.cpp       | An example to show how to load  |
| Author  : Francis Maes                   |  a classifier from a file       |
| Started : 08/06/2009 21:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#include <lbcpp/lbcpp.h>
using namespace lbcpp;

int main(int argc, char* argv[])
{
  /*
  ** Load a Classifier from file "classifier.model"
  */
  ClassifierPtr classifier = loadClassifier(File::getCurrentWorkingDirectory().getChildFile("classifier.model"));
  assert(classifier);

  File dataDirectory = File::getCurrentWorkingDirectory().getChildFile("../Data/Classification");

  /*
  ** Evaluate training accuracy
  */
  double accuracy = classifier->evaluateAccuracy(
    classificationExamplesParser(dataDirectory.getChildFile("small.train"), classifier->getInputDictionary(), classifier->getLabels()));
  std::cout << "Training Accuracy: " << accuracy * 100 << "%." << std::endl;

  /*
  ** Evaluate testing accuracy
  */
  accuracy = classifier->evaluateAccuracy(
    classificationExamplesParser(dataDirectory.getChildFile("small.test"), classifier->getInputDictionary(), classifier->getLabels()));
  std::cout << "Testing Accuracy: " << accuracy * 100 << "%." << std::endl;
  return 0;
}
