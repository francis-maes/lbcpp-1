/*-----------------------------------------.---------------------------------.
| Filename: IncrementalLearner.h           | Incremental Learner             |
| Author  : Francis Maes                   |                                 |
| Started : 14/03/2013 14:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef ML_INCREMENTAL_LEARNER_H_
# define ML_INCREMENTAL_LEARNER_H_

# include <ml/Expression.h>
# include <ml/Solver.h>

namespace lbcpp
{

class IncrementalLearnerCallback;
typedef ReferenceCountedObjectPtr<IncrementalLearnerCallback> IncrementalLearnerCallbackPtr;
  
class IncrementalLearner : public Object
{
public:
  IncrementalLearner() : verbosity((SolverVerbosity) 0), callback(IncrementalLearnerCallbackPtr()) {}

  virtual ExpressionPtr createExpression(ExecutionContext& context, ClassPtr supervisionType) const = 0;
  /** This method adds a training sample to the incremental learning algorithm. This method is the most general case,
   *  if your incremental learner can deal with non-numerical data, you should probably override this method in child
   *  classes. By default this method transforms the first \f$n-1\f$ elements to a DenseDoubleVector representing the
   *  input, and the last element to a DenseDoubleVector representing the output.
   *  \param context The execution context
   *  \param sample The training sample, represented as a vector of length \f$n\f$. The first
   *                \f$n-1\f$ elements are considered the feature vector. The \f$n\f$th element
   *                is considered the target value.
   *  \param expression The model which should be updated
   */
  virtual void addTrainingSample(ExecutionContext& context, const std::vector<ObjectPtr>& sample, ExpressionPtr expression) const
  {
    DenseDoubleVectorPtr input = new DenseDoubleVector(sample.size() - 1, 0.0);
    for (size_t i = 0; i < input->getNumValues(); ++i)
      input->setValue(i, Double::get(sample[i]));
    
    DenseDoubleVectorPtr output;;
    ObjectPtr supervision = sample.back();
    if (supervision.isInstanceOf<Double>())
      output = new DenseDoubleVector(1, Double::get(supervision));
    else
    {
      std::vector<double> vec = supervision.staticCast<DenseDoubleVector>()->getValues();
      output = new DenseDoubleVector(supervision.staticCast<DenseDoubleVector>()->getNumValues(), 0.0);
      size_t i = 0;
      for (std::vector<double>::iterator it = vec.begin(); it != vec.end(); ++it)
        output->setValue(i++,*it);
    }
    addTrainingSample(context, expression, input, output);
  }
  
  virtual void addTrainingSample(ExecutionContext& context, ExpressionPtr expression, const DenseDoubleVectorPtr& input, const DenseDoubleVectorPtr& output) const = 0;

  virtual void initialiseLearnerStatistics(ExecutionContext& context, ExpressionPtr model, ObjectPtr data) const {}

  void setVerbosity(SolverVerbosity verbosity)
    {this->verbosity = verbosity;}

  SolverVerbosity getVerbosity()
    {return verbosity;}

  void setCallback(IncrementalLearnerCallbackPtr callback)
    {this->callback = callback;}

protected:
  SolverVerbosity verbosity;
  IncrementalLearnerCallbackPtr callback;
};

typedef ReferenceCountedObjectPtr<IncrementalLearner> IncrementalLearnerPtr;

/** Base class for incremental splitting criteria 
 *  Subclasses should implement findBestSplit
 */
class IncrementalSplittingCriterion : public Object
{
public:
  struct Split
  {
    Split() : attribute(DVector::missingValue), value(DVector::missingValue), quality(0.0) {}
    Split(size_t attribute, double value, double quality) : attribute(attribute), value(value), quality(quality) {}
    size_t attribute;
    double value;
    double quality;

    /** Compares this Split's attribute and value against the other's attribute
     *  and value. Returns true if both are equal. Note that the quality does not
     *  need to be equal.
     */
    bool isSameSplitAs(const Split& other)
      {return attribute == other.attribute && value == other.value;}
  };

  /** Find the best split point for this leaf.
   *  \return A Split struct containing the attribute index, split value and split quality.
   *          If no suitable Split was found, set attribute and value to DVector::missingValue to indicate this.
   */
  virtual Split findBestSplit(ExecutionContext& context, TreeNodePtr leaf) const = 0;
  virtual double splitQuality(ScalarVariableMeanAndVariancePtr leftVariance, PearsonCorrelationCoefficientPtr leftCorrelation,
    ScalarVariableMeanAndVariancePtr rightVariance, PearsonCorrelationCoefficientPtr rightCorrelation) const = 0;
  virtual double splitQuality(MultiVariateRegressionStatisticsPtr total, MultiVariateRegressionStatisticsPtr left, MultiVariateRegressionStatisticsPtr right) const
  {
    jassertfalse;
    return 0.0;
  }
};

