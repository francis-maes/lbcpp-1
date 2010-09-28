/*-----------------------------------------.---------------------------------.
| Filename: JobThreadInferenceContext.h    | Inference Context for an        |
| Author  : Francis Maes                   |   Inference job                 |
| Started : 20/09/2010 19:27               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
# define LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_

# include <lbcpp/Inference/Inference.h>
# include <lbcpp/Inference/InferenceStack.h>

namespace lbcpp
{

class JobThreadInferenceContext : public InferenceContext
{
public:
  JobThreadInferenceContext(InferenceContextPtr parentContext, Thread* thread, ThreadPoolPtr pool, InferenceStackPtr stack)
    : parentContext(parentContext), thread(thread), pool(pool), stack(stack ? stack->cloneAndCast<InferenceStack>() : InferenceStackPtr(new InferenceStack()))
    {}
  JobThreadInferenceContext() : thread(NULL) {}

  virtual void callPreInference(InferenceStackPtr stack, Variable& input, Variable& supervision, Variable& output, ReturnCode& returnCode)
    {parentContext->callPreInference(stack, input, supervision, output, returnCode);}

  virtual void callPostInference(InferenceStackPtr stack, const Variable& input, const Variable& supervision, Variable& output, ReturnCode& returnCode)
    {parentContext->callPostInference(stack, input, supervision, output, returnCode);}

  virtual InferenceStackPtr getCurrentStack() const
    {return stack;}

  virtual Variable run(InferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    if (thread->threadShouldExit())
    {
      returnCode = Inference::canceledReturnCode;
      return Variable();
    }
    return InferenceContext::run(inference, input, supervision, returnCode);
  }

  virtual Variable runParallelInference(ParallelInferencePtr inference, const Variable& input, const Variable& supervision, ReturnCode& returnCode)
  {
    ParallelInferenceStatePtr state = inference->prepareInference(InferenceContextPtr(this), input, supervision, returnCode);
    if (returnCode != Inference::finishedReturnCode)
      return Variable();
    
    size_t n = state->getNumSubInferences();
    size_t step = n / pool->getNumCpus();
    if (!step)
      step = 1;

    //juce::DBG("Run Parallel Inference: " + inference->toString() + T(" num inferences: ") + String((int)n) + T(" step = ") + String((int)step));
    
    for (size_t begin = 0; begin < n; )
    {
      size_t end = begin + step;
      if (end > n)
        end = n;
      ThreadPoolJob* job = new RunParallelInferencesJob(parentContext, pool, stack, inference, state, begin, end, thread);
      pool->addJob(job, stack->getDepth());
      begin = end;
    }
    pool->waitThread(thread);
    //juce::DBG("OK Run Parallel Inference: " + inference->toString());
    return inference->finalizeInference(InferenceContextPtr(this), state, returnCode);
  }

protected:
  friend class JobThreadInferenceContextClass;

  InferenceContextPtr parentContext;
  Thread* thread;
  ThreadPoolPtr pool;
  InferenceStackPtr stack;
};

}; /* namespace lbcpp */

#endif // !LBCPP_INFERENCE_CONTEXT_JOB_THREAD_H_
