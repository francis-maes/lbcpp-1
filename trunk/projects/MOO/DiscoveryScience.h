/*---------------------------------------------.-----------------------------------------.
 | Filename: DiscoveryScience.h                | Experiments for Discovery Science 2014  |
 | Author  : Denny Verbeeck                    |                                         |
 | Started : 18/04/2014 11:25                  |                                         |
 `---------------------------------------------/                                         |
                                  |                                                      |
                                  `-----------------------------------------------------*/

#ifndef MOO_DISCOVERY_SCIENCE_H_
# define MOO_DISCOVERY_SCIENCE_H_

# include <oil/Execution/WorkUnit.h>
# include <ml/Solver.h>
# include <ml/IncrementalLearner.h>
# include <ml/SelectionCriterion.h>
# include <ml/Domain.h>
# include <ml/IncrementalLearner.h>
# include <ml/Sampler.h>
# include <math.h>
# include "SharkProblems.h"
# include "../../lib/ml/Learner/IncrementalLearnerBasedLearner.h"
# include "../../lib/ml/Loader/ArffLoader.h"

namespace lbcpp
{

extern void lbCppMLLibraryCacheTypes(ExecutionContext& context); // tmp

class XValWorkUnit : public WorkUnit
{
public:
  XValWorkUnit() {}
  XValWorkUnit(std::vector<ProblemPtr> folds, size_t chunkSize, SolverVerbosity verbosity, string problemName) 
    : folds(folds), chunkSize(chunkSize), verbosity(verbosity), problemName(problemName) {}

  virtual ObjectPtr run(ExecutionContext& context)
  {
    std::vector<double> deltas;
    deltas.push_back(0.001);
    deltas.push_back(0.005);
    deltas.push_back(0.01);
    deltas.push_back(0.05);
    deltas.push_back(0.10);
    deltas.push_back(0.15);
    deltas.push_back(0.20);
    deltas.push_back(0.25);

    std::vector<double> thresholds;
    thresholds.push_back(0.001);
    thresholds.push_back(0.005);
    thresholds.push_back(0.01);
    thresholds.push_back(0.05);
    thresholds.push_back(0.1);
    thresholds.push_back(0.15);
    thresholds.push_back(0.20);
    thresholds.push_back(0.25);

    std::vector<string> algoNames;
    algoNames.push_back("FIMTp");
    algoNames.push_back("FIMTslr");
    algoNames.push_back("iMauve");


    context.enterScope(problemName);
    runSolvers(context, 0.005, 0.1, algoNames);
    /*
    for (size_t d = 0; d < deltas.size(); ++d)
    {
      context.enterScope("delta = " + string(deltas[d]));
      context.resultCallback("delta", deltas[d]);
      for (size_t t = 0; t < thresholds.size(); ++t)
      {
        context.enterScope("threshold = " + string(thresholds[t]));
        context.resultCallback("threshold", thresholds[t]);
        runSolvers(context, deltas[d], thresholds[t], algoNames);
        context.leaveScope();
      }
      context.leaveScope();
    }*/
    context.leaveScope();
    return new Boolean(true);
  }

protected:
  std::vector<ProblemPtr> folds;
  size_t chunkSize;
  SolverVerbosity verbosity;
  string problemName;

  ExpressionPtr solveProblem(ExecutionContext& context, const ProblemPtr& problem, const SolverPtr& learner)
  {
    ExpressionPtr model;
    FitnessPtr fitness;
    ObjectivePtr problemObj = problem->getObjective(0);
    const TablePtr& problemData = problemObj.staticCast<LearningObjective>()->getData();
    learner->solve(context, problem, storeBestSolverCallback(*(ObjectPtr* )&model, fitness));
    if (verbosity > verbosityDetailed)
    {
      context.resultCallback("model", model);
      context.resultCallback("data", problemData);
      context.resultCallback("fitness", fitness);
    }
    return model;
  }

