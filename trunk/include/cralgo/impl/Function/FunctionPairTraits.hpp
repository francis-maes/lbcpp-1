/*-----------------------------------------.---------------------------------.
| Filename: FunctionPairTraits.hpp         | Traits for function pairs       |
| Author  : Francis Maes                   |                                 |
| Started : 07/03/2009 15:06               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef CRALGO_IMPL_FUNCTION_PAIR_TRAITS_H_
# define CRALGO_IMPL_FUNCTION_PAIR_TRAITS_H_

# include "ContinuousFunction.hpp"

namespace cralgo
{
namespace impl
{

template<class Function1, class Function2, class Parameter>
struct ContinuousFunctionPair
{
  struct LinearBinaryOperation {}; 
  struct Composition {}; 
  struct Multiplication {};
};

/*
** Function vs Function
*/
#define _BEGIN_DECLARE_FUNCTION_PAIR(Traits, BaseClass1, BaseClass2, Parameter, OperationName, FunctionName) \
template<class Function1, class Function2> \
inline typename Traits<Function1, Function2, Parameter>::OperationName \
  FunctionName (const BaseClass1< Function1 >& l, const BaseClass2< Function2 >& r) { \
  typedef Traits<Function1, Function2, Parameter> Traits; \
  typedef typename Traits::OperationName ReturnType; \
  const Function1& left = static_cast<const Function1& >(l); \
  const Function2& right = static_cast<const Function2& >(r);

#define DECLARE_FUNCTION_COMPOSE(Traits, BaseClass1, BaseClass2) \
  _BEGIN_DECLARE_FUNCTION_PAIR(Traits, BaseClass1, BaseClass2, void, Composition, compose) \
      return ReturnType(left, right); }

#define DECLARE_FUNCTION_MULTIPLY(Traits, BaseClass1, BaseClass2) \
  _BEGIN_DECLARE_FUNCTION_PAIR(Traits, BaseClass1, BaseClass2, void, Multiplication, multiply) \
      return ReturnType(left, right); }

#define DECLARE_FUNCTION_LINEAR_BINARY_FUNCTIONS(Traits, BaseClass1, BaseClass2) \
  _BEGIN_DECLARE_FUNCTION_PAIR(Traits, BaseClass1, BaseClass2, AdditionBinaryScalarOperation, LinearBinaryOperation, add) \
      return ReturnType(*(const AdditionBinaryScalarOperation* )0, left, right); } \
  _BEGIN_DECLARE_FUNCTION_PAIR(Traits, BaseClass1, BaseClass2, SubstractionBinaryScalarOperation, LinearBinaryOperation, substract) \
      return ReturnType(*(const SubstractionBinaryScalarOperation* )0, left, right); }

/*
** Function vs Constant
*/
#define _BEGIN_DECLARE_FUNCTION_PAIR_RIGHT_CONST(Traits, BaseClass1, Parameter, OperationName, FunctionName) \
template<class Function1> \
inline typename Traits<Function1, Parameter>::OperationName \
  FunctionName (const BaseClass1< Function1 >& l, const ScalarConstant& right) { \
  typedef Traits<Function1, Parameter> Traits; \
  typedef typename Traits::OperationName ReturnType; \
  const Function1& left = static_cast<const Function1& >(l);

#define DECLARE_FUNCTION_COMPOSE_RIGHT_CONST(Traits, BaseClass1) \
  _BEGIN_DECLARE_FUNCTION_PAIR_RIGHT_CONST(Traits, BaseClass1, void, Composition, compose) \
      return ReturnType(left, right); }

#define DECLARE_FUNCTION_MULTIPLY_RIGHT_CONST(Traits, BaseClass1) \
  _BEGIN_DECLARE_FUNCTION_PAIR_RIGHT_CONST(Traits, BaseClass1, void, Multiplication, multiply) \
      return ReturnType(left, right); }

