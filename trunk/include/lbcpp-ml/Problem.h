/*-----------------------------------------.---------------------------------.
| Filename: Problem.h                      | Optimization Problem            |
| Author  : Francis Maes                   |                                 |
| Started : 22/09/2012 20:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_PROBLEM_H_
# define LBCPP_ML_PROBLEM_H_

# include "Domain.h"
# include "Objective.h"
# include "Fitness.h"

namespace lbcpp
{

class Problem : public Object
{
public:
  Problem(DomainPtr domain, ObjectivePtr objective)
    : domain(domain), objectives(1, objective) {}
  Problem(DomainPtr domain)
    : domain(domain) {}
  Problem() {}

  /*
  ** Domain
  */
  DomainPtr getDomain() const
    {return domain;}
  
  void setDomain(DomainPtr domain)
    {this->domain = domain;}
  
  /*
  ** Objectives
  */
  size_t getNumObjectives() const
    {return objectives.size();}

  ObjectivePtr getObjective(size_t index) const
    {jassert(index < objectives.size()); return objectives[index];}
  
  void addObjective(ObjectivePtr objective)
    {objectives.push_back(objective);}

  /*
  ** Validation Objectives
  */
  size_t getNumValidationObjectives() const
    {return validationObjectives.size();}

  ObjectivePtr getValidationObjective(size_t index) const
    {jassert(index < validationObjectives.size()); return validationObjectives[index];}
  
  void addValidationObjective(ObjectivePtr objective)
    {validationObjectives.push_back(objective);}

  /*
  ** Initial Guess
  */
  ObjectPtr getInitialGuess() const
    {return initialGuess;}

  void setInitialGuess(ObjectPtr initialGuess)
    {this->initialGuess = initialGuess;}

  /*
  ** Evaluation
  */
  FitnessLimitsPtr getFitnessLimits() const;
  FitnessPtr evaluate(ExecutionContext& context, const ObjectPtr& object);

  /*
  ** Loading / Initialization
  */
  void reinitialize(ExecutionContext& context);
  virtual bool loadFromString(ExecutionContext& context, const string& str);

  lbcpp_UseDebuggingNewOperator

protected:
  DomainPtr domain;
  std::vector<ObjectivePtr> objectives;
  std::vector<ObjectivePtr> validationObjectives;
  ObjectPtr initialGuess;

  virtual void initialize(ExecutionContext& context) {}

private:
  FitnessLimitsPtr limits;
};

}; /* namespace lbcpp */

#endif // !LBCPP_ML_PROBLEM_H_