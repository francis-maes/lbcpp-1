/*-----------------------------------------.---------------------------------.
| Filename: RandomVariable.h               | Random variable statistics      |
| Author  : Francis Maes                   |                                 |
| Started : 12/03/2009 16:18               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_RANDOM_VARIABLE_H_
# define LBCPP_RANDOM_VARIABLE_H_

# include "ObjectPredeclarations.h"
# include <cfloat>

namespace lbcpp
{

class ScalarRandomVariableMean : public Object
{
public:
  ScalarRandomVariableMean(const std::string& name = "")
    : name(name), value(0), cnt(0) {}
  
  void push(double val)
    {value = (value * cnt + val) / (cnt + 1); ++cnt;}

  void push(double val, double weight)
    {value = (value * cnt + val) / (cnt + weight); cnt += weight;}
  
  double getMean() const
    {return value;}
  
  double getCount() const
    {return cnt;}
    
  double getSum() const
    {return value * cnt;}

  virtual std::string getName() const
    {return name;}

  virtual std::string toString() const
    {return lbcpp::toString(getMean());}

  virtual void save(std::ostream& ostr) const
  { 
    write(ostr, name);
    write(ostr, value);
    write(ostr, cnt);
  }
  
  virtual bool load(std::istream& istr)
    {return read(istr, name) && read(istr, value) && read(istr, cnt);}

protected:
  std::string name;
  double value;
  double cnt;
};

class ScalarRandomVariableMeanAndVariance : public ScalarRandomVariableMean
{
public:
  ScalarRandomVariableMeanAndVariance(const std::string& name = "")
    : ScalarRandomVariableMean(name) {}
    
  void push(double val)
    {ScalarRandomVariableMean::push(val); meansqr.push(sqr(val));}

  void push(double val, double weight)
    {ScalarRandomVariableMean::push(val, weight); meansqr.push(sqr(val), weight);}
  
  double getVariance() const
    {return meansqr.getMean() - sqr(getMean());}

  double getStandardDeviation() const
    {double v = getVariance(); return v > DBL_EPSILON ? sqrt(v) : 0.0;}

  virtual std::string toString() const
    {return ScalarRandomVariableMean::toString() + " +/- " + lbcpp::toString(getStandardDeviation());}

  virtual void save(std::ostream& ostr) const
  { 
    ScalarRandomVariableMean::save(ostr);
    meansqr.save(ostr);
  }
  
  virtual bool load(std::istream& istr)
    {return ScalarRandomVariableMean::load(istr) && meansqr.load(istr);}

private:
  ScalarRandomVariableMean meansqr;
  
  static inline double sqr(double x)
    {return x * x;}
};

class ScalarRandomVariableStatistics : public ScalarRandomVariableMeanAndVariance
{
 public:
  ScalarRandomVariableStatistics(const std::string& name = "")
    : ScalarRandomVariableMeanAndVariance(name), min(DBL_MAX), max(-DBL_MAX) {}

  void push(double val)
  {
    ScalarRandomVariableMeanAndVariance::push(val);
    if (val < min)
	    min = val;
    if (val > max)
	    max = val;
  }

  void push(double val, double weight)
  {
    ScalarRandomVariableMeanAndVariance::push(val, weight);
    if (val < min)
	    min = val;
    if (val > max)
	    max = val;
  }

  double getMinimum() const
    {return min;}
    
  double getMaximum() const
    {return max;}
    
  double getRange() const
    {return max - min;}

  virtual std::string toString() const
  {
    return ScalarRandomVariableMeanAndVariance::toString() + " [" +
      lbcpp::toString(min) + " - " + lbcpp::toString(max) + "]";
  }

  virtual void save(std::ostream& ostr) const
  { 
    ScalarRandomVariableMeanAndVariance::save(ostr);
    write(ostr, min);
    write(ostr, max);
  }
  
  virtual bool load(std::istream& istr)
    {return ScalarRandomVariableMeanAndVariance::load(istr) && read(istr, min) && read(istr, max);}

private:
  double min, max;
};

}; /* namespace lbcpp */

#endif // !LBCPP_RANDOM_VARIABLE_H_
