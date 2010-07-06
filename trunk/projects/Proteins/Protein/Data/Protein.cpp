/*-----------------------------------------.---------------------------------.
| Filename: Protein.cpp                    | Protein                         |
| Author  : Francis Maes                   |                                 |
| Started : 26/06/2010 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

class ProteinClass : public Class
{
public:
  ProteinClass() : Class(T("Protein"), objectClass())
  {
    addVariable(vectorClass(aminoAcidTypeEnumeration()), T("primaryStructure"));
    addVariable(vectorClass(secondaryStructureElementEnumeration()), T("secondaryStructure"));
    addVariable(vectorClass(dsspSecondaryStructureElementEnumeration()), T("dsspSecondaryStructure"));

    addVariable(vectorClass(probabilityType()), T("solventAccesibility"));
    addVariable(vectorClass(probabilityType()), T("solventAccesibilityAt20p"));
    addVariable(vectorClass(booleanType()), T("disorderRegions"));
    addVariable(vectorClass(probabilityType()), T("disorderRegionProbabilities"));

    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Ca"));
    addVariable(symmetricMatrixClass(probabilityType()), T("contactMap8Cb"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCa"));
    addVariable(symmetricMatrixClass(angstromDistanceType()), T("distanceMapCb"));

    addVariable(vectorClass(residueClass()), T("residues"));
  }
};

void declareProteinClass()
{
  Class::declare(new ProteinClass());
}

extern ClassPtr lbcpp::proteinClass()
  {static ClassPtr res = Class::get(T("Protein")); return res;}

///////////////////

Variable Protein::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return primaryStructure;
  case 1: return secondaryStructure;
  case 2: return dsspSecondaryStructure;
  case 3: return solventAccessibility;
  case 4: return solventAccessibilityAt20p;
  case 5: return disorderRegions;
  case 6: return disorderRegionProbabilities;
  case 7: return contactMap8Ca;
  case 8: return contactMap8Cb;
  case 9: return distanceMapCa;
  case 10: return distanceMapCb;
  case 11: return residues;
  };
  jassert(false);
  return Variable();
}

void Protein::computeMissingVariables()
{
  if (primaryStructure)
  {
    if (!residues)
      createResidues();
  }
}

void Protein::createResidues()
{
  jassert(primaryStructure);
  size_t n = getLength();
  residues = new Vector(residueClass(), n);
  ProteinPtr pthis(const_cast<Protein* >(this));
  for (size_t i = 0; i < n; ++i)
    residues->setVariable(i, new Residue(pthis, i));
}

void Protein::clearResidues()
{
  if (residues)
  {
    size_t n = residues->size();
    for (size_t i = 0; i < n; ++i)
      residues->getVariable(i).getObjectAndCast<Residue>()->clear();
  }
}
