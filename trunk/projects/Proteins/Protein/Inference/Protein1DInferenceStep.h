/*-----------------------------------------.---------------------------------.
| Filename: ProteinSequenceLabelingInfe...h| Protein Sequence Labeling       |
| Author  : Francis Maes                   |                                 |
| Started : 22/04/2010 21:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
# define LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_

# include "ProteinInferenceStepHelper.h"

namespace lbcpp
{

// Inference:
//   Input, Supervision: Protein
//   Output: a class inherited from Sequence
//
// SubInference:
//   Input: Features
//   Output, Supervision: Sequence elements
class Protein1DInferenceStep : public SharedParallelInference, public ProteinResidueRelatedInferenceStepHelper
{
public:
  Protein1DInferenceStep(const String& name, InferencePtr subInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : SharedParallelInference(name, subInference), ProteinResidueRelatedInferenceStepHelper(targetName, features, supervisionName) {}
  
  Protein1DInferenceStep() {}
  
  virtual size_t getNumSubInferences(ObjectPtr input) const
    {return getProteinLength(input);}

  virtual ObjectPtr getSubInput(ObjectPtr input, size_t index) const
    {return features->compute(getProtein(input), index);}

  virtual ObjectPtr getSubSupervision(ObjectPtr supervision, size_t index, ObjectPtr predictedObject) const
  {
    if (!supervision)
      return ObjectPtr();
    ObjectContainerPtr objects = getSupervision(supervision).dynamicCast<ObjectContainer>();
    return objects ? objects->get(index) : ObjectPtr();
  }

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
    {return ProteinResidueRelatedInferenceStepHelper::createEmptyOutput(input);}

  virtual void setSubOutput(ObjectPtr output, size_t index, ObjectPtr subOutput) const
    {output.dynamicCast<ObjectContainer>()->set(index, subOutput);}

protected:
  virtual bool load(InputStream& istr)
  {
    return SharedParallelInference::load(istr) &&
      ProteinResidueRelatedInferenceStepHelper::load(istr);
  }

  virtual void save(OutputStream& ostr) const
  {
    SharedParallelInference::save(ostr);
    ProteinResidueRelatedInferenceStepHelper::save(ostr);
  }
};
  
typedef ReferenceCountedObjectPtr<Protein1DInferenceStep> Protein1DInferenceStepPtr;

class ProteinSequenceLabelingInferenceStep : public Protein1DInferenceStep
{
public:
  ProteinSequenceLabelingInferenceStep(const String& name, InferencePtr classificationInference, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : Protein1DInferenceStep(name, classificationInference, features, targetName, supervisionName)
  {
    classificationInference->setName(getName() + T(" Classif"));
  }

  ProteinSequenceLabelingInferenceStep(const String& name, ProteinResidueFeaturesPtr features, const String& targetName, const String& supervisionName = String::empty)
    : Protein1DInferenceStep(name, new ClassificationInferenceStep(name + T(" Classif")), features, targetName, supervisionName) {}
  ProteinSequenceLabelingInferenceStep() {}

  virtual ObjectPtr createEmptyOutput(ObjectPtr input) const
  {
    SequencePtr res = Protein1DInferenceStep::createEmptyOutput(input).dynamicCast<Sequence>();
    ClassificationInferenceStepPtr step = getSharedInferenceStep().dynamicCast<ClassificationInferenceStep>();
    if (step)
    {
      LabelSequencePtr ls = res.dynamicCast<LabelSequence>();
      if (ls)
        step->setLabels(ls->getDictionary());
      else
      {
        ScoreVectorSequencePtr svs = res.dynamicCast<ScoreVectorSequence>();
        if (svs)
          step->setLabels(svs->getDictionary());
        else
        {
          ScalarSequencePtr ss = res.dynamicCast<ScalarSequence>();
          if (ss)
            step->setLabels(BinaryClassificationDictionary::getInstance());
          else
            jassert(false);
        }
      }
    }
    return res;
  }
};

class PSSMRowPredictionInferenceStep : public ParallelSharedMultiRegressionInference
{
public:
  PSSMRowPredictionInferenceStep()
    : ParallelSharedMultiRegressionInference(T("PSSMRow"), AminoAcidDictionary::getInstance()) {}

  virtual size_t getNumSubInferences(ObjectPtr) const
    {return AminoAcidDictionary::numAminoAcids;}
 
  FeatureGeneratorPtr getInputFeatures(ObjectPtr input, size_t scoreIndex) const;
};

class PSSMPredictionInferenceStep : public Protein1DInferenceStep
{
public:
  PSSMPredictionInferenceStep(const String& name, ProteinResidueFeaturesPtr features)
    : Protein1DInferenceStep(name, new PSSMRowPredictionInferenceStep(), features, T("PositionSpecificScoringMatrix")) {}
  PSSMPredictionInferenceStep() {}
};

}; /* namespace lbcpp */

#endif //!LBCPP_PROTEIN_INFERENCE_STEP_SEQUENCE_H_
