/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

class ProteinClass : public DynamicClass
{
public:
  ProteinClass() : DynamicClass(T("Protein"), nameableObjectClass())
  {
    addVariable(vectorClass(aminoAcidTypeEnumeration()), T("primaryStructure"));
    addVariable(vectorClass(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration())), T("positionSpecificScoringMatrix"));

    addVariable(vectorClass(secondaryStructureElementEnumeration()), T("secondaryStructure"));
    addVariable(vectorClass(dsspSecondaryStructureElementEnumeration()), T("dsspSecondaryStructure"));
    addVariable(vectorClass(structuralAlphaElementEnumeration()), T("structuralAlphabetSequence"));

    addVariable(vectorClass(probabilityType()), T("solventAccesibility"));
    addVariable(vectorClass(probabilityType()), T("solventAccesibilityAt20p"));
    addVariable(vectorClass(probabilityType()), T("disorderRegions"));

    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Ca"));
    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Cb"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCa"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCb"));

    addVariable(cartesianPositionVectorClass(), T("calphaTrace"));
    addVariable(tertiaryStructureClass(), T("tertiaryStructure"));
  }

  virtual VariableValue create() const
    {return new Protein();}
};

void declareProteinClass()
{
  Class::declare(new ProteinClass());
}

extern ClassPtr lbcpp::proteinClass()
  {static ClassPtr res = Class::get(T("Protein")); return res;}

///////////////////

void Protein::setPrimaryStructure(const String& primaryStructure)
{
  jassert(!this->primaryStructure);
  size_t n = primaryStructure.length();

  this->primaryStructure = new Vector(aminoAcidTypeEnumeration(), n);
  for (size_t i = 0; i < n; ++i)
    this->primaryStructure->setVariable(i, AminoAcid::fromOneLetterCode(primaryStructure[i]));
}

SymmetricMatrixPtr Protein::getContactMap(double threshold, bool betweenCBetaAtoms) const
{
  jassert(threshold == 8.0);
  return betweenCBetaAtoms ? contactMap8Cb : contactMap8Ca;
}

void Protein::setContactMap(SymmetricMatrixPtr contactMap, double threshold, bool betweenCBetaAtoms)
{
  jassert(threshold == 8.0);
  if (betweenCBetaAtoms)
    contactMap8Cb = contactMap;
  else
    contactMap8Ca = contactMap;
}

SymmetricMatrixPtr Protein::getDistanceMap(bool betweenCBetaAtoms) const
{
  return betweenCBetaAtoms ? distanceMapCb : distanceMapCa;
}

void Protein::setDistanceMap(SymmetricMatrixPtr distanceMap, bool betweenCBetaAtoms)
{
  if (betweenCBetaAtoms)
    distanceMapCb = distanceMap;
  else
    distanceMapCa = distanceMap;
}

VariableReference Protein::getVariableReference(size_t index)
{
  size_t baseClassVariables = nameableObjectClass()->getNumStaticVariables();
  if (index < baseClassVariables)
    return NameableObject::getVariableReference(index);
  index -= baseClassVariables;
  switch (index)
  {
  case 0: return primaryStructure;
  case 1: return positionSpecificScoringMatrix;
  case 2: return secondaryStructure;
  case 3: return dsspSecondaryStructure;
  case 4: return structuralAlphabetSequence;
  case 5: return solventAccessibility;
  case 6: return solventAccessibilityAt20p;
  case 7: return disorderRegions;
  case 8: return contactMap8Ca;
  case 9: return contactMap8Cb;
  case 10: return distanceMapCa;
  case 11: return distanceMapCb;
  case 12: return calphaTrace;
  case 13: return tertiaryStructure;
  };
  jassert(false);
  return VariableReference();
}

void Protein::computeMissingVariables()
{
  if (primaryStructure)
  {
  }
}

VectorPtr Protein::createEmptyPositionSpecificScoringMatrix() const
{
  size_t n = getLength();
  VectorPtr res = new Vector(discreteProbabilityDistributionClass(aminoAcidTypeEnumeration()), n);
  for (size_t i = 0; i < n; ++i)
    res->setVariable(i, new DiscreteProbabilityDistribution(aminoAcidTypeEnumeration()));
  return res;
}

VectorPtr Protein::createEmptySecondaryStructure() const
  {return new Vector(secondaryStructureElementEnumeration(), getLength());}

VectorPtr Protein::createEmptyDSSPSecondaryStructure() const
  {return new Vector(dsspSecondaryStructureElementEnumeration(), getLength());}

VectorPtr Protein::createEmptySolventAccesibility() const
  {return new Vector(probabilityType(), getLength());}

VectorPtr Protein::createEmptyDisorderRegions() const
  {return new Vector(booleanType(), getLength());}

Variable Protein::createEmptyTarget(size_t index) const
{
  size_t n = getLength();

  switch (index)
  {
  case 1: return createEmptyPositionSpecificScoringMatrix();
  case 2: return createEmptySecondaryStructure();
  case 3: return createEmptyDSSPSecondaryStructure();
  case 4: return new Vector(structuralAlphaElementEnumeration(), n);
  case 5: return createEmptySolventAccesibility();

  default:
    jassert(false); return Variable();
  }
}
