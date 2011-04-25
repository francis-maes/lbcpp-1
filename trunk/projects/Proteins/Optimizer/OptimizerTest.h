/*-----------------------------------------.---------------------------------.
| Filename: OptimizerTest.h                | WorkUnit used to test Optimizer |
| Author  : Arnaud Schoofs                 | (debug purpose)                 |
| Started : 02/03/2011                     |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/


#ifndef OPTIMIZER_TEST_H_
#define OPTIMIZER_TEST_H_

# include <lbcpp/Execution/WorkUnit.h>
# include <lbcpp/Optimizer/GridOptimizer.h>
# include "../Optimizer/ProteinGridEvoOptimizer.h"
# include "../../../src/Optimizer/Optimizer/Grid/GridEvoOptimizer.h"
# include <lbcpp/Distribution/MultiVariateDistribution.h>
# include "../Predictor/ProteinPredictorParameters.h"
# include "../../../src/Distribution/Builder/GaussianDistributionBuilder.h"
# include "../../../src/Distribution/Builder/BernoulliDistributionBuilder.h"

// TODO arnaud : just to compile header file
# include <lbcpp/Optimizer/OptimizerContext.h>
# include "../WorkUnit/DebugWorkUnit.h"

# include "../../../src/Optimizer/Optimizer/UniformSampleAndPickBestOptimizer.h"
# include "../../../src/Optimizer/Optimizer/EDAOptimizer.h"
# include "../../../src/Optimizer/Optimizer/AsyncEDAOptimizer.h"
# include "../../../src/Optimizer/Context/SynchroneousOptimizerContext.h"
# include "../../../src/Optimizer/Context/MultiThreadsOptimizerContext.h"
# include <lbcpp/Function/ScalarFunction.h>

namespace lbcpp
{

class OptimizerTest : public WorkUnit 
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    
    // TESTS OPTIMIZER
    
    OptimizerPtr optimizer = new UniformSampleAndPickBestOptimizer(1000);
    //OptimizerPtr optimizer = new EDAOptimizer(20, 500, 100);
    //OptimizerPtr optimizer = new AsyncEDAOptimizer(10000, 1000, 2, 1, 1, 500);
    //OptimizerContextPtr optimizerContext = new SynchroneousOptimizerContext(squareFunction());
    OptimizerContextPtr optimizerContext = new MultiThreadsOptimizerContext(squareFunction());
    OptimizerStatePtr optimizerState = new OptimizerState();
    optimizerState->setDistribution(context, new UniformDistribution(-5,5));
    //optimizerState->setDistribution(context, new GaussianDistribution(10, 10000));
    return optimizer->compute(context, optimizerContext, optimizerState);
    
    
    return Variable();
    
    /*
    // initial distribution
    IndependentMultiVariateDistributionPtr distributions = new IndependentMultiVariateDistribution(numericalProteinFeaturesParametersClass);      
    distributions->setSubDistribution(0, new PositiveIntegerGaussianDistribution(1,9));
    distributions->setSubDistribution(1, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(2, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(3, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(4, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(5, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(6, new PositiveIntegerGaussianDistribution(2,9));
    distributions->setSubDistribution(7, new PositiveIntegerGaussianDistribution(3,9));
    distributions->setSubDistribution(8, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(9, new PositiveIntegerGaussianDistribution(5,9));
    distributions->setSubDistribution(10, new BernoulliDistribution(0.5));
    distributions->setSubDistribution(11, new PositiveIntegerGaussianDistribution(15,4));
    distributions->setSubDistribution(12, new PositiveIntegerGaussianDistribution(15,100));
    distributions->setSubDistribution(13, new PositiveIntegerGaussianDistribution(50,225));
    */
    // Create initial state either from distri or from existing file
    //ProteinGridEvoOptimizerStatePtr state = /*new ProteinGridEvoOptimizerState(distributions);*/ Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(T("EvoOptimizerState.xml"))).staticCast<ProteinGridEvoOptimizerState>();
    /*
    // variables used by EvoOptimizer
    size_t totalNumberWuRequested = 10000; // total number of WUs to evaluate
    size_t numberWuToUpdate = 300;       // min number of WUs evaluated needed to update distribution
    size_t numberWuInProgress = 500;      // number of WUs in progress (either in thread pool or in Boinc Network), should be <= totalNumberWuRequested. (for local optimizer should be a little bit higher than the number of CPUs)
    size_t ratioUsedForUpdate = 3;      // number of WUs used to calculate new distribution is numberWuToUpdate/ratioUsedForUpdate 
    size_t timeToSleep = 5*60;            // time to sleep between work generation and attemps to use finished results (avoid busy waiting)
    size_t updateFactor = 1;            // preponderance of new distri vs old distri (low value avoid too quick convergence)
    
    // variables used by GridOptimizer
    String projectName(T("BoincProteinLearner1"));
    String source(T("arnaud@monster24"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    size_t managerPort = 1664;
    size_t requiredMemory = 1;
    size_t requiredTime = 1;
    
    */
    /**
     * EvoOptimizer is the Local Evo Optimizer.
     * To use it you need:
     * - implement a GridEvoOptimizerState, i.e. WorkUnitPtr generateSampleWU(ExecutionContext& context). The state should be serializable !
     * - implement a ...GetVariableFromTraceFunction, i.e function : trace -> variable (to optimize) used for this evaluation
     * - implement a ...GetScoreFromTraceFunction, i.e function : trace -> score to optimize (== 1 - getScoreToMinize())
     *   NB : this function won't be necessary in the future as it is the same for every kind of trace !
     * 
     * Example : see ProteinGridEvoOptimizer.h
     */
    //EvoOptimizerPtr optimizer = new EvoOptimizer(totalNumberWuRequested, numberWuToUpdate, numberWuInProgress, ratioUsedForUpdate, timeToSleep, updateFactor);
    //return optimizer->optimize(context, state, new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
    
    
    /**
     * GridEvoOptimizer
     */
    //GridEvoOptimizerPtr optimizer = new GridEvoOptimizer(totalNumberWuRequested, numberWuToUpdate, numberWuInProgress, ratioUsedForUpdate, projectName, source, destination,
    //                                                     managerHostName, managerPort, requiredMemory, requiredTime, timeToSleep, updateFactor);
    //return optimizer->optimize(context, state, new ProteinGetVariableFromTraceFunction(), new ProteinGetScoreFromTraceFunction());
    
    
    
    /**
     * Tests
     */
    /*
    ExecutionTracePtr trace = Object::createFromFile(context, File(T("/Users/arnaudschoofs/Proteins/traces/1299675529047.trace"))).staticCast<ExecutionTrace>();
    FunctionPtr f1 = new ProteinGetScoreFromTraceFunction();
    std::cout << f1->compute(context, trace).toString() << std::endl;
    
    FunctionPtr f2 = new ProteinGetVariableFromTraceFunction();
    std::cout << f2->compute(context, trace).toString() << std::endl;
    */
    
    
  }
protected:
  friend class OptimizerTestClass;
};

  
};/* namespace lbcpp */

#endif // !OPTIMIZER_TEST_H_