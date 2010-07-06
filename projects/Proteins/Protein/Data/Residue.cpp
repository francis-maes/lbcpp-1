/*-----------------------------------------.---------------------------------.
| Filename: Residue.cpp                    | Protein Residue                 |
| Author  : Francis Maes                   |                                 |
| Started : 28/06/2010 18:02               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "Protein.h"
using namespace lbcpp;

Variable Residue::getVariable(size_t index) const
{
  switch (index)
  {
  case 0: return aminoAcid;
  case 1: return previous;
  case 2: return next;
  };
  return Variable();
}

class ResidueClass : public Class
{
public:
  ResidueClass() : Class(T("Residue"), objectClass())
  {
    addVariable(AminoAcid::getCollection(), T("aminoAcid"));
    addVariable(TypePtr(this), T("previous"));
    addVariable(TypePtr(this), T("next"));
  }
};

void declareResidueClasses()
{
  Class::declare(new ResidueClass());
}
