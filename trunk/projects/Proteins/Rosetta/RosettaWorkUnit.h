/*-----------------------------------------.---------------------------------.
| Filename: RosettaWorkUnit.h              | Rosetta WorkUnits               |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : 22/04/2011 15:17               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_

# include "precompiled.h"
# include "../Data/Protein.h"
# include "../Data/Formats/PDBFileGenerator.h"
# include "RosettaUtils.h"
# include "ProteinOptimizer.h"
# include "ProteinMover.h"
# include "ProteinOptimizer/SimulatedAnnealingOptimizer.h"
# include "Sampler/ProteinMoverSampler.h"

namespace lbcpp
{

class ProteinOptimizerWorkUnit;
typedef ReferenceCountedObjectPtr<ProteinOptimizerWorkUnit> ProteinOptimizerWorkUnitPtr;

class ProteinOptimizerWorkUnit: public CompositeWorkUnit
{
public:
  // returnPose must be already instantiated
  ProteinOptimizerWorkUnit()
  {
  }

  ProteinOptimizerWorkUnit(const String& proteinName, core::pose::PoseOP& pose,
      ProteinOptimizerPtr& optimizer, ProteinMoverSamplerPtr& sampler, RandomGeneratorPtr& random) :
    CompositeWorkUnit(T("ProteinOptimizerWorkUnit"))
  {
    this->proteinName = proteinName;
    this->pose = pose;
    this->optimizer = optimizer;
    this->sampler = sampler;
    this->random = random;
  }

  virtual Variable run(ExecutionContext& context)
  {
    context.informationCallback(T("Protein : ") + proteinName + T(" loaded succesfully."));
    context.resultCallback(T("Initial energy"), Variable(getConformationScore(pose)));
    core::pose::PoseOP returnPose = new core::pose::Pose(*pose);
    *returnPose = (*(optimizer->apply(pose, sampler, context, random)));
    context.informationCallback(T("Optimization done."));
    double score = getConformationScore(returnPose);
    context.resultCallback(T("Final energy"), Variable(score));

    return Variable(proteinName);
  }

protected:
  friend class ProteinOptimizerWorkUnitClass;
  String proteinName;
  core::pose::PoseOP pose;
  ProteinOptimizerPtr optimizer;
  ProteinMoverSamplerPtr sampler;
  RandomGeneratorPtr random;
};

class ProteinFeaturesGeneratorWorkUnit: public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);

    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
      context.errorCallback(T("Proteins' directory not found."));
    ContainerPtr proteins = Protein::loadProteinsFromDirectory(context, directory);
    int numProteins = proteins->getNumElements();

    double frequenceVerbosity = 0.0001;

    // Creating parallel workunits
    CompositeWorkUnitPtr proteinsOptimizer = new CompositeWorkUnit(T("ProteinsOptimizer"),
        numProteins);
    for (int i = 0; i < proteinsOptimizer->getNumWorkUnits(); i++)
    {
      ProteinPtr currentProtein = proteins->getElement(i).getObjectAndCast<Protein> ();
      core::pose::PoseOP currentPose = convertProteinToPose(context, currentProtein);
      core::pose::PoseOP initialPose = initializeProteinStructure(currentPose);
      core::pose::PoseOP returnPose = new core::pose::Pose();
      String currentName = currentProtein->getName();

      // Output arguments
      juce::File outputDirectory;
      if (timesFeatureGeneration > 0)
      {
        outputDirectory = context.getFile(resultsDirectory);
        if (!outputDirectory.exists())
          outputDirectory.createDirectory();
      }

      // Optimizer
      PhiPsiMoverSamplerPtr samp0 = new PhiPsiMoverSampler(initialPose->n_residue(), 0, 25, 0, 25);
      ShearMoverSamplerPtr samp1 = new ShearMoverSampler(initialPose->n_residue(), 0, 25, 0, 25);
      RigidBodyTransMoverSamplerPtr samp2 = new RigidBodyTransMoverSampler(
          initialPose->n_residue(), 0.5, 0.25);
      RigidBodySpinMoverSamplerPtr samp3 = new RigidBodySpinMoverSampler(initialPose->n_residue(),
          0, 20);
      RigidBodyGeneralMoverSamplerPtr samp4 = new RigidBodyGeneralMoverSampler(
          initialPose->n_residue(), 0.5, 0.25, 0, 20);

      std::vector<Variable> samplers;
      samplers.push_back(Variable(samp0));
      samplers.push_back(Variable(samp1));
      samplers.push_back(Variable(samp2));
      samplers.push_back(Variable(samp3));
      samplers.push_back(Variable(samp4));
      ProteinMoverSamplerPtr samp(new ProteinMoverSampler(5, samplers));
      ProteinOptimizerPtr o = new ProteinSimulatedAnnealingOptimizer(4.0, 0.01, 50,
          maxNumberIterations, 5, currentProtein->getName(), 0.01, timesFeatureGeneration,
          outputDirectory);
      RandomGeneratorPtr random = new RandomGenerator(0);

      WorkUnitPtr childWorkUnit = new ProteinOptimizerWorkUnit(currentName, initialPose, o, samp,
          random);
      proteinsOptimizer->setWorkUnit(i, childWorkUnit);
    }
    proteinsOptimizer->setPushChildrenIntoStackFlag(true);
    context.informationCallback(T("Computing..."));
    context.run(proteinsOptimizer);

    context.informationCallback(T("ProteinFeaturesGeneratorWorkUnit done."));
    return Variable();
  }

protected:
  friend class ProteinFeaturesGeneratorWorkUnitClass;
  String proteinsDirectory;
  String resultsDirectory;

  int timesFeatureGeneration;
  int maxNumberIterations;
};

class XmlToPDBConverterWorkUnit: public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
      context.errorCallback(T("Proteins' directory not found."));

    juce::OwnedArray<File> results;
    directory.findChildFiles(results, File::findFiles, false, T("*.xml"));

    // Other arguments
    juce::File outputDirectory = context.getFile(resultsDirectory);
    if (!outputDirectory.exists())
      outputDirectory.createDirectory();

    context.informationCallback(T("Performing conversion..."));

    for (int i = 0; i < results.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));

      String nameXmlFile = results[i]->getFileNameWithoutExtension();

      File outFile(outputDirectory.getFullPathName() + T("/") + nameXmlFile + T(".pdb"));
      currentProtein->saveToPDBFile(context, outFile);
    }

    context.informationCallback(T("Conversion in PDB files done."));
    return Variable();
  }

protected:
  friend class XmlToPDBConverterWorkUnitClass;
  String proteinsDirectory;
  String resultsDirectory;
};

class EnergyEvaluationWorkUnit: public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    rosettaInitialization(context, false);

    // Load all xml files in proteinsDir
    File directory = context.getFile(proteinsDirectory);
    if (!directory.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return Variable();
    }

    juce::OwnedArray<File> results;
    directory.findChildFiles(results, File::findFiles, false, T("*.xml"));

    // Other arguments
    juce::File output(directory.getFullPathName() + T("/") + T("energies_")
        + directory.getFileName() + T(".data"));
    juce::OutputStream *fos = output.createOutputStream();

    context.informationCallback(T("Performing calculation..."));

    for (int i = 0; i < results.size(); i++)
    {
      ProteinPtr currentProtein = Protein::createFromXml(context, (*results[i]));
      core::pose::PoseOP pose = convertProteinToPose(context, currentProtein);
      context.informationCallback((*results[i]).getFileNameWithoutExtension());
      context.informationCallback(currentProtein->getName());
      context.informationCallback(Variable(getTotalEnergy(pose)).toString());

      *fos << getTotalEnergy(pose) << "\r\n";
    }

    fos->flush();
    delete fos;

    context.informationCallback(T("Energies evaluated and stored in energies.data"));
    return Variable();
  }

protected:
  friend class EnergyEvaluationWorkUnitClass;
  String proteinsDirectory;
};

class FilesHarmonizationWorkUnit: public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    File homeDirectory = context.getFile(proteinsDirectory);
    if (!homeDirectory.exists())
    {
      context.errorCallback(T("Proteins' directory not found."));
      return Variable();
    }

    juce::OwnedArray<File> directories;
    homeDirectory.findChildFiles(directories, File::findDirectories, false);

    for (int j = 0; j < directories.size(); j++)
    {
      juce::OwnedArray<File> results;
      (*directories[j]).findChildFiles(results, File::findFiles, false, T("*.xml"));

      for (int i = 0; i < results.size(); i++)
      {
        String nameToModify = (*results[i]).getFileNameWithoutExtension();
        String goodName;
        if (nameToModify.contains(T("_")))
          goodName = nameToModify.upToFirstOccurrenceOf(T("_"), false, true);
        else
          goodName = nameToModify;

        bool exist = existsForAllDirectories(directories, goodName);
        if (!exist)
          deleteForAllDirectories(directories, goodName);

      }
    }
    context.informationCallback(T("All files harmonized."));
    return Variable();
  }

  bool existsForAllDirectories(juce::OwnedArray<File>& directories, String name)
  {
    bool exists = true;
    for (int k = 0; k < directories.size(); k++)
    {
      juce::OwnedArray<File> results;
      (*directories[k]).findChildFiles(results, File::findFiles, false, name + T("*.xml"));
      if (results.size() == 0)
      {
        exists = false;
        break;
      }
    }
    return exists;
  }

  void deleteForAllDirectories(juce::OwnedArray<File>& directories, String name)
  {
    for (int k = 0; k < directories.size(); k++)
    {
      juce::OwnedArray<File> results;
      (*directories[k]).findChildFiles(results, File::findFiles, false, name + T("*.xml"));
      for (int i = 0; i < results.size(); i++)
      {
        (*results[i]).deleteFile();
      }
    }
  }

protected:
  friend class FilesHarmonizationWorkUnitClass;
  String proteinsDirectory;
};

}; /* namespace lbcpp */

#endif // ! LBCPP_PROTEINS_ROSETTA_WORKUNIT_H_