#define DECLARE_FUNCTION_LINEAR_BINARY_FUNCTIONS_RIGHT_CONST(Traits, BaseClass1) \
  _BEGIN_DECLARE_FUNCTION_PAIR_RIGHT_CONST(Traits, BaseClass1, AdditionBinaryScalarOperation, LinearBinaryOperation, add) \
      return ReturnType(*(const AdditionBinaryScalarOperation* )0, left, right); } \
  _BEGIN_DECLARE_FUNCTION_PAIR_RIGHT_CONST(Traits, BaseClass1, SubstractionBinaryScalarOperation, LinearBinaryOperation, substract) \
      return ReturnType(*(const SubstractionBinaryScalarOperation* )0, left, right); }


/*
** ScalarFunction vs ScalarFunction
*/
template<class Function1, class Function2, class Parameter>
struct ScalarFunctionPair : public ContinuousFunctionPair<Function1, Function2, Parameter>
{
  struct LinearBinaryOperation : public ScalarFunction<LinearBinaryOperation>
  {
    LinearBinaryOperation(const Parameter& operation, const Function1& left, const Function2& right)
      : operation(operation), left(left), right(right) {}
      
    const Parameter& operation;
    Function1 left;
    Function2 right;
    
    enum {isDerivable = Function1::isDerivable && Function2::isDerivable};
    
    void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {
      double leftOutput, rightOutput;
      double leftDerivative, rightDerivative;
      left.compute(input, output ? &leftOutput : NULL, derivativeDirection, derivative ? &leftDerivative : NULL);
      left.compute(input, output ? &rightOutput : NULL, derivativeDirection, derivative ? &rightDerivative : NULL);
      if (output)
        *output = operation.compute(leftOutput, rightOutput);
      if (derivative)
        *derivative = operation.computeDerivativeWrtLeft(leftOutput, rightOutput) * leftDerivative 
            + operation.computeDerivativeWrtRight(leftOutput, rightOutput) * rightDerivative;
    }
  };
  
  struct Composition : public ScalarFunction<Composition>
  {
    Composition(const Function1& left, const Function2& right)
      : left(left), right(right) {}
      
    Function1 left;
    Function2 right;
    
    enum {isDerivable = Function1::isDerivable && Function2::isDerivable};

    void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {
      double leftOutput;
      double leftDerivative, rightDerivative;
      left.compute(input, &leftOutput, derivativeDirection, derivative ? &leftDerivative : NULL);
      right.compute(leftOutput, output, derivative ? &leftDerivative : NULL, derivative ? &rightDerivative : NULL);
      if (derivative)
        *derivative = leftDerivative * rightDerivative;
    }
  };
   
  struct Multiplication : public ScalarFunction<Composition>
  {
    Multiplication(const Function1& left, const Function2& right)
      : left(left), right(right) {}
      
    Function1 left;
    Function2 right;
    
    enum {isDerivable = Function1::isDerivable && Function2::isDerivable};
  
    void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
    {
      double leftOutput, rightOutput;
      double leftDerivative, rightDerivative;
      left.compute(input, leftOutput, derivativeDirection, derivative ? &leftDerivative : NULL);
      right.compute(input, rightOutput, derivativeDirection, derivative ? &rightDerivative : NULL);
      if (output)
        *output = leftOutput * rightOutput;
      if (derivative)
        *derivative = leftOutput * rightDerivative + leftDerivative * rightOutput;
    }
  };
};

DECLARE_FUNCTION_COMPOSE(ScalarFunctionPair, ScalarFunction, ScalarFunction);
DECLARE_FUNCTION_MULTIPLY(ScalarFunctionPair, ScalarFunction, ScalarFunction);
DECLARE_FUNCTION_LINEAR_BINARY_FUNCTIONS(ScalarFunctionPair, ScalarFunction, ScalarFunction);

