/*-----------------------------------------.---------------------------------.
| Filename: RandomPolicy.h                 | RandomGenerator Policy                   |
| Author  : Francis Maes                   |                                 |
| Started : 11/06/2009 21:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_POLICY_RANDOM_H_
# define LBCPP_POLICY_RANDOM_H_

# include <lbcpp/CRAlgorithm/Policy.h>

namespace lbcpp
{

class RandomPolicy : public Policy
{
public:
  virtual String toString() const
    {return "randomPolicy()";}

  virtual VariablePtr policyChoose(ChoosePtr choose)
    {return choose->sampleRandomChoice();}
};

}; /* namespace impl */

#endif // !LBCPP_POLICY_RANDOM_H_