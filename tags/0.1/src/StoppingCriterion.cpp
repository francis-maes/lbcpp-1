/*-----------------------------------------.---------------------------------.
| Filename: StoppingCriterion.cpp          | Stopping Criterions             |
| Author  : Francis Maes                   |                                 |
| Started : 09/06/2009 16:40               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/StoppingCriterion.h>
#include <deque>
#include <cfloat>
using namespace lbcpp;

class MaxIterationsStoppingCriterion : public StoppingCriterion
{
public:
  MaxIterationsStoppingCriterion(size_t maxIterations = 0)
    : maxIterations(maxIterations) {}

  virtual std::string toString() const
    {return "MaxIterations(" + lbcpp::toString(maxIterations) + ")";}

  virtual void reset()
    {iterations = 0;}

  bool shouldStop()
    {assert(maxIterations); ++iterations; return iterations >= maxIterations;}

  virtual bool shouldOptimizerStop(double)
    {return shouldStop();}

  virtual bool shouldCRAlgorithmLearnerStop(PolicyPtr, ObjectContainerPtr)
    {return shouldStop();}

  virtual void save(std::ostream& ostr) const
    {write(ostr, maxIterations);}

  virtual bool load(std::istream& istr)
    {return read(istr, maxIterations);}

private:
  size_t iterations, maxIterations;
};

StoppingCriterionPtr lbcpp::maxIterationsStoppingCriterion(size_t maxIterations)
  {return new MaxIterationsStoppingCriterion(maxIterations);}

class AverageImprovementStoppingCriterion : public StoppingCriterion
{
public:
  AverageImprovementStoppingCriterion(double tolerance = 0.001, bool relativeImprovment = false)
    : tolerance(tolerance), relativeImprovment(relativeImprovment) {}
    
  virtual std::string toString() const
    {return "AvgImprovment(" + lbcpp::toString(tolerance) + ")";}

  virtual void reset()
    {prevs.clear();}

  virtual bool shouldOptimizerStop(double value)
  {
    if (prevs.size())
    {
      double prevVal = prevs.front();
      /*if (energy > prevVal)
        prevs.clear();
      else */if (prevs.size() >= 5)
      {
        double averageImprovement = (prevVal - value) / prevs.size();
        if (relativeImprovment)
        {
          double relAvgImpr = value ? averageImprovement / fabs(value) : 0;
  //        std::cout << "Av-Improvment: " << averageImprovement << " RealImprovment: " << relAvgImpr << " tol = " << tolerance << std::endl;
          if ((averageImprovement >= 0 && averageImprovement < DBL_EPSILON) || (relAvgImpr >= 0 && relAvgImpr <= tolerance))
            return true;
        }
        else if (averageImprovement >= 0 && averageImprovement <= tolerance)
          return true;
        prevs.pop_front();
      }
    }
    prevs.push_back(value);
    return false;
  }

  virtual bool shouldCRAlgorithmLearnerStop(PolicyPtr policy, ObjectContainerPtr examples)
  {
    assert(false); // not implemented
    return false;
  }

  virtual void save(std::ostream& ostr) const
    {write(ostr, tolerance);}

  virtual bool load(std::istream& istr)
    {return read(istr, tolerance);}

private:
  double tolerance;
  bool relativeImprovment;
  std::deque<double> prevs;
};

StoppingCriterionPtr lbcpp::averageImprovementStoppingCriterion(double tolerance, bool relativeImprovment)
  {return new AverageImprovementStoppingCriterion(tolerance, relativeImprovment);}

class LogicalOrStoppingCriterion : public StoppingCriterion
{
public:
  LogicalOrStoppingCriterion(StoppingCriterionPtr criterion1, StoppingCriterionPtr criterion2)
    : criterion1(criterion1), criterion2(criterion2) {}
  LogicalOrStoppingCriterion() {}

  virtual std::string toString() const
    {return criterion1->toString() + " || " + criterion2->toString();}

  virtual void reset()
    {criterion1->reset(); criterion2->reset();}

  virtual bool shouldOptimizerStop(double value)
  {
    bool t1 = criterion1->shouldOptimizerStop(value);
    bool t2 = criterion2->shouldOptimizerStop(value);
    return t1 || t2;
  }

  virtual bool shouldCRAlgorithmLearnerStop(PolicyPtr policy, ObjectContainerPtr examples)
  {
    bool t1 = criterion1->shouldCRAlgorithmLearnerStop(policy, examples);
    bool t2 = criterion2->shouldCRAlgorithmLearnerStop(policy, examples);
    return t1 || t2;
  }
  
  virtual void save(std::ostream& ostr) const
    {write(ostr, criterion1); write(ostr, criterion2);}

  virtual bool load(std::istream& istr)
    {return read(istr, criterion1) && read(istr, criterion2);}

private:
  StoppingCriterionPtr criterion1;
  StoppingCriterionPtr criterion2;
};

StoppingCriterionPtr lbcpp::logicalOr(StoppingCriterionPtr criterion1, StoppingCriterionPtr criterion2)
  {return new LogicalOrStoppingCriterion(criterion1, criterion2);}

/*
** Serializable classes declaration
*/
void declareStoppingCriterions()
{
  LBCPP_DECLARE_CLASS(MaxIterationsStoppingCriterion);
  LBCPP_DECLARE_CLASS(AverageImprovementStoppingCriterion);
  LBCPP_DECLARE_CLASS(LogicalOrStoppingCriterion);
}