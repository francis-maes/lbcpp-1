/*-----------------------------------------.---------------------------------.
| Filename: ResidueHistogram..Generator.h  | Residue Histogram               |
| Author  : Alejandro Marcos Alvarez       | Feature Generator               |
| Started : Feb 16, 2012  3:24:43 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_
# define LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_

# include <lbcpp/FeatureGenerator/FeatureGenerator.h>
# include "../Pose.h"
# include "../../../Data/AminoAcid.h"

namespace lbcpp
{

class ResidueHistogramFeatureGenerator : public FeatureGenerator
{
public:
  virtual size_t getNumRequiredInputs() const
    {return 1;}

  virtual TypePtr getRequiredInputType(size_t index, size_t numInputs) const
    {return poseClass;}

  virtual EnumerationPtr initializeFeatures(ExecutionContext& context, const std::vector<VariableSignaturePtr>& inputVariables, TypePtr& elementsType, String& outputName, String& outputShortName)
    {return aminoAcidTypeEnumeration;}

  virtual void computeFeatures(const Variable* inputs, FeatureGeneratorCallback& callback) const
  {
    DenseDoubleVectorPtr histogram = (inputs[0].getObjectAndCast<Pose> ())->getHistogram();
    for (size_t i = 0; i < histogram->getNumElements(); i++)
      callback.sense(i, histogram->getValue(i));
  }

protected:
  friend class ResidueHistogramFeatureGeneratorClass;
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_DATA_FEATURES_RESIDUEHISTOGRAMFEATUREGENERATOR_H_


//    // inputs and variables
//    size_t input = builder.addInput(rosettaProteinClass);
//    size_t numResidues = builder.addFunction(getVariableFunction(T("numResidues")), input);
//    size_t energy = builder.addFunction(getVariableFunction(T("normalizedScore")), input);
//    //size_t histogram = builder.addFunction(getVariableFunction(T("histogram")), input);
//    size_t shortDistance = builder.addFunction(getVariableFunction(T("shortDistance")), input);
//    size_t longDistance = builder.addFunction(getVariableFunction(T("longDistance")), input);
//
//    // select features
//    builder.startSelection();
//    size_t residueFeatures;
//    size_t energyFeatures;
//    size_t histogramFeatures;
//    size_t shortDistancesFeatures;
//    size_t longDistancesFeatures;
//    if (selectResiduesFeatures)
//      residueFeatures = builder.addFunction(softDiscretizedLogNumberFeatureGenerator(1, std::log10(100.0), 10, true), numResidues);
//    if (selectEnergyFeatures)
//      energyFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), energy);
//    if (selectHistogramFeatures)
//      histogramFeatures = builder.addFunction(new RosettaProteinResidueHistogramFeatureGenerator(), input);
//    if (selectDistanceFeatures)
//    {
//      shortDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), shortDistance);
//      longDistancesFeatures = builder.addFunction(defaultProbabilityFeatureGenerator(10), longDistance);
//    }
//
//    // end selection
//    builder.finishSelectionWithFunction(concatenateFeatureGenerator());