  std::pair<double,double> doCrossValidation(ExecutionContext& context, const std::vector<ProblemPtr>& folds, SolverPtr learner, string algoName)
  {
    ScalarVariableMean meanTesting;
    ScalarVariableMean meanTreeSize;
    //context.enterScope(algoName);
    for (size_t foldNb = 0; foldNb < folds.size(); ++foldNb)
    {
      context.progressCallback(new ProgressionState(foldNb, folds.size(), "Folds"));
      context.enterScope("Fold " + string((int) foldNb));
      ExpressionPtr model = solveProblem(context, folds[foldNb], learner);
      
      double testingScore = folds[foldNb]->getValidationObjective(0)->evaluate(context, model);
      size_t nbLeaves = model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves();
      //context.resultCallback("testingScore", testingScore);
      //context.resultCallback("tree size", nbLeaves);
      context.leaveScope(testingScore);
      meanTesting.push(testingScore);
      meanTreeSize.push(nbLeaves);
    }
    context.progressCallback(new ProgressionState(folds.size(), folds.size(), "Folds"));
    //context.resultCallback("Mean testing score", meanTesting.getMean());
    //context.resultCallback("Mean tree size", meanTreeSize.getMean());
    //context.leaveScope(meanTesting.getMean());
    return std::make_pair(meanTesting.getMean(), meanTreeSize.getMean());
  }

  void runSolvers(ExecutionContext& context, double delta, double threshold, const std::vector<string>& algoNames)
  {
    std::vector<SolverPtr> solvers;
    /*
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(delta, threshold), perceptronIncrementalLearner(20, 0.1, 0.005), chunkSize)));
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(hoeffdingBoundStdDevReductionIncrementalSplittingCriterion2(delta, threshold), simpleLinearRegressionIncrementalLearner(), chunkSize)));
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(mauveIncrementalSplittingCriterion(delta, threshold, 2.0), simpleLinearRegressionIncrementalLearner(), chunkSize)));
    */
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(nullIncrementalSplittingCriterion(), perceptronIncrementalLearner(20, 0.1, 0.005), chunkSize)));
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(nullIncrementalSplittingCriterion(), simpleLinearRegressionIncrementalLearner(), chunkSize)));
    solvers.push_back(incrementalLearnerBasedLearner(hoeffdingTreeIncrementalLearner(nullIncrementalSplittingCriterion(), simpleLinearRegressionIncrementalLearner(), chunkSize)));

    for (size_t s = 0; s < solvers.size(); ++s)
    {
      context.progressCallback(new ProgressionState(s, solvers.size(), "Solvers"));
      context.enterScope(algoNames[s]);
      solvers[s]->setVerbosity(verbosity);
      //std::pair<double, double> results = doCrossValidation(context, folds, solvers[s], algoNames[s]);
      //context.resultCallback(algoNames[s] + " RRE", results.first);
      //context.resultCallback(algoNames[s] + " TreeSize", results.second);
      //context.leaveScope(results.first);

      ExpressionPtr model = solveProblem(context, folds[0], solvers[s]);
      double testingScore = folds[0]->getValidationObjective(0)->evaluate(context, model);
      size_t nbLeaves = model.staticCast<HoeffdingTreeNode>()->getNbOfLeaves();
      context.resultCallback(algoNames[s] + " RRE", testingScore);
      context.resultCallback(algoNames[s] + " TreeSize", nbLeaves);
      context.leaveScope(testingScore);
    }
    context.progressCallback(new ProgressionState(solvers.size(), solvers.size(), "Solvers"));
  }

};

class DiscoveryScience : public WorkUnit
{
public:
  DiscoveryScience() : randomSeed(456), numSamples(1000), chunkSize(100), verbosity(2), datasetPath("") {}
  
