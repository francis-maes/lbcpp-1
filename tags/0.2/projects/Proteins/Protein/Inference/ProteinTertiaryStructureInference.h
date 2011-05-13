/*-----------------------------------------.---------------------------------.
| Filename: ProteinTertiaryStructureInfe..h| Tertiary Structure Inference    |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 19:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_

# include "Protein1DTargetInference.h"

namespace lbcpp
{

class ProteinResidueRefinementInferenceStep : public VectorStaticParallelInference
{
public:
  ProteinResidueRefinementInferenceStep(const String& name)
    : VectorStaticParallelInference(name)
  {
    for (size_t i = 0; i < 3; ++i)
    {
      // FIXME:
      /*
      String prefix = name + T(" ") + getBackboneAtomName(i) + T(".");
      subInferences.append(new RegressionInferenceStep(prefix + T("x")));
      subInferences.append(new RegressionInferenceStep(prefix + T("y")));
      subInferences.append(new RegressionInferenceStep(prefix + T("z")));
      */
    }
  }
  ProteinResidueRefinementInferenceStep() {}

  static String getBackboneAtomName(size_t index)
  {
    jassert(index < 3);
    static const juce::tchar* backboneAtomNames[] = {T("N"), T("CA"), T("C")};
    return backboneAtomNames[index];
  }

  virtual ParallelInferenceStatePtr prepareInference(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode)
  {
    ProteinResiduePtr residue = supervision.dynamicCast<ProteinResidue>();
    jassert(residue || !supervision);

    ParallelInferenceStatePtr res = new ParallelInferenceState(input, supervision);
    for (size_t i = 0; i < 3; ++i)
    {
      ProteinAtomPtr atom = residue ? residue->findAtomByName(getBackboneAtomName(i)) : ProteinAtomPtr();
      if (atom && !atom->getPosition().exists())
        atom = ProteinAtomPtr();
      for (size_t j = 0; j < 3; ++j)
        if (atom)
        {
          Vector3 position = atom->getPosition();
          double target = (j == 0 ? position.getX() : (j == 1 ? position.getY() : position.getZ()));
          res->addSubInference(subInferences[i], input, new Scalar(target));
        }
        else
          res->addSubInference(subInferences[i], input, ObjectPtr());
    }
    return res;
  }

  virtual ObjectPtr finalizeInference(InferenceContextPtr context, ParallelInferenceStatePtr state, ReturnCode& returnCode)
  {
    ProteinResiduePtr res = new ProteinResidue();
    res->addAtom(new ProteinAtom(T("N"), T("N")));
    res->addAtom(new ProteinAtom(T("CA"), T("C")));
    res->addAtom(new ProteinAtom(T("C"), T("C")));
    jassert(false); // FIXME
    return res;
  }/*

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    return res;
  }

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
  {
    ProteinResiduePtr residue = output.dynamicCast<ProteinResidue>();
    ScalarPtr prediction = subOutput.dynamicCast<Scalar>();
    jassert(residue && prediction);
    ProteinAtomPtr atom = residue->findAtomByName(getBackboneAtomName(index / 3));
    jassert(atom);
    index %= 3;
    if (index == 0)
      atom->getPosition().setX(prediction->getValue());
    else if (index == 1)
      atom->getPosition().setY(prediction->getValue());
    else if (index == 2)
      atom->getPosition().setZ(prediction->getValue());
  }*/
};

class ProteinTertiaryStructureRefinementInferenceStep : public Protein1DTargetInference
{
public:
  ProteinTertiaryStructureRefinementInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DTargetInference(name, new ProteinResidueRefinementInferenceStep(name + T(" Residue")), features, T("TertiaryStructure")) {}
  
  ProteinTertiaryStructureRefinementInferenceStep() {}
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_TERTIARY_STRUCTURE_STEP_H_