/*-----------------------------------------.---------------------------------.
| Filename: CRAlgorithm.hpp                | Static to Dynamic CR-algorithm  |
| Author  : Francis Maes                   |   Wrapper                       |
| Started : 04/02/2009 19:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef LBCPP_STATIC_CRALGORITHM_HPP_
# define LBCPP_STATIC_CRALGORITHM_HPP_

# include "../../CRAlgorithm.h"
# include "CRAlgorithmScope.hpp"
# include "Choose.hpp"
# include "Callback.hpp"

namespace lbcpp
{

template<class T_impl>
class StaticToDynamicCRAlgorithm
  : public StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithm, StaticToDynamicCRAlgorithm<T_impl> >
{
public:
  typedef StaticToDynamicCRAlgorithmScope_<T_impl, CRAlgorithm, StaticToDynamicCRAlgorithm<T_impl> > BaseClassType;

  StaticToDynamicCRAlgorithm(T_impl* newImpl) : BaseClassType(newImpl) {}

  virtual String getName() const
    {return BaseClassType::getImplementation().getName();}

  virtual bool hasReturn() const
    {return BaseClassType::getImplementation().hasReturn();}

  virtual VariablePtr getReturn() const
  {
    const T_impl& impl = BaseClassType::getImplementation();
    if (impl.hasReturn())
      return Variable::createFromPointer(const_cast<typename T_impl::ReturnType* >(impl.getReturn()),
                impl.getReturnType(), "'" + getName() + "' return value");
    else
      return VariablePtr();
  }

  virtual void run(PolicyPtr policy, VariablePtr choice)
  {
    T_impl& impl = BaseClassType::getImplementation();
    if (impl.__state__ < 0)
    {
      lbcpp::Object::error("CRAlgorithm::run(policy, choice)", "This is not an initial state, in order to run a policy from the initial state, use run(policy).");
      jassert(false);
    }
    PolicyToStaticCallback staticCallback(policy, CRAlgorithmPtr(this), choice);
    while (!stepImpl(staticCallback, staticCallback.getLastChoice()));      
  }
    
  virtual bool run(PolicyPtr policy)
  {
    // todo: inlined version
    jassert(BaseClassType::getImplementation().__state__ == -1); // in order to run a policy from a non-initial state, use run(policy, choice)
    PolicyToStaticCallback staticCallback(policy, CRAlgorithmPtr(this));
    if (stepImpl(staticCallback, VariablePtr()))
      return true;
    while (true)
    {
      VariablePtr choice = staticCallback.getLastChoice();
      if (!choice)
      {
        Object::error("CRAlgorithm::run", "No choices made");
        return false;
      }
      if (stepImpl(staticCallback, choice))
        return true;
    }
  }

  virtual bool step(Callback& callback, VariablePtr choice)
  {
    DynamicToStaticCallback<T_impl> staticCallback(callback, CRAlgorithmPtr(this));
    return stepImpl(staticCallback, choice);
  }

  virtual ChoosePtr runUntilFirstChoose(double* reward = NULL)
  {
    T_impl& impl = BaseClassType::getImplementation();
    if (impl.__state__ >= 0)
    {
      lbcpp::Object::error("CRAlgorithm::runUntilFirstChoose", "This is not an initial state, use the runUntilNextChoose() function.");
      jassert(false);
      return ChoosePtr();
    }
    jassert(impl.__state__ == -1);
    return runUntilNextChoose(NULL, reward);
  }
  
  virtual ChoosePtr runUntilNextChoose(VariablePtr choice, double* reward = NULL)
  {
    StoreChooseAndRewardStaticCallback<T_impl> callback(CRAlgorithmPtr(this));
    stepImpl(callback, choice);
    if (reward)
      *reward = callback.getCurrentReward();
    return callback.getCurrentChoose();
  }
  
private:
  template<class CallbackType>
  bool stepImpl(CallbackType& callback, VariablePtr choice) // returns true if the cr-algorithm is finished
  {
    T_impl& impl = BaseClassType::getImplementation();
    int res = T_impl::__step__(impl, impl, callback, choice ? choice->getUntypedPointer() : NULL);
    return res == stateFinish || res == stateReturn;
  }
};

template<class CRAlgorithmType>
inline CRAlgorithmType& dynamicToStaticCRAlgorithm(CRAlgorithmPtr crAlgorithm)
{
  typedef StaticToDynamicCRAlgorithm<CRAlgorithmType> CRAlgorithmWrapperType;
  
  CRAlgorithmWrapperType* lbcpp = dynamic_cast<CRAlgorithmWrapperType* >(crAlgorithm.get());
  jassert(lbcpp); // todo: error message
  return lbcpp->getImplementation();
}

template<class CRAlgorithmType>
inline CRAlgorithmPtr staticToDynamicCRAlgorithm(CRAlgorithmType* newStaticCRAlgorithm)
{
  return CRAlgorithmPtr(new StaticToDynamicCRAlgorithm<CRAlgorithmType>(newStaticCRAlgorithm));
}

}; /* namespace lbcpp */

#endif // !LBCPP_STATIC_CRALGORITHM_HPP_
