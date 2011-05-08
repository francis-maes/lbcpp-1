/*-----------------------------------------.---------------------------------.
| Filename: ProteinResiduePairFeatures.h   | A feature function phi :        |
| Author  : Francis Maes                   | protein x i x j -> features     |
| Started : 27/04/2010 18:50               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEIN_INFERENCE_RESIDUE_PAIR_FEATURES_H_
# define LBCPP_PROTEIN_INFERENCE_RESIDUE_PAIR_FEATURES_H_

# include "ProteinResidueFeatures.h"

namespace lbcpp
{

class ProteinResiduePairFeatures : public Object
{
public:
  virtual String getName() const
    {return getClassName();}

  virtual FeatureGeneratorPtr compute(ProteinObjectPtr protein, size_t firstPosition, size_t secondPosition) = 0;
};

typedef ReferenceCountedObjectPtr<ProteinResiduePairFeatures> ProteinResiduePairFeaturesPtr;

extern ProteinResiduePairFeaturesPtr proteinUnitResiduePairFeature();
extern ProteinResiduePairFeaturesPtr proteinPointResiduePairFeatures(ProteinResidueFeaturesPtr pointFeatureGenerator);
extern ProteinResiduePairFeaturesPtr proteinGlobalToResiduePairFeatures(ProteinGlobalFeaturesPtr globalFeatures);
extern ProteinResiduePairFeaturesPtr proteinCentralCompositionResiduePairFeatures(const String& sequenceName);
extern ProteinResiduePairFeaturesPtr proteinPositionIndexResiduePairFeature();

extern ProteinResiduePairFeaturesPtr separationLengthResiduePairFeatures(size_t intervalsPerLog10 = 5);
extern ProteinResiduePairFeaturesPtr separationDistanceResiduePairFeatures(size_t intervalsPerLog10 = 2);
extern ProteinResiduePairFeaturesPtr contactMapResiduePairFeatures(const String& contactMapName, size_t size = 5);


enum ResiduePairConjunctionType
{
  aaResiduePairConjunction,
  aaCategoryResiduePairConjunction,
  proteinLengthResiduePairConjunction
};

extern ProteinResiduePairFeaturesPtr conjunctionResiduePairFeatures(ResiduePairConjunctionType conjunctionType, ProteinResiduePairFeaturesPtr baseFeatures);

class CompositeProteinResiduePairFeatures : public ProteinResiduePairFeatures
{
public:
  void addSubFeatures(ProteinResiduePairFeaturesPtr subFeatures)
    {subFeatureFunctions.push_back(subFeatures);}

  virtual String toString() const;

  virtual FeatureGeneratorPtr compute(ProteinObjectPtr protein, size_t firstPosition, size_t secondPosition);

  size_t getNumSubFeatureFunctions() const
    {return subFeatureFunctions.size();}

  ProteinResiduePairFeaturesPtr getSubFeatureFunction(size_t i) const
    {jassert(i < subFeatureFunctions.size()); return subFeatureFunctions[i];}

protected:
  std::vector<ProteinResiduePairFeaturesPtr> subFeatureFunctions;
  
  virtual bool load(InputStream& istr);
  virtual void save(OutputStream& ostr) const;
};

typedef ReferenceCountedObjectPtr<CompositeProteinResiduePairFeatures> CompositeProteinResiduePairFeaturesPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_PROTEIN_INFERENCE_RESIDUE_PAIR_FEATURES_H_