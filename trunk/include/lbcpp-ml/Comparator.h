/*-----------------------------------------.---------------------------------.
| Filename: Comparator.h                   | Solution Comparator             |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:26               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_COMPARATOR_H_
# define LBCPP_ML_COMPARATOR_H_

# include "Solution.h"

namespace lbcpp
{

class MOOSolutionComparator : public Object
{
public:
  virtual void initialize(const MOOSolutionSetPtr& solutions) = 0;

  // returns -1 if solution1 is prefered, +1 if solution2 is prefered and 0 if there is no preference between the two solutions
  virtual int compare(size_t index1, size_t index2) = 0;
};

extern MOOSolutionComparatorPtr objectiveComparator(size_t index);
extern MOOSolutionComparatorPtr lexicographicComparator();
extern MOOSolutionComparatorPtr dominanceComparator();
extern MOOSolutionComparatorPtr paretoRankAndCrowdingDistanceComparator();

}; /* namespace lbcpp */

#endif // !LBCPP_ML_COMPARATOR_H_
