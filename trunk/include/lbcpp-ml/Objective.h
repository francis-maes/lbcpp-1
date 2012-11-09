/*-----------------------------------------.---------------------------------.
| Filename: Objective.h                    | Optimization Objective          |
| Author  : Francis Maes                   |                                 |
| Started : 08/11/2012 11:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_ML_OBJECTIVE_H_
# define LBCPP_ML_OBJECTIVE_H_

# include <lbcpp/Data/DoubleVector.h>
# include <lbcpp/Luape/LuapeCache.h>
# include "DataTable.h"

namespace lbcpp
{

class ScalarProperty : public Object
{
public:
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object) = 0;
};

class Objective : public ScalarProperty
{
public:
  virtual void getObjectiveRange(double& worst, double& best) const = 0;
};

class DifferentiableObjective : public Objective
{
public:
  virtual void evaluate(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, double* value, DoubleVectorPtr* gradient) = 0;
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
    {double res; evaluate(context, object.staticCast<DenseDoubleVector>(), &res, NULL); return res;}

  bool testDerivativeWithRandomDirection(ExecutionContext& context, const DenseDoubleVectorPtr& parameters);
  bool testDerivative(ExecutionContext& context, const DenseDoubleVectorPtr& parameters, const DoubleVectorPtr& direction);
};

class StochasticObjective : public Objective
{
public:
  virtual size_t getNumInstances() const {return 0;} // 0 stands for infinity
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object, size_t instanceIndex) = 0;
  
  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object)
  {
    size_t numInstances = getNumInstances();
    if (!numInstances)
      numInstances = 1000;
    double res = 0.0;
    for (size_t i = 0; i < numInstances; ++i)
      res += evaluate(context, object, i);
    return res / (double)numInstances;
  }
};

class LearningObjective : public StochasticObjective
{
public:
  LearningObjective(DataTablePtr data = DataTablePtr())
    : data(data) {}

  virtual double evaluate(ExecutionContext& context, const ObjectPtr& object, size_t instanceIndex)
    {jassertfalse; return 0.0;}
  
  virtual size_t getNumInstances() const
    {return data->getNumSamples();}

  LuapeSampleVectorPtr computePredictions(ExecutionContext& context, ExpressionPtr expression) const
    {return expression->compute(context, data);}

protected:
  friend class LearningObjectiveClass;

  DataTablePtr data;
};

class SupervisedLearningObjective : public LearningObjective
{
public:
  SupervisedLearningObjective(DataTablePtr data, VariableExpressionPtr supervision)
    : LearningObjective(data), supervision(supervision) {}
  SupervisedLearningObjective() {}

  VectorPtr getSupervisions() const
    {return data->getSamplesByExpression(supervision);}

protected:
  friend class SupervisedLearningObjectiveClass;

  VariableExpressionPtr supervision;
};

extern SupervisedLearningObjectivePtr binaryAccuracyObjective(DataTablePtr data, VariableExpressionPtr supervision);
extern SupervisedLearningObjectivePtr multiClassAccuracyObjective(DataTablePtr data, VariableExpressionPtr supervision);

extern SupervisedLearningObjectivePtr mseRegressionObjective(DataTablePtr data, VariableExpressionPtr supervision);
extern SupervisedLearningObjectivePtr rmseRegressionObjective(DataTablePtr data, VariableExpressionPtr supervision);
extern SupervisedLearningObjectivePtr normalizedRMSERegressionObjective(DataTablePtr data, VariableExpressionPtr supervision);

// todo: StochasticDifferentiableObjective

}; /* namespace lbcpp */

#endif // !LBCPP_ML_OBJECTIVE_H_