/*
** ScalarVectorFunction vs Scalar constant
*/
template<class Function1, class Parameter>
struct ScalarVectorFunctionScalarConstantPair
  : public ContinuousFunctionPair<Function1, ScalarConstant, Parameter>
{
  struct LinearBinaryOperation {}; // FIXME
  struct Composition {}; // FIXME
  
  struct Multiplication : public ScalarVectorFunction<Multiplication>
  {
    Multiplication(const Function1& left, const ScalarConstant& right)
      : left(left), right(right.compute()) {}

    Function1 left;
    double right;
    
    enum {isDerivable = Function1::isDerivable};
    
    void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {
      if (right)
      {
        left.compute(input, output, gradientDirection, gradient);
        if (output)
          *output *= right;
        if (gradient)
          gradient->multiplyByScalar(right);
      }
      else
      {
        if (output) *output = 0;
      }
    }
  };
};

DECLARE_FUNCTION_MULTIPLY_RIGHT_CONST(ScalarVectorFunctionScalarConstantPair, ScalarVectorFunction);
/*
template<class Function1>
inline typename ScalarVectorFunctionScalarConstantPair<Function1, void>::Multiplication
  multiply(const ScalarVectorFunction<Function1>& left, const ScalarConstant& right)
{
  typedef ScalarVectorFunctionScalarConstantPair<Function1, void> Traits;
  typedef typename Traits::Multiplication ReturnType;
  return ReturnType(static_cast<const Function1& >(left), right);
}*/

/*
** ScalarVectorFunction vs ScalarVectorFunction
*/
template<class Function1, class Function2, class Parameter>
struct ScalarVectorFunctionPair : public ContinuousFunctionPair<Function1, Function2, Parameter>
{
  // FIXME: multiplication, composition

  // op(f1(x), f2(x)) : R^n -> R
  struct LinearBinaryOperation : public ScalarVectorFunction< LinearBinaryOperation >
  {
    LinearBinaryOperation(const Parameter& operation, const Function1& left, const Function2& right)
      : operation(operation), left(left), right(right) {}
    
    const Parameter& operation;
    Function1 left;
    Function2 right;

    enum {isDerivable = Function1::isDerivable && Function2::isDerivable};
    
    void compute(const FeatureGeneratorPtr input, double* output, const FeatureGeneratorPtr gradientDirection, LazyVectorPtr gradient) const
    {
      double leftOutput, rightOutput;
      LazyVectorPtr leftGradient, rightGradient;
      if (gradient)
      {
        leftGradient = new LazyVector();
        rightGradient = new LazyVector();
      }
      left.compute(input, output ? &leftOutput : NULL, gradientDirection, leftGradient);
      right.compute(input, output ? &rightOutput : NULL, gradientDirection, rightGradient);      
      if (output)
        *output = operation.compute(leftOutput, rightOutput);
      if (gradient)
      {
        gradient->addWeighted(leftGradient, operation.computeDerivativeWrtLeft(leftOutput, rightOutput));
        gradient->addWeighted(rightGradient, operation.computeDerivativeWrtRight(leftOutput, rightOutput));
      }
    }
  };
};

//DECLARE_FUNCTION_COMPOSE(ScalarVectorFunctionPair, ScalarVectorFunction, ScalarVectorFunction);
//DECLARE_FUNCTION_MULTIPLY(ScalarVectorFunctionPair, ScalarVectorFunction, ScalarVectorFunction);
DECLARE_FUNCTION_LINEAR_BINARY_FUNCTIONS(ScalarVectorFunctionPair, ScalarVectorFunction, ScalarVectorFunction);

/*template<class Function1, class Function2>
inline typename ScalarVectorFunctionPair<Function1, Function2, AdditionBinaryScalarOperation>::BinaryLinearOperation
  add(const ScalarVectorFunction<Function1>& left, const ScalarVectorFunction<Function2>& right)
{
  typedef ScalarVectorFunctionPair<Function1, Function2, AdditionBinaryScalarOperation> Traits;
  typedef typename Traits::BinaryLinearOperation ReturnType;
  return ReturnType(*(const AdditionBinaryScalarOperation* )0, static_cast<const Function1& >(left), static_cast<const Function2& >(right));
}*/

