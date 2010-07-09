/*-----------------------------------------.---------------------------------.
| Filename: ConvertProteinsToNewFormat.cpp | Protein Converter               |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 16:45               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/lbcpp.h>
#include "Protein/ProteinObject.h" // old
#include "Protein/Data/Protein.h" // new
using namespace lbcpp;

extern void declareProteinClasses();

ObjectContainerPtr loadProteins(const File& directory, size_t maxCount = 0)
{
  ObjectStreamPtr proteinsStream = directoryObjectStream(directory, T("*.protein"));
#ifdef JUCE_DEBUG
  ObjectContainerPtr res = proteinsStream->load(maxCount ? maxCount : 7)->randomize();
#else
  ObjectContainerPtr res = proteinsStream->load(maxCount)->randomize();
#endif
  for (size_t i = 0; i < res->size(); ++i)
  {
    ProteinObjectPtr protein = res->getAndCast<ProteinObject>(i);
    jassert(protein);
    protein->computeMissingFields();
  }
  return res;
}

static VectorPtr convertLabelSequence(LabelSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();
  size_t n = sequence->size();
  VectorPtr res = new Vector(targetType, n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
      res->setVariable(i, Variable((int)sequence->getIndex(i), targetType));
  return res;
}

static VectorPtr convertScoreVectorSequence(ScoreVectorSequencePtr sequence, EnumerationPtr targetType)
{
  if (!sequence)
    return VectorPtr();

  size_t n = sequence->size();
  size_t numScores = sequence->getNumScores();

  VectorPtr res = new Vector(discreteProbabilityDistributionClass(targetType), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
    {
      DiscreteProbabilityDistributionPtr distribution = new DiscreteProbabilityDistribution(targetType);
      for (size_t j = 0; j < numScores; ++j)
        distribution->setVariable(j, sequence->getScore(i, j));
      res->setVariable(i, distribution);
    }
  return res;
}

static VectorPtr convertScalarSequenceToProbabilityVector(ScalarSequencePtr sequence)
{
  if (!sequence)
    return VectorPtr();
  size_t n = sequence->size();

  VectorPtr res = new Vector(probabilityType(), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasValue(i))
    {
      double value = sequence->getValue(i);
      jassert(value >= 0 && value <= 1);
      res->setVariable(i, Variable(value, probabilityType()));
    }
  return res;
}

static VectorPtr convertBinaryLabelSequenceToProbabilityVector(LabelSequencePtr sequence)
{
  if (!sequence)
    return VectorPtr();

  jassert(sequence->getDictionary() == BinaryClassificationDictionary::getInstance());
  size_t n = sequence->size();

  VectorPtr res = new Vector(probabilityType(), n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
    {
      size_t value = sequence->getIndex(i);
      jassert(value == 0 || value == 1);
      res->setVariable(i, Variable(value ? 1.0 : 0.0, probabilityType()));
    }
  return res;
}

static CartesianPositionVectorPtr convertCartesianPositionVector(CartesianCoordinatesSequencePtr sequence)
{
  if (!sequence)
    return CartesianPositionVectorPtr();

  size_t n = sequence->size();
  CartesianPositionVectorPtr res = new CartesianPositionVector(n);
  for (size_t i = 0; i < n; ++i)
    if (sequence->hasObject(i))
      res->setPosition(i, sequence->getPosition(i));
  return res;
}

static AtomPtr convertAtom(ProteinAtomPtr atom)
{
  if (!atom)
    return AtomPtr();

  AtomPtr res = new Atom(atom->getName(), atom->getElementSymbol(), new Vector3(atom->getPosition()));
  res->setOccupancy(res->getOccupancy());
  res->setTemperatureFactor(res->getTemperatureFactor());
  return res;
}

static ResiduePtr convertResidue(ProteinResidueAtomsPtr residue)
{
  if (!residue)
    return ResiduePtr();

  size_t n = residue->getNumAtoms();
  ResiduePtr res = new Residue((AminoAcidType)residue->getAminoAcid());
  for (size_t i = 0; i < n; ++i)
    res->addAtom(convertAtom(residue->getAtom(i)));
  return res;
}

static TertiaryStructurePtr convertTertiaryStructure(ProteinTertiaryStructurePtr tertiaryStructure)
{
  if (!tertiaryStructure)
    return TertiaryStructurePtr();

  size_t n = tertiaryStructure->size();
  TertiaryStructurePtr res = new TertiaryStructure(n);
  for (size_t i = 0; i < n; ++i)
    res->setResidue(i, convertResidue(tertiaryStructure->getResidue(i)));
  return res;
}

static ProteinPtr convertProtein(ProteinObjectPtr protein)
{
  ProteinPtr res = new Protein(protein->getName());
  res->setPrimaryStructure(convertLabelSequence(protein->getAminoAcidSequence(), aminoAcidTypeEnumeration()));
  res->setPositionSpecificScoringMatrix(convertScoreVectorSequence(protein->getPositionSpecificScoringMatrix(), aminoAcidTypeEnumeration()));

  res->setSecondaryStructure(convertLabelSequence(protein->getSecondaryStructureSequence(), secondaryStructureElementEnumeration()));
  res->setDSSPSecondaryStructure(convertLabelSequence(protein->getDSSPSecondaryStructureSequence(), dsspSecondaryStructureElementEnumeration()));
  res->setStructuralAlphabetSequence(convertLabelSequence(protein->getStructuralAlphabetSequence(), structuralAlphaElementEnumeration()));

  res->setSolventAccessibility(convertScalarSequenceToProbabilityVector(protein->getNormalizedSolventAccessibilitySequence()));
  res->setSolventAccessibilityAt20p(convertBinaryLabelSequenceToProbabilityVector(protein->getSolventAccessibilityThreshold20()));

  if (protein->getDisorderProbabilitySequence())
    res->setDisorderRegions(convertScalarSequenceToProbabilityVector(protein->getDisorderProbabilitySequence()));
  else
    res->setDisorderRegions(convertBinaryLabelSequenceToProbabilityVector(protein->getDisorderSequence()));

  res->setCAlphaTrace(convertCartesianPositionVector(protein->getCAlphaTrace()));
  res->setTertiaryStructure(convertTertiaryStructure(protein->getTertiaryStructure()));

  //res->computeMissingVariables();
  return res;
}

static VectorPtr convertProteins(ObjectContainerPtr oldStyleProteins)
{
  VectorPtr proteins = new Vector(proteinClass(), oldStyleProteins->size());
  for (size_t i = 0; i < proteins->size(); ++i)
  {
    ProteinObjectPtr oldStyleProtein = oldStyleProteins->getAndCast<ProteinObject>(i);
    jassert(oldStyleProtein);
    ProteinPtr protein = convertProtein(oldStyleProtein);
    jassert(protein);
    proteins->setVariable(i, protein);
  }
  return proteins;
}


int main(int argc, char** argv)
{
  lbcpp::initialize();
  declareProteinClasses();

  File workingDirectory(T("C:\\Projets\\LBC++\\projects\\temp"));
  //File workingDirectory(T("/Users/francis/tmp"));
  ObjectContainerPtr oldStyleProteins = loadProteins(workingDirectory.getChildFile(T("SmallPDB\\protein")));
  
  // convert proteins
  VectorPtr proteins = convertProteins(oldStyleProteins);
  oldStyleProteins = ObjectContainerPtr();
  std::cout << proteins->size() << " proteins" << std::endl;
  proteins->getVariable(0).saveToFile(workingDirectory.getChildFile(T("NewProt.xml")));
  
  Variable loadedProtein = Variable::createFromFile(workingDirectory.getChildFile(T("NewProt.xml")));
  std::cout << "Loaded protein: " << loadedProtein << std::endl;
  loadedProtein.saveToFile(workingDirectory.getChildFile(T("NewProt2.xml")));
  
  return 0;
}
