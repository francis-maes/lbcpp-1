/*-----------------------------------------.---------------------------------.
| Filename: UniformSampleAndPickBestOpti..h| Uniform sample and pick best    |
| Author  : Francis Maes, Arnaud Schoofs   | optimizer                       |
| Started : 21/12/2010 23:43               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_
# define LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_

# include <lbcpp/Optimizer/Optimizer.h>

namespace lbcpp
{
#if 0
class UniformSampleAndPickBestOptimizer : public Optimizer
{
public:
  UniformSampleAndPickBestOptimizer(size_t numSamples = 0, bool verbose = false)
    : numSamples(numSamples), verbose(verbose) {}
  
  virtual OptimizerStatePtr optimize(ExecutionContext& context, const OptimizerContextPtr& optimizerContext, const OptimizerStatePtr& optimizerState) const
  { 
    
 // TODO : use SamplerBasedOptimizerState ?
    DistributionBasedOptimizerStatePtr state = optimizerState.dynamicCast<DistributionBasedOptimizerState>();
    jassert(state);
    
    std::vector<double> values;
    ContinuousDistributionPtr apriori = state->getDistribution().dynamicCast<ContinuousDistribution>();
    apriori->sampleUniformly(numSamples, values);
    
    for (size_t i = 0; i < numSamples; ++i) 
    {
      if (!optimizerContext->evaluate(values[i]))
        i--;
      else 
      {
        state->incTotalNumberOfRequests();
        context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), numSamples, T("Evaluations")));
      }
    }
    
    // wait (in case of async context) & update progression
    while (!optimizerContext->areAllRequestsProcessed()) {
      Thread::sleep(100);
      context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), numSamples, T("Evaluations")));
    }
    jassert(state->getNumberOfProcessedRequests() == numSamples);
    context.progressCallback(new ProgressionState(state->getNumberOfProcessedRequests(), numSamples, T("Evaluations"))); // needed to be sure to have 100% in Explorer
    
    std::vector< std::pair<double, Variable> >::const_iterator it;
    {
      ScopedLock _(state->getLock());
      size_t i = 1;
      for (it = state->getProcessedRequests().begin(); it < state->getProcessedRequests().end(); it++)
      {
        state->submitSolution(it->second, it->first);
        if (verbose) 
        {
          context.enterScope(T("Request ") + String((int) i));
          context.resultCallback(T("requestNumber"), i);
          context.resultCallback(T("parameter"), it->second);      
          context.leaveScope(it->first);
        }
        i++;  // outside if to avoid a warning for unused variable
      }
      state->flushProcessedRequests();
    }
    return state->getBestScore();

    return Variable();

 }
  
protected:
  friend class UniformSampleAndPickBestOptimizerClass;
  size_t numSamples;
  bool verbose;
};

typedef ReferenceCountedObjectPtr<UniformSampleAndPickBestOptimizer> UniformSampleAndPickBestOptimizerPtr;  
#endif
}; /* namespace lbcpp */

#endif // !LBCPP_OPTIMIZER_UNIFORM_SAMPLE_AND_PICK_BEST_H_