typedef ReferenceCountedObjectPtr<IncrementalSplittingCriterion> IncrementalSplittingCriterionPtr;

extern IncrementalLearnerPtr pureRandomScalarVectorTreeIncrementalLearner();
extern IncrementalLearnerPtr ensembleIncrementalLearner(IncrementalLearnerPtr baseLearner, size_t ensembleSize);
extern IncrementalLearnerPtr perceptronIncrementalLearner(size_t numInitialTrainingSamples, double learningRate, double learningRateDecay);
extern IncrementalLearnerPtr hoeffdingTreeIncrementalLearner(IncrementalSplittingCriterionPtr splittingCriterion, IncrementalLearnerPtr perceptronLearner);
extern IncrementalLearnerPtr simpleLinearRegressionIncrementalLearner();
extern IncrementalLearnerPtr linearLeastSquaresRegressionIncrementalLearner();

extern IncrementalSplittingCriterionPtr hoeffdingBoundMauveIncrementalSplittingCriterion(size_t chunkSize, double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundExtendedMauveIncrementalSplittingCriterion(size_t chunkSize, double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundStdDevReductionIncrementalSplittingCriterion(size_t chunkSize, double delta, double threshold);
extern IncrementalSplittingCriterionPtr hoeffdingBoundTotalMauveIncrementalSplittingCriterion(size_t chunkSize, double delta, double threshold);
/*extern IncrementalSplittingCriterionPtr hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(double delta, double threshold);
extern IncrementalSplittingCriterionPtr mauveIncrementalSplittingCriterion(double delta, double threshold, double maxCoefficientOfDetermination);
extern IncrementalSplittingCriterionPtr quandtAndrewsIncrementalSplittingCriterion(size_t numVariables, double threshold);*/
extern IncrementalSplittingCriterionPtr nullIncrementalSplittingCriterion();


class IncrementalLearnerStatistics : public Object
{
public:
  IncrementalLearnerStatistics() : examplesSeen(0) {}

  size_t getExamplesSeen()
    {return examplesSeen;}

  void incrementExamplesSeen(size_t amount = 1)
    {examplesSeen += amount;}

protected:
  friend class IncrementalLearnerStatisticsClass;

  size_t examplesSeen;
};

extern IncrementalLearnerStatisticsPtr hoeffdingTreeIncrementalLearnerStatistics(size_t numAttributes = 0);
extern IncrementalLearnerStatisticsPtr hoeffdingTreeIncrementalLearnerStatistics(ExecutionContext& context, IncrementalLearnerStatisticsPtr parentStats, size_t attribute, double splitValue, bool leftSide);


/** Callbacks **/
class IncrementalLearnerCallback : public Object
{
public:
  virtual void exampleAdded(ExecutionContext& context, ExpressionPtr model) = 0;
};

class EvaluatorIncrementalLearnerCallback : public IncrementalLearnerCallback
{
public:
  EvaluatorIncrementalLearnerCallback(string name, ObjectivePtr evaluationObjective) : name(name), objective(evaluationObjective) {}
  
  virtual void exampleAdded(ExecutionContext& context, ExpressionPtr model)
    {context.resultCallback(name, objective->evaluate(context, model));}
protected:
  string name;
  ObjectivePtr objective;
};

class CurveBuilderIncrementalLearnerCallback : public IncrementalLearnerCallback
{
public:
  CurveBuilderIncrementalLearnerCallback(string name, ObjectivePtr evaluationObjective, size_t period) : name(name), objective(evaluationObjective), period(period), numCalls(0) {}
  
  virtual void exampleAdded(ExecutionContext& context, ExpressionPtr model)
    {if (++numCalls % period == 0) curve.push_back(std::make_pair(numCalls, objective->evaluate(context, model)));}

  std::vector< std::pair<size_t, double> > getCurve()
    {return curve;}
protected:
  string name;
  ObjectivePtr objective;
  size_t period;
  size_t numCalls;
  std::vector< std::pair<size_t, double> > curve;
};

typedef ReferenceCountedObjectPtr<CurveBuilderIncrementalLearnerCallback> CurveBuilderIncrementalLearnerCallbackPtr;

class EvaluationPeriodIncrementalLearnerCallback : public IncrementalLearnerCallback
{
public:
  EvaluationPeriodIncrementalLearnerCallback(IncrementalLearnerCallbackPtr callback, size_t period) : callback(callback), period(period), numCalls(0) {}

  virtual void exampleAdded(ExecutionContext& context, ExpressionPtr model)
    {if (++numCalls % period == 0) callback->exampleAdded(context, model);}
protected:
  IncrementalLearnerCallbackPtr callback;
  size_t period;
  size_t numCalls;
};

typedef ReferenceCountedObjectPtr<EvaluationPeriodIncrementalLearnerCallback> EvaluationPeriodIncrementalLearnerCallbackPtr;

}; /* namespace lbcpp */

#endif // !ML_INCREMENTAL_LEARNER_H_
