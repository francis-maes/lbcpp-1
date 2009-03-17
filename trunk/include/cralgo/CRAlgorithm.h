/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithm.h                  | CR-algorithm base class         |
| Author  : Francis Maes                   |                                 |
| Started : 04/02/2009 18:09               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_CRALGORITHM_H_
# define CRALGO_CRALGORITHM_H_

# include "Choose.h"
# include "Callback.h"
# include "Policy.h"
# include "CRAlgorithmScope.h"

namespace cralgo
{

class CRAlgorithm : public CRAlgorithmScope
{
public:
  /*
  ** Clone/assignment
  */
  CRAlgorithmPtr clone() const
    {return cloneScope().dynamicCast<CRAlgorithm>();}

  CRAlgorithm& operator =(const CRAlgorithm& otherCRAlgorithm)
    {setScope(otherCRAlgorithm); return *this;}
   
  /*
  ** Policy running
  */
  virtual bool run(PolicyPtr policy) = 0; // run a policy from the initial state
  virtual void run(PolicyPtr policy, VariablePtr choice) = 0; // run a policy from the current state
    
  /*
  ** Step by step
  */
  virtual bool step(Callback& callback, VariablePtr choice) = 0; // returns false if the new state is a final state
  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL) = 0; // returns false if there was no choose
  virtual ChoosePtr runUntilNextChoose(VariablePtr choice, double* reward = NULL) = 0; // returns false if there was no remaining chooses
  
  /*
  ** Result
  */
  virtual bool hasReturn() const = 0;
  virtual VariablePtr getReturn() const = 0;
};

}; /* namespace cralgo */

#endif // !CRALGO_CRALGORITHM_H_
