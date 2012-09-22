/*-----------------------------------------.---------------------------------.
| Filename: ProteinStatisticsWorkUnit.h    | Protein Statistics Work Unit    |
| Author  : Julien Becker                  |                                 |
| Started : 24/06/2011 12:05               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "Data/Protein.h"

#include <lbcpp/UserInterface/PieChartComponent.h>
#include <lbcpp/UserInterface/HistogramComponent.h>

namespace lbcpp
{

class ProteinStatisticsWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    if (inputDirectory == File::nonexistent)
      inputDirectory = File::getCurrentWorkingDirectory();

    ContainerPtr trainingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/train/")), 0, T("Loading training proteins"));
    ContainerPtr validationProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/validation/")), 0, T("Loading validation proteins"));
    ContainerPtr testingProteins = Protein::loadProteinsFromDirectory(context, File(inputDirectory.getFullPathName() + T("/test/")), 0, T("Loading testing proteins"));

    if (!trainingProteins->getNumElements() && !validationProteins->getNumElements() && !testingProteins->getNumElements())
    {
      context.informationCallback(T("No proteins"));
      return false;
    }

    VectorPtr allProteins = vector(proteinClass, trainingProteins->getNumElements() + validationProteins->getNumElements() + testingProteins->getNumElements());
    size_t index = 0;
    for (size_t i = 0; i < trainingProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, trainingProteins->getElement(i));
    for (size_t i = 0; i < validationProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, validationProteins->getElement(i));
    for (size_t i = 0; i < testingProteins->getNumElements(); ++i, ++index)
      allProteins->setElement(index, testingProteins->getElement(i));

    computeProteinStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, ss3Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, ss8Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, sa20Target);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, drTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, stalTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cbpTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cbsTarget);
    computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, dsbTarget);
    //computeStatistics(context, trainingProteins, validationProteins, testingProteins, allProteins, cma8Target);
    computeDisulfideStatistics(context, allProteins);
    
    return true;
  }

protected:
  friend class ProteinStatisticsWorkUnitClass;

  File inputDirectory;

protected:
  void computeProteinStatistics(ExecutionContext& context, ContainerPtr trainingProteins, ContainerPtr validationProteins, ContainerPtr testingProteins, ContainerPtr allProteins) const
  {
    context.enterScope(T("Computing gobal statistics"), WorkUnitPtr());
    
    HistogramConfigurationPtr res;
    res = computeLengthStatistics(context, trainingProteins, T("Training"));
    context.resultCallback(T("Train"), res);
    
    res = computeLengthStatistics(context, validationProteins, T("Validation"));
    context.resultCallback(T("Validation"), res);
    
    res = computeLengthStatistics(context, testingProteins, T("Testing"));
    context.resultCallback(T("Test"), res);
    
    res = computeLengthStatistics(context, allProteins, T("Global"));
    context.resultCallback(T("All"), res);

    context.leaveScope();
  }
  
  HistogramConfigurationPtr computeLengthStatistics(ExecutionContext& context, const ContainerPtr& proteins, const String& name) const
  {
    const size_t n = proteins->getNumElements();
    HistogramConfigurationPtr config = new HistogramConfiguration(36, 0, 1400, true, name);
    for (size_t i = 0; i < n; i++)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      jassert(protein);
      config->addData(protein->getLength());
    }
    return config;
  }
  
  void computeStatistics(ExecutionContext& context, ContainerPtr trainingProteins, ContainerPtr validationProteins, ContainerPtr testingProteins, ContainerPtr allProteins, ProteinTarget target) const
  {
    context.enterScope(T("Computing statistics of ") + proteinClass->getMemberVariableName(target), WorkUnitPtr());

    const TypePtr targetType = proteinClass->getMemberVariableType(target);
    if (targetType->inheritsFrom(objectVectorClass(doubleVectorClass(enumValueType, probabilityType))))
    {
      PieChartConfigurationPtr res;
      res = computeMultiClassStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);

      res = computeMultiClassStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);

      res = computeMultiClassStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);

      res = computeMultiClassStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (targetType->inheritsFrom(simpleDenseDoubleVectorClass))
    {
      PieChartConfigurationPtr res;
      res = computeBinaryStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeBinaryStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeBinaryStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeBinaryStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (target == cbpTarget)
    {
      PieChartConfigurationPtr res;
      res = computeMultiClassPropertyStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeMultiClassPropertyStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeMultiClassPropertyStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeMultiClassPropertyStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else if (targetType->inheritsFrom(doubleSymmetricMatrixClass(probabilityType)))
    {
      PieChartConfigurationPtr res;
      res = computeSymmetricMatrixStatistics(context, trainingProteins, target, T("Training"));
      context.resultCallback(T("Train"), res);
      
      res = computeSymmetricMatrixStatistics(context, validationProteins, target, T("Validation"));
      context.resultCallback(T("Validation"), res);
      
      res = computeSymmetricMatrixStatistics(context, testingProteins, target, T("Testing"));
      context.resultCallback(T("Test"), res);
      
      res = computeSymmetricMatrixStatistics(context, allProteins, target, T("Global"));
      context.resultCallback(T("All"), res);
    }
    else
      context.errorCallback(T("computeStatistics"), T("The target is not (yet) supported !"));

    context.leaveScope();
  }
  
  void computeDisulfideStatistics(ExecutionContext& context, const ContainerPtr& proteins) const
  {
    const size_t n = proteins->getNumElements();
    context.enterScope(T("Computing disulfide histograms"), WorkUnitPtr());
    
    HistogramConfigurationPtr configNumBridges = new HistogramConfiguration(1, 0, 30, false, T("#Seqs vs #Bridges"));
    HistogramConfigurationPtr configBridgeDensity = new HistogramConfiguration(0.00555555, 0, 0.18, false, T("#Seqs vs #Briges/Length"));
    for (size_t i = 0; i < n; i++)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>(context);
      jassert(protein);
      const DoubleVectorPtr& states = protein->getCysteinBondingStates(context);
      const size_t numStates = states->getNumElements();
      size_t numBondedCysteins = 0;
      for (size_t i = 0; i < numStates; ++i)
        if (states->getElement(i).getDouble() > 0.5)
          ++numBondedCysteins;
      jassert(numBondedCysteins % 2 == 0);
      
      configNumBridges->addData(numBondedCysteins / 2);
      configBridgeDensity->addData((numBondedCysteins / 2) / (double)protein->getLength());
    }
    context.resultCallback(T("#Seqs vs #Bridges"), configNumBridges);
    context.resultCallback(T("#Seqs vs #Briges/Length"), configBridgeDensity);

    context.leaveScope();
  }

  PieChartConfigurationPtr computeMultiClassStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    EnumerationPtr enumeration = proteinClass->getMemberVariableType(target)->getTemplateArgument(0)->getTemplateArgument(0).dynamicCast<Enumeration>();
    jassert(enumeration);
    std::vector<size_t> counts(enumeration->getNumElements() + 1, 0);
    size_t totalCount = 0;
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      ObjectVectorPtr vector = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<ObjectVector>(context);
      if (!vector)
        continue;

      size_t numElements = vector->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = (size_t)-1;
        DoubleVectorPtr values = vector->getElement(j).getObjectAndCast<DoubleVector>();
        if (values)
          index = (size_t)values->getIndexOfMaximumValue();
        counts[index == (size_t)-1 ? counts.size() - 1 : index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      config->appendElement(enumeration->getElementName(i) + T(" (") + String((int)counts[i]) + T(" examples)"), (double)counts[i]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[counts.size() - 1]) + T(" examples)"), (double)counts[counts.size()-1]/(double)totalCount);

    return config;
  }
  
  PieChartConfigurationPtr computeBinaryStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    enum {numBinaryElement = 3};
    std::vector<size_t> counts(numBinaryElement, 0);
    size_t totalCount = 0;

    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      DoubleVectorPtr vector = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<DoubleVector>(context);
      if (!vector)
        continue;

      size_t numElements = vector->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = numBinaryElement - 1; // Missing
        Variable value = vector->getElement(j);
        if (value.exists())
          index = value.getDouble() > 0.5 ? 0 : 1;
        counts[index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
                                                                
    config->appendElement(T("True (") + String((int)counts[0]) + T(" examples)"), (double)counts[0]/(double)totalCount);
    config->appendElement(T("False (") + String((int)counts[1]) + T(" examples)"), (double)counts[1]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[2]) + T(" examples)"), (double)counts[2]/(double)totalCount);

    return config;
  }
  
  PieChartConfigurationPtr computeMultiClassPropertyStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    EnumerationPtr enumeration = proteinClass->getMemberVariableType(target)->getTemplateArgument(0).dynamicCast<Enumeration>();
    jassert(enumeration);
    std::vector<size_t> counts(enumeration->getNumElements() + 1, 0);
    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);

      size_t index = (size_t)-1;
      DoubleVectorPtr values = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<DoubleVector>();
      if (values)
        index = (size_t)values->getIndexOfMaximumValue();
      counts[index == (size_t)-1 ? counts.size() - 1 : index]++;
    }
    
    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)n));
    for (size_t i = 0; i < enumeration->getNumElements(); ++i)
      config->appendElement(enumeration->getElementName(i) + T(" (")+ String((int)counts[i]) + T(" examples)"), (double)counts[i]/(double)n);
    config->appendElement(T("Missing (") + String((int)counts[counts.size() - 1]) + T(" examples)"), (double)counts[counts.size()-1]/(double)n);
    
    return config;
  }
  
  PieChartConfigurationPtr computeSymmetricMatrixStatistics(ExecutionContext& context, const ContainerPtr& proteins, ProteinTarget target, const String& name) const
  {
    enum {numBinaryElement = 3};
    std::vector<size_t> counts(numBinaryElement, 0);
    size_t totalCount = 0;

    const size_t n = proteins->getNumElements();
    for (size_t i = 0; i < n; ++i)
    {
      ProteinPtr protein = proteins->getElement(i).getObjectAndCast<Protein>();
      jassert(protein);
      SymmetricMatrixPtr matrix = protein->getTargetOrComputeIfMissing(context, target).getObjectAndCast<SymmetricMatrix>(context);
      if (!matrix)
        continue;

      size_t numElements = matrix->getNumElements();
      for (size_t j = 0; j < numElements; ++j)
      {
        size_t index = numBinaryElement - 1; // Missing
        Variable value = matrix->getElement(j);
        if (value.exists())
          index = value.getDouble() > 0.5 ? 0 : 1;
        counts[index]++;
      }
      totalCount += numElements;
    }

    PieChartConfigurationPtr config = new PieChartConfiguration(name + T(" ") + proteinClass->getMemberVariableShortName(target),
                                                                T("Number of examples: ") + String((int)totalCount));
    config->appendElement(T("True (") + String((int)counts[0]) + T(" examples)"), (double)counts[0]/(double)totalCount);
    config->appendElement(T("False (") + String((int)counts[1]) + T(" examples)"), (double)counts[1]/(double)totalCount);
    config->appendElement(T("Missing (") + String((int)counts[2]) + T(" examples)"), (double)counts[2]/(double)totalCount);

    return config;
  }
};

};
