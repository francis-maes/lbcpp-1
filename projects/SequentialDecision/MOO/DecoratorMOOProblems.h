/*-----------------------------------------.---------------------------------.
| Filename: DecoratorMOOProblems.h         | Decorator MOO Problems          |
| Author  : Francis Maes                   |                                 |
| Started : 12/09/2012 15:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_MOO_PROBLEM_DECORATOR_H_
# define LBCPP_MOO_PROBLEM_DECORATOR_H_

# include "MOOCore.h"

namespace lbcpp
{

class DecoratorMOOProblem : public MOOProblem
{
public:
  DecoratorMOOProblem(MOOProblemPtr problem = MOOProblemPtr())
    : problem(problem) {}

  virtual MOODomainPtr getSolutionDomain() const
    {return problem->getSolutionDomain();}

  virtual MOOFitnessLimitsPtr getFitnessLimits() const
    {return problem->getFitnessLimits();}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {return problem->evaluate(context, solution);}

  virtual ObjectPtr proposeStartingSolution(ExecutionContext& context) const
    {return problem->proposeStartingSolution(context);}

  virtual bool shouldStop() const
    {return problem->shouldStop();}

protected:
  friend class DecoratorMOOProblemClass;

  MOOProblemPtr problem;
};

class MaxNumIterationsDecoratorMOOProblem : public DecoratorMOOProblem
{
public:
  MaxNumIterationsDecoratorMOOProblem(MOOProblemPtr problem, size_t maxNumEvaluations)
    : DecoratorMOOProblem(problem), maxNumEvaluations(maxNumEvaluations), numEvaluations(0) {}
  MaxNumIterationsDecoratorMOOProblem() : maxNumEvaluations(0), numEvaluations(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
    {jassert(numEvaluations < maxNumEvaluations); ++numEvaluations; return DecoratorMOOProblem::evaluate(context, solution);}

  virtual bool shouldStop() const
    {return numEvaluations >= maxNumEvaluations || DecoratorMOOProblem::shouldStop();}

  size_t getNumEvaluations() const
    {return numEvaluations;}

protected:
  friend class MaxNumIterationsDecoratorMOOProblemClass;

  size_t maxNumEvaluations;

  size_t numEvaluations;
};

class HyperVolumeEvaluatorDecoratorMOOProblem : public MaxNumIterationsDecoratorMOOProblem
{
public:
  HyperVolumeEvaluatorDecoratorMOOProblem(MOOProblemPtr problem, size_t maxNumEvaluations, size_t hyperVolumeComputationPeriod)
    : MaxNumIterationsDecoratorMOOProblem(problem, maxNumEvaluations), hyperVolumeComputationPeriod(hyperVolumeComputationPeriod)
    {front = new MOOParetoFront(problem->getFitnessLimits());}
  HyperVolumeEvaluatorDecoratorMOOProblem() : hyperVolumeComputationPeriod(0) {}

  virtual MOOFitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& solution)
  {
    if (numEvaluations == 0)
      firstEvaluationTime = juce::Time::getMillisecondCounterHiRes() / 1000.0;
    MOOFitnessPtr res = MaxNumIterationsDecoratorMOOProblem::evaluate(context, solution);
    front->insert(solution, res);
    if ((numEvaluations % hyperVolumeComputationPeriod) == 0)
    {
      hyperVolumes.push_back(front->computeHyperVolume(problem->getFitnessLimits()->getWorstPossibleFitness()));
      cpuTimes.push_back(juce::Time::getMillisecondCounterHiRes() / 1000.0 - firstEvaluationTime);
    }
    return res;
  }

  const std::vector<double>& getHyperVolumes() const
    {return hyperVolumes;}

  const std::vector<double>& getCpuTimes() const
    {return cpuTimes;}

  size_t getHyperVolumeComputationPeriod() const
    {return hyperVolumeComputationPeriod;}

protected:
  friend class HyperVolumeEvaluatorDecoratorMOOProblemClass;

  size_t hyperVolumeComputationPeriod;

  MOOParetoFrontPtr front;
  double firstEvaluationTime;
  std::vector<double> cpuTimes;
  std::vector<double> hyperVolumes;
};

typedef ReferenceCountedObjectPtr<HyperVolumeEvaluatorDecoratorMOOProblem> HyperVolumeEvaluatorDecoratorMOOProblemPtr;

}; /* namespace lbcpp */

#endif // !LBCPP_MOO_PROBLEM_DECORATOR_H_