/*-----------------------------------------.---------------------------------.
 | Filename: SharkSOOptimizers.h            | Wrapper for Single-Objective    |
 | Author  : Denny Verbeeck                 | Shark Optimizers                |
 | Started : 12/03/2013 21:03               |                                 |
 `------------------------------------------/                                 |
                                |                                             |
                                `--------------------------------------------*/

#ifndef ML_SO_OPTIMIZER_SHARK_H_
# define ML_SO_OPTIMIZER_SHARK_H_

# include <ml/Solver.h>
# undef T
# include <shark/Algorithms/DirectSearch/CMA.h>
# include <shark/ObjectiveFunctions/AbstractObjectiveFunction.h>
# include <shark/Core/SearchSpaces/VectorSpace.h>
# define T JUCE_T

namespace shark
{

class SharkObjectiveFunctionFromProblem : public AbstractObjectiveFunction< VectorSpace<double>,double >
{
public:
  SharkObjectiveFunctionFromProblem(lbcpp::ExecutionContext& context, lbcpp::ProblemPtr problem, lbcpp::SolverPtr solver)
  : context(context), problem(problem), solver(solver)
  {
    m_name = problem->toShortString();
    lbcpp::ScalarVectorDomainPtr domain = problem->getDomain().staticCast<lbcpp::ScalarVectorDomain>();
    m_numberOfVariables = domain->getNumDimensions();
    std::vector<double> lower(domain->getNumDimensions());
    std::vector<double> upper(domain->getNumDimensions());
    for (size_t i = 0; i < lower.size(); ++i)
    {
      lower[i] = domain->getLowerLimit(i);
      upper[i] = domain->getUpperLimit(i);
    }
    //constrainthandler = new BoxConstraintHandler(lower, upper); 
  }
  virtual ~SharkObjectiveFunctionFromProblem()
    {/*delete constrainthandler;*/}
  
  virtual unsigned int objectives() const
    {return (int)problem->getNumObjectives();}
  
  virtual void result(double* const& point, std::vector<double>& value)
  {
    lbcpp::DenseDoubleVectorPtr solution = new lbcpp::DenseDoubleVector((size_t)m_numberOfVariables, 0.0);
    memcpy(solution->getValuePointer(0), point, sizeof (double) * m_numberOfVariables);
    lbcpp::FitnessPtr fitness = problem->evaluate(context, solution);
    jassert(fitness);
    value = fitness->getValues();
    solver->addSolution(context, solution, fitness);
    m_evaluationCounter++;
  }
  
  virtual bool ProposeStartingPoint(double*& point) const
  {
    lbcpp::DenseDoubleVectorPtr solution = problem->getInitialGuess().staticCast<lbcpp::DenseDoubleVector>();
    if (!solution)
      return false;
    memcpy(point, solution->getValuePointer(0), sizeof (double) * m_numberOfVariables);
    return true;
  }

  virtual size_t numberOfVariables() const
    {return m_numberOfVariables;}
  
protected:
  lbcpp::ExecutionContext& context;
  lbcpp::ProblemPtr problem;
  lbcpp::SolverPtr solver;
  size_t m_numberOfVariables;
};

}; /* namespace shark */

namespace lbcpp
{

class CMAESSOOptimizer : public IterativeSolver
{
public:
  CMAESSOOptimizer(size_t numGenerations = 0)
    : IterativeSolver(numGenerations), objective(NULL), cma(NULL) {}
  
  virtual void startSolver(ExecutionContext& context, ProblemPtr problem, SolverCallbackPtr callback, ObjectPtr startingSolution)
  {
    IterativeSolver::startSolver(context, problem, callback, startingSolution);
    objective = new shark::SharkObjectiveFunctionFromProblem(context, problem, refCountedPointerFromThis(this));
    cma = new shark::CMA();
  }
  
  virtual bool iterateSolver(ExecutionContext& context, size_t iter)
  {
    jassert(cma && objective);
    if (iter == 0)
      cma->init(*objective);
    else
      cma->step(*objective);
    return true;
  }
  
  virtual void stopSolver(ExecutionContext& context)
  {
    deleteAndZero(cma);
    deleteAndZero(objective);
    IterativeSolver::stopSolver(context);
  }
  
protected:
  friend class CMAESSOOptimizerClass;
  
  shark::SharkObjectiveFunctionFromProblem* objective;
  shark::CMA* cma;
};

}; /* namespace lbcpp */

#endif // !ML_SO_OPTIMIZER_SHARK_H_