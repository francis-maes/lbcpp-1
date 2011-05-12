/*-----------------------------------------.---------------------------------.
| Filename: ProteinContactMapInference.h   | Protein Contact Map Inference   |
| Author  : Francis Maes                   |                                 |
| Started : 28/04/2010 11:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_
# define LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_

# include "Protein2DTargetInference.h"

namespace lbcpp
{

class ContactMapScoresInference : public Protein2DTargetInference
{
public:
  ContactMapScoresInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr features, const String& targetName);
  ContactMapScoresInference() {}

  virtual void computeSubStepIndices(ProteinPtr protein, std::vector< std::pair<size_t, size_t> >& res) const;
  virtual ObjectPtr getSubSupervision(ObjectPtr supervisionObject, size_t firstPosition, size_t secondPosition) const;
  virtual void setSubOutput(ObjectPtr output, size_t firstPosition, size_t secondPosition, ObjectPtr subOutput) const;
};

class ContactMapScoresToProbabilitiesInference : public Inference, public ProteinTargetInferenceHelper
{
public:
  ContactMapScoresToProbabilitiesInference(const String& name, const String& targetName);
  ContactMapScoresToProbabilitiesInference() {}

  virtual ObjectPtr run(InferenceContextPtr context, ObjectPtr input, ObjectPtr supervision, ReturnCode& returnCode);
  
  void setThreshold(double threshold)
    {this->threshold = threshold;}

protected:
  virtual bool load(InputStream& istr)
    {return Inference::load(istr) && ProteinTargetInferenceHelper::load(istr) && lbcpp::read(istr, threshold);}

  virtual void save(OutputStream& ostr) const
    {Inference::save(ostr); ProteinTargetInferenceHelper::save(ostr); lbcpp::write(ostr, threshold);}

private:
  double threshold;
};

typedef ReferenceCountedObjectPtr<ContactMapScoresToProbabilitiesInference> ContactMapScoresToProbabilitiesInferencePtr;

class ProteinContactMapInference : public VectorSequentialInference, public ProteinTargetInferenceHelper
{
public:
  ProteinContactMapInference(const String& name, InferencePtr scoreInference, ProteinResiduePairFeaturesPtr scoreFeatures, const String& targetName);
  ProteinContactMapInference() {}

  virtual std::pair<ObjectPtr, ObjectPtr> prepareSubInference(SequentialInferenceStatePtr state, ReturnCode& returnCode) const;
};

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_CONTACT_MAP_STEP_H_