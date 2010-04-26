/*-----------------------------------------.---------------------------------.
| Filename: DSSPFileParser.cpp             | DSSP Parser                     |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 19:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
# define LBCPP_PROTEIN_DSSP_FILE_PARSER_H_

# include "../Protein.h"
# include "../SecondaryStructureDictionary.h"

namespace lbcpp
{

class DSSPFileParser : public TextObjectParser
{
public:
  DSSPFileParser(const File& file, ProteinPtr protein)
    : TextObjectParser(file), protein(protein), aminoAcidSequence(protein->getAminoAcidSequence())
  {
    jassert(aminoAcidSequence);
    std::cout << "AA: " << protein->getAminoAcidSequence()->toString() << std::endl;
  }

  virtual void parseBegin()
    {serialNumber = 0;}

  virtual bool parseLine(const String& line)
  {
    if (line.isEmpty())
      return true; // skip empty lines

    size_t n = protein->getLength();

    if (serialNumber == 0)
    {
      if (line.startsWith(T("  #  RESIDUE AA STRUCTURE BP1 BP2  ACC")))
      {
        dsspSecondaryStructureSequence = protein->createEmptyObject(T("DSSPSecondaryStructureSequence"));
        solventAccesibilitySequence = protein->createEmptyObject(T("NormalizedSolventAccessibilitySequence"));
        ++serialNumber;
      }
      return true;
    }

    if (line.length() < 100)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Line is not long enough"));
      return false;
    }
    int newSerialNumber = line.substring(0, 5).trim().getIntValue();
    if (newSerialNumber != (int)serialNumber)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid serial number: ") + lbcpp::toString(newSerialNumber));
      return false;
    }
    ++serialNumber;
    
    String residueNumberString = line.substring(5, 10).trim();
    if (residueNumberString.isEmpty())
      return true; // skip

    int residueNumber = residueNumberString.getIntValue() - 1;
    if (residueNumber < 0 || residueNumber >= (int)n)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid residue number: ") + lbcpp::toString(residueNumber));
      return false;
    }

    /*
    ** Amino Acid
    */
    String aminoAcidCode = line.substring(10, 14).trim();
    if (aminoAcidCode != aminoAcidSequence->getString(residueNumber))
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Amino acid does not match: ") + aminoAcidCode);
      return false;
    }
    
    /*
    ** 8-state Secondary Structure
    */
    String secondaryStructureCode = line.substring(16, 17);
    if (secondaryStructureCode == T(" "))
      secondaryStructureCode = T("_");
    int secondaryStructureIndex = DSSPSecondaryStructureDictionary::getInstance()->getFeatures()->getIndex(secondaryStructureCode);
    if (secondaryStructureIndex < 0)
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Unrecognized secondary structure code: ") + secondaryStructureCode);
      return false;
    }
    dsspSecondaryStructureSequence->setIndex((size_t)residueNumber, (size_t)secondaryStructureIndex);

    /*
    ** Solvent accesibility
    */
    String solventAccesibilityString = line.substring(34, 38).trim();
    if (!solventAccesibilityString.containsOnly(T("0123456789")))
    {
      Object::error(T("DSSPFileParser::parseLine"), T("Invalid solvent accesibility: ") + solventAccesibilityString);
      return false;
    }
    int absoluteSolventAccesiblity = solventAccesibilityString.getIntValue();

    static const double maximumSolventAccissibilityValue[] = {118.1, 256.0, 165.5, 158.7, 146.1, 186.2, 193.2, 88.1, 202.5, 181.0, 193.1, 225.8, 203.4, 222.8, 146.8, 129.8, 152.5, 266.3, 236.8, 164.5, 162.1, 189.7, 187.05, 256.0};
    double normalizedSolventAccessibility = (double)absoluteSolventAccesiblity / maximumSolventAccissibilityValue[secondaryStructureIndex];
    jassert(normalizedSolventAccessibility <= 1.);
    if (normalizedSolventAccessibility > 1.)
      normalizedSolventAccessibility = 1.;
    solventAccesibilitySequence->setValue((size_t)residueNumber, normalizedSolventAccessibility);
    return true;
  }

  virtual bool parseEnd()
  {
    if (!serialNumber)
    {
      Object::error(T("DSSPFileParser::parseEnd"), T("No residues in dssp file"));
      return false;
    }

    setResult(dsspSecondaryStructureSequence);
    protein->setObject(dsspSecondaryStructureSequence);
    // todo: convert to three states
    return true;
  }
  
protected:
  ProteinPtr protein;
  LabelSequencePtr aminoAcidSequence;
  LabelSequencePtr dsspSecondaryStructureSequence;
  ScalarSequencePtr solventAccesibilitySequence;
  int serialNumber;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_DSSP_FILE_PARSER_H_
