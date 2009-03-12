/*-----------------------------------------.---------------------------------.
| Filename: StaticToDynamic.hpp            | Static to dynamic bridge        |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 19:24               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
                               
#ifndef CRALGO_IMPL_STATIC_TO_DYNAMIC_H_
# define CRALGO_IMPL_STATIC_TO_DYNAMIC_H_

# include "Object.hpp"
# include "Macros.hpp"

/*
** Static To Dynamic functions
*/
#define STATIC_TO_DYNAMIC_BEGIN_0(_Class, _BaseClass) \
  TEMPLATE_INHERIT_BEGIN_1(StaticToDynamic##_Class, StaticToDynamic##_BaseClass, ImplementationType) \
  StaticToDynamic##_Class(const ImplementationType& impl) : BaseClass(impl) {}

#define STATIC_TO_DYNAMIC_BEGIN_1(_Class, _BaseClass, BaseClassArg0) \
  TEMPLATE_INHERIT_BEGIN_2(StaticToDynamic##_Class, StaticToDynamic##_BaseClass, ImplementationType, BaseClassArg0) \
  StaticToDynamic##_Class(const ImplementationType& impl) : BaseClass(impl) {}

#define STATIC_TO_DYNAMIC_BEGIN_0_0(Class, BaseClass) \
  template<class ImplementationType> STATIC_TO_DYNAMIC_BEGIN_0(Class, BaseClass)

#define STATIC_TO_DYNAMIC_BEGIN_0_1(Class, BaseClass, BaseClassArg0) \
  template<class ImplementationType> STATIC_TO_DYNAMIC_BEGIN_1(Class, BaseClass, BaseClassArg0) 

#define STATIC_TO_DYNAMIC_BEGIN_1_1(Class, ClassArg0, BaseClass, BaseClassArg0) \
  template<class ImplementationType, class ClassArg0> STATIC_TO_DYNAMIC_BEGIN_1(Class, BaseClass, BaseClassArg0) 

#define STATIC_TO_DYNAMIC_END_0(Class) \
}; \
template<class ExactType> \
inline Class##Ptr staticToDynamic(const Class<ExactType>& impl) \
  {return Class##Ptr(new StaticToDynamic##Class<ExactType>(static_cast<const ExactType& >(impl)));}

#define STATIC_TO_DYNAMIC_END_1(Class) \
}; \
template<class ExactType, class ClassArg0> \
inline Class##Ptr staticToDynamic(const Class<ExactType, ClassArg0>& impl) \
  {return Class##Ptr(new StaticToDynamic##Class<ExactType>(static_cast<const ExactType& >(impl)));}

namespace cralgo {
namespace impl {

template<class ImplementationType, class BaseClass>
class StaticToDynamicObject : public BaseClass
{
public:
  StaticToDynamicObject(const ImplementationType& impl)
    : impl(impl) {}

  virtual bool load(std::istream& istr)
    {return impl.load(istr);}
    
  virtual void save(std::ostream& ostr) const
    {impl.save(ostr);}

  virtual std::string toString() const
    {return impl.toString();}
  
  virtual std::string getName() const
    {return impl.getName();}
    
protected:
  ImplementationType impl;
};
  
}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_STATIC_TO_DYNAMIC_H_