  virtual ObjectPtr run(ExecutionContext& context)
  {
    lbCppMLLibraryCacheTypes(context);
    
    context.getRandomGenerator()->setSeed(randomSeed);
    

    // set up test problems
    std::vector<ProblemPtr> problems;
    std::vector<string> problemnames;
    problems.push_back(new FriedmannProblem());
    problems.push_back(new LEXPProblem());
    problems.push_back(new LOSCProblem());
    problemnames.push_back("Friedmann");
    problemnames.push_back("Lexp");
    problemnames.push_back("Losc");

    SamplerPtr sampler = uniformSampler();
    std::vector< std::vector<ProblemPtr> > xValProblems;
    for (size_t i = 0; i < problems.size(); ++i)
    {
      sampler->initialize(context, problems[i]->getDomain());
      xValProblems.push_back(problems[i]->generateFolds(context, 10, numSamples / 10, sampler));
    }
    
    std::vector<string> datasets;
    datasets.push_back("winequality-white.arff");
    datasets.push_back("cal_housing.arff");
    //datasets.push_back("CASP.arff");
    std::vector<string> datasetnames;
    problemnames.push_back("Wine quality");
    problemnames.push_back("California housing");
    //problemnames.push_back("Physicochemical Properties of Protein Tertiary Structure");
    
    ArffLoader loader;
    for (size_t i = 0; i < datasets.size(); ++i)
    {
      TablePtr table = loader.loadFromFile(context, juce::File(datasetPath + "/" + datasets[i])).staticCast<Table>();
      xValProblems.push_back(Problem::generateFoldsFromTable(context, table, 10));
    }
    
    CompositeWorkUnitPtr subWorkUnits = new CompositeWorkUnit("Discovery Science", xValProblems.size());
    for (size_t p = 0; p < xValProblems.size(); ++p)
      subWorkUnits->setWorkUnit(p, new XValWorkUnit(xValProblems[p], chunkSize, (SolverVerbosity) verbosity, problemnames[p]));
    subWorkUnits->setPushChildrenIntoStackFlag(false);
    context.run(subWorkUnits);
    
    return new Boolean(true);
  }
  
protected:
  friend class DiscoveryScienceClass;
  
  size_t numSamples;
  size_t randomSeed;
  size_t chunkSize;
  double learningRate;
  double learningRateDecay;
  double delta;
  double threshold;
  size_t numDims;
  size_t verbosity;

  string datasetPath;

private:

  

    void makeMatlabSurface(ExecutionContext& context, ProblemPtr problem, ExpressionPtr expr)
  {
    if (problem->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() <= 1) return;
    std::ofstream file, file2, file3;
    file.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\surface.dat");
    file2.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\realsurface.dat");
    file3.open("C:\\Users\\xavier\\Documents\\MATLAB\\FunctionData\\error.dat");
    std::vector<ObjectPtr> point(2);
    DenseDoubleVectorPtr vpoint = new DenseDoubleVector(2, 0.0);
    for (size_t i = 0; i < 100; ++i)
    {
      point[1] = new Double(i / 100.0);
      vpoint->setValue(1, i / 100.0);
      for (size_t j = 0; j < 100; ++j)
      {
        point[0] = new Double(j / 100.0);
        vpoint->setValue(0, j / 100.0);
        ObjectPtr result = expr->compute(context, point);
        FitnessPtr realval = problem->evaluate(context, vpoint);
        file << Double::get(result) << " ";
        file2 << realval->getValue(0) << " ";
        file3 << abs(Double::get(result) - realval->getValue(0)) << " ";
      }
      file << std::endl;
      file2 << std::endl;
      file3 << std::endl;
    }
    file.close();
    file2.close();
    file3.close();
  }

  void makeCurve(ExecutionContext& context, ProblemPtr baseProblem, ExpressionPtr expression)
  {
      //context.enterScope("Curve");
    if (baseProblem->getDomain().staticCast<ScalarVectorDomain>()->getNumDimensions() > 1) return;
    double x = 0.0;
    size_t curveSize = 200;
    std::vector<ObjectPtr> input = std::vector<ObjectPtr>(1);
    input[0] = new Double(0.0);
    DenseDoubleVectorPtr problemInput = new DenseDoubleVector(1, 0.0);
    ScalarVectorDomainPtr domain = baseProblem->getDomain().staticCast<ScalarVectorDomain>();
    double range = domain->getUpperLimit(0) - domain->getLowerLimit(0);
    double offset = domain->getLowerLimit(0);
    for (size_t i = 0; i < curveSize; ++i)
    {
      x = offset + range * i / curveSize;
      context.enterScope(string(x));
      context.resultCallback("x", x);
      input[0].staticCast<Double>()->set(x);
      problemInput->setValue(0, x);
      context.resultCallback("supervision", baseProblem->evaluate(context, problemInput)->getValue(0));
      context.resultCallback("prediction", expression->compute(context, input));
      context.leaveScope();
    }
    //context.leaveScope();
  }
};

}; /* namespace lbcpp */

#endif // !MOO_DISCOVERY_SCIENCE_H_
