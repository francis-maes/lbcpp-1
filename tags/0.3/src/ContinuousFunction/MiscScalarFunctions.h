/*-----------------------------------------.---------------------------------.
| Filename: MiscScalarFunctions.h          | Misc. R -> R functions          |
| Author  : Francis Maes                   |                                 |
| Started : 03/05/2010 15:46               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_CONTINUOUS_FUNCTION_MISC_SCALAR_FUNCTIONS_H_
# define LBCPP_CONTINUOUS_FUNCTION_MISC_SCALAR_FUNCTIONS_H_

# include <lbcpp/FeatureGenerator/ContinuousFunction.h>

namespace lbcpp
{

class ScalarFunctionComposition : public ScalarFunction
{
public:
  ScalarFunctionComposition(ScalarFunctionPtr f1, ScalarFunctionPtr f2)
    : f1(f1), f2(f2) {}
  ScalarFunctionComposition() {}

  virtual bool isDerivable() const
    {return f1->isDerivable() && f2->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    double f1output, f1derivative;
    f1->compute(input, &f1output, derivativeDirection, derivative ? &f1derivative : NULL);
    f2->compute(f1output, output, NULL, derivative);
    if (derivative)
      *derivative *= f1derivative;
  }

private:
  ScalarFunctionPtr f1;
  ScalarFunctionPtr f2;

  virtual bool load(InputStream& istr)
    {return ScalarFunction::load(istr) && lbcpp::read(istr, f1) && lbcpp::read(istr, f2);}

  virtual void save(OutputStream& ostr) const
    {ScalarFunction::save(ostr); lbcpp::write(ostr, f1); lbcpp::write(ostr, f2);}
};

class MultiplyByScalarFunction : public ScalarFunction
{
public:
  MultiplyByScalarFunction(ScalarFunctionPtr function, double scalar)
    : function(function), scalar(scalar) {}
  MultiplyByScalarFunction() : scalar(0.0) {}

  virtual bool isDerivable() const
    {return function->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (scalar == 0.0)
    {
      if (output) *output = 0.0;
      if (derivative) *derivative = 0.0;
    }
    else if (scalar == 1.0)
      function->compute(input, output, derivativeDirection, derivative);
    else
    {
      double dd;
      if (derivativeDirection)
        dd = *derivativeDirection * (scalar < 0.0 ? -1.0 : 1.0);
      function->compute(input, output, derivativeDirection ? &dd : NULL, derivative);
      if (output)
        *output *= scalar;
      if (derivative)
        *derivative *= scalar;
    }
  }

protected:
  ScalarFunctionPtr function;
  double scalar;

  virtual bool load(InputStream& istr)
    {return ScalarFunction::load(istr) && lbcpp::read(istr, function) && lbcpp::read(istr, scalar);}

  virtual void save(OutputStream& ostr) const
    {ScalarFunction::save(ostr); lbcpp::write(ostr, function); lbcpp::write(ostr, scalar);}
};

class AddConstantScalarFunction : public ScalarFunction
{
public:
  AddConstantScalarFunction(double constant)
    : constant(constant) {}

  virtual bool isDerivable() const
    {return true;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = input + constant;
    if (derivative)
      *derivative = 1.0;
  }

private:
  double constant;
};

class ScalarFunctionPlusConstant : public ScalarFunction
{
public:
  ScalarFunctionPlusConstant(ScalarFunctionPtr function, double constant)
    : function(function), constant(constant) {}

  virtual bool isDerivable() const
    {return function->isDerivable();}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    function->compute(input, output, derivativeDirection, derivative);
    if (output)
      *output += constant;
  }

protected:
  ScalarFunctionPtr function;
  double constant;
};

class AngleDifferenceScalarFunction : public ScalarFunction
{
public:
  AngleDifferenceScalarFunction(double referenceAngle = 0.0)
    : referenceAngle(referenceAngle) {}

  virtual bool isDerivable() const
    {return false;}

  virtual void compute(double input, double* output, const double* derivativeDirection, double* derivative) const
  {
    if (output)
      *output = normalizeAngle(input - referenceAngle);
    if (derivative)
      *derivative = 1.0;
  }

protected:
  double referenceAngle;

  virtual bool load(InputStream& istr)
    {return ScalarFunction::load(istr) && lbcpp::read(istr, referenceAngle);}

  virtual void save(OutputStream& ostr) const
    {ScalarFunction::save(ostr); lbcpp::write(ostr, referenceAngle);}
};

}; /* namespace lbcpp */

#endif // !LBCPP_CONTINUOUS_FUNCTION_MISC_SCALAR_FUNCTIONS_H_