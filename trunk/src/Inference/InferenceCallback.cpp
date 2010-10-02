/*-----------------------------------------.---------------------------------.
| Filename: InferenceCallback.cpp          | Inference Callback              |
| Author  : Francis Maes                   |                                 |
| Started : 05/05/2010 12:42               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include <lbcpp/Inference/InferenceOnlineLearner.h>
#include <lbcpp/Inference/InferenceResultCache.h>
using namespace lbcpp;

/*
** InferenceStepResultCache
*/
ObjectPtr InferenceStepResultCache::get(ObjectPtr input) const
{
  InputOutputMap::const_iterator it = cache.find(input->getName());
  return it == cache.end() ? ObjectPtr() : it->second;
}

/*
** InferenceResultCache
*/
InferenceStepResultCachePtr InferenceResultCache::getCacheForInferenceStep(InferencePtr step) const
{
  CacheMap::const_iterator it = cache.find(step);
  return it == cache.end() ? InferenceStepResultCachePtr() : it->second;
}

ObjectPtr InferenceResultCache::get(InferencePtr step, ObjectPtr input) const
{
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(step);
  ObjectPtr res = stepCache ? stepCache->get(input) : ObjectPtr();
  //if (res)
  //  std::cout << "Use: " << step->getName() << " input: " << input->getName() << std::endl;
  return res;
}

void InferenceResultCache::addStepCache(InferenceStepResultCachePtr stepCache)
{
  cache[stepCache->getInference()] = stepCache;
}

void InferenceResultCache::add(InferencePtr inference, ObjectPtr input, ObjectPtr output)
{
  //std::cout << "Add: " << step->getName() << " input: " << input->getName() << " output: " << output->toString() << std::endl;
  InferenceStepResultCachePtr stepCache = getCacheForInferenceStep(inference);
  if (!stepCache)
    stepCache = cache[inference] = new InferenceStepResultCache(inference);
  stepCache->add(input, output);
}