/*
** Scalar Architecture vs Scalar Constant
*/
template<class Function1, class Parameter>
struct ScalarArchitectureScalarConstantPair : public ContinuousFunctionPair<Function1, ScalarConstant, Parameter>
{
  struct LinearBinaryOperation : public ScalarArchitecture< LinearBinaryOperation >
  {
    LinearBinaryOperation(const Parameter& operation, const Function1& left, const ScalarConstant& right)
      : operation(operation), left(left), right(right.compute()) {}
      
    const Parameter& operation;
    Function1 left;
    double right;
    
    enum {isDerivable = Function1::isDerivable};
    
    void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
        double* output,
        LazyVectorPtr gradientWrtParameters,
        LazyVectorPtr gradientWrtInput) const
    {
      double leftOutput;
      left.compute(parameters, input, leftOutput, gradientWrtParameters, gradientWrtInput);
      if (output)
        *output = operation.compute(leftOutput, right);
      double k = left.computeDerivativeWrtLeft(leftOutput, right);
      if (gradientWrtParameters)
        gradientWrtParameters->multiplyByScalar(k);
      if (gradientWrtInput)
        gradientWrtInput->multiplyByScalar(k);
    }
  };

  struct Composition {};
};

DECLARE_FUNCTION_LINEAR_BINARY_FUNCTIONS_RIGHT_CONST(ScalarArchitectureScalarConstantPair, ScalarArchitecture);
/*
template<class Function1>
inline typename ScalarArchitectureScalarConstantPair<Function1, SubstractionBinaryScalarOperation>::LinearBinaryOperation
  substract(const ScalarArchitecture<Function1>& left, const ScalarConstant& right)
{
  typedef ScalarArchitectureScalarConstantPair<Function1, SubstractionBinaryScalarOperation> Traits;
  typedef typename Traits::LinearBinaryOperation ReturnType;
  return ReturnType(*(const SubstractionBinaryScalarOperation* )0, left, right);
}*/

/*
** Scalar Architecture vs Scalar Function
*/
template<class Function1, class Function2, class Parameter>
struct ScalarArchitectureScalarFunctionPair : public ContinuousFunctionPair<Function1, Function2, Parameter>
{
  struct LinearBinaryOperation {}; // undefined

  struct Composition : public ScalarArchitecture< Composition >
  {
    Composition(const Function1& left, const Function2& right)
      : left(left), right(right) {}
      
    Function1 left;
    Function2 right;

    enum {isDerivable = Function1::isDerivable && Function2::isDerivable};
      
    void compute(const DenseVectorPtr parameters, const FeatureGeneratorPtr input,
        double* output,
        LazyVectorPtr gradientWrtParameters,
        LazyVectorPtr gradientWrtInput) const
    {
      double leftOutput;
      left.compute(parameters, input, &leftOutput, gradientWrtParameters, gradientWrtInput);
      
      double rightDerivative;
      const static double zero = 0; // FIXME : derivative direction
      right.compute(leftOutput, output, &zero, gradientWrtParameters || gradientWrtInput ? &rightDerivative : NULL);
      
      if (gradientWrtParameters)
        gradientWrtParameters->multiplyByScalar(rightDerivative);
      
      if (gradientWrtInput)
        gradientWrtInput->multiplyByScalar(rightDerivative);
    }
  };
};


DECLARE_FUNCTION_COMPOSE(ScalarArchitectureScalarFunctionPair, ScalarArchitecture, ScalarFunction);
/*
template<class Function1, class Function2>
inline typename ScalarArchitectureScalarFunctionPair<Function1, Function2, void>::Composition
  compose(const ScalarArchitecture<Function1>& left, const ScalarFunction<Function2>& right)
{
  typedef ScalarArchitectureScalarFunctionPair<Function1, Function2, void> Traits;
  typedef typename Traits::Composition ReturnType;
  return ReturnType(left, right);
}*/


}; /* namespace impl */
}; /* namespace cralgo */

#endif // !CRALGO_IMPL_FUNCTION_PAIR_TRAITS_H_
