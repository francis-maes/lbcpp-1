/*-----------------------------------------.---------------------------------.
| Filename: DiscretizedNumberFeatures.h    | Base class for discretize       |
| Author  : Francis Maes                   | number feature generators       |
| Started : 04/10/2010 13:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_NUMERICAL_LEARNING_PERCEPTION_DISCRETIZED_NUMBER_H_
# define LBCPP_NUMERICAL_LEARNING_PERCEPTION_DISCRETIZED_NUMBER_H_

# include <lbcpp/Perception/Perception.h>

namespace lbcpp
{
  
class DiscretizedNumberFeatures : public Perception
{
public:
  DiscretizedNumberFeatures(TypePtr inputType, double minimumValue, double maximumValue, size_t numIntervals, bool doOutOfBoundsFeatures)
    : inputType(inputType), minimumValue(minimumValue), maximumValue(maximumValue), numIntervals(numIntervals), doOutOfBoundsFeatures(doOutOfBoundsFeatures)
  {
    jassert(maximumValue > minimumValue);
    jassert(numIntervals > 1);
    isDouble = inputType->inheritsFrom(doubleType);
  }

  DiscretizedNumberFeatures()
    : minimumValue(0.0), maximumValue(0.0), numIntervals(0), doOutOfBoundsFeatures(false) {}

  virtual TypePtr getInputType() const
    {return inputType;}

  virtual bool isSparse() const
    {return true;}

  virtual bool loadFromXml(XmlImporter& importer)
  {
    if (!Perception::loadFromXml(importer))
      return false;
    isDouble = inputType->inheritsFrom(doubleType);
    return true;
  }

protected:
  friend class DiscretizedNumberFeaturesClass;

  TypePtr inputType;
  double minimumValue;
  double maximumValue;
  size_t numIntervals;
  bool doOutOfBoundsFeatures;

  bool isDouble;

  virtual double getValue(const Variable& input) const
    {jassert(input.exists()); return isDouble ? input.getDouble() : (double)input.getInteger();}

  virtual String getBoundaryName(size_t index) const
    {return String(getBoundary(index));}

  double getBoundary(size_t index) const
  {
    jassert(index <= numIntervals);
    return minimumValue + (maximumValue - minimumValue) * (double)index / numIntervals;
  }
};

}; /* namespace lbcpp */

#endif // !LBCPP_NUMERICAL_LEARNING_PERCEPTION_DISCRETIZED_NUMBER_H_