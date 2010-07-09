/*-----------------------------------------.---------------------------------.
| Filename: Residue.h                      | Residue Atoms                   |
| Author  : Francis Maes                   |                                 |
| Started : 09/07/2010 15:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_RESIDUE_H_
# define LBCPP_PROTEIN_RESIDUE_H_

# include "AminoAcid.h"
# include "Atom.h"
# include "../../Geometry/Matrix4.h"

namespace lbcpp
{

class Residue;
typedef ReferenceCountedObjectPtr<Residue> ResiduePtr;

class Residue : public Object
{
public:
  Residue(AminoAcidType aminoAcidType)
    : aminoAcidType(aminoAcidType), atoms(new Vector(atomClass())) {}
  Residue() {}

  virtual String getName() const
    {return Variable(aminoAcidType, aminoAcidTypeEnumeration()).toString();}

  AminoAcidType getAminoAcidType() const
    {return aminoAcidType;}

  void setAminoAcidType(AminoAcidType aminoAcidType)
    {this->aminoAcidType = aminoAcidType;}

  size_t getNumAtoms() const
    {return atoms->size();}

  AtomPtr getAtom(size_t index) const
    {jassert(index < atoms->size()); return atoms->getObjectAndCast<Atom>(index);}

  void addAtom(AtomPtr atom)
    {atoms->append(atom);}

  // central atom (back bone, first atom of the side chain)
  AtomPtr getCAlphaAtom() const
    {return findAtomByName(T("CA"));}

  // second carbon of the side chain
  AtomPtr getCBetaAtom() const
    {return findAtomByName(T("CB"));}

  // back bone
  AtomPtr getNitrogenAtom() const
    {return findAtomByName(T("N"));}

  // back bone
  AtomPtr getCarbonAtom() const
    {return findAtomByName(T("C"));}

  AtomPtr findAtomByName(const String& name) const;
  
  impl::Vector3 getAtomPosition(const String& name) const
    {AtomPtr atom = findAtomByName(name); return atom ? atom->getPosition() : impl::Vector3();}

  Variable getDistanceBetweenAtoms(const String& name1, const String& name2) const;
  Variable getDistanceBetweenAtoms(const String& name1, ResiduePtr residue2, const String& name2) const;

  bool hasCAlphaAtom() const
    {return getCAlphaAtom();}

  bool hasOnlyCAlphaAtom() const
    {return atoms->size() && hasCAlphaAtom();}

  bool hasCompleteBackbone() const
    {return getCAlphaAtom() && getNitrogenAtom() && getCarbonAtom();}

  bool isCBetaAtomMissing() const
    {return aminoAcidType != glycine && !getCBetaAtom();}

  AtomPtr checkAndGetCBetaOrCAlphaAtom() const;

  void applyAffineTransform(const impl::Matrix4& affineTransform) const;

  // Object
  virtual String toString() const;

  virtual Variable getVariable(size_t index) const
  {
    if (index == 0) return Variable(aminoAcidType, aminoAcidTypeEnumeration());
    if (index == 1) return atoms;
    jassert(false); return Variable();
  }

  virtual void setVariable(size_t index, const Variable& var)
  {
    if (index == 0) aminoAcidType = (AminoAcidType)var.getInteger();
    else if (index == 1) atoms = var.getObjectAndCast<Vector>();
    else jassert(false);
  }

protected:
  AminoAcidType aminoAcidType;
  VectorPtr atoms;
};

extern ClassPtr residueClass();

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_RESIDUE_H_
