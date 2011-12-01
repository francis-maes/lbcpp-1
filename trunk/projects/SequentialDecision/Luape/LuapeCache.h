/*-----------------------------------------.---------------------------------.
| Filename: LuapeCache.h                   | Luape Cache                     |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_CACHE_H_
# define LBCPP_LUAPE_CACHE_H_

namespace lbcpp
{

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;
class LuapeInputNode;
typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeInstanceCache : public Object
{
public:
  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, const ObjectPtr& object);
  void set(const LuapeNodePtr& node, const Variable& value);
  Variable compute(ExecutionContext& context, const LuapeNodePtr& node);

protected:
  typedef std::map<LuapeNodePtr, Variable> NodeToValueMap;
  NodeToValueMap m;
};

typedef ReferenceCountedObjectPtr<LuapeInstanceCache> LuapeInstanceCachePtr;

class LuapeSamplesCache : public Object
{
public:
  LuapeSamplesCache(const std::vector<LuapeInputNodePtr>& inputs, size_t size, size_t maxCacheSize = 10000);
  LuapeSamplesCache() {}

  void set(const LuapeNodePtr& node, const VectorPtr& samples);
  void setInputObject(const std::vector<LuapeInputNodePtr>& inputs, size_t index, const ObjectPtr& object);
  VectorPtr get(const LuapeNodePtr& node) const;

  VectorPtr compute(ExecutionContext& context, const LuapeNodePtr& node, SparseDoubleVectorPtr* sortedDoubleValues = NULL, bool isRemoveable = true);

  size_t getNumberOfCachedNodes() const
    {return m.size();}

  size_t getNumSamples() const
    {return inputCaches.size() ? inputCaches[0]->getNumElements() : 0;}

protected:
  // node -> (samples, sorted double values)
  typedef std::map<LuapeNodePtr, std::pair<VectorPtr, SparseDoubleVectorPtr> > NodeToSamplesMap;

  NodeToSamplesMap m;
  std::vector<VectorPtr> inputCaches;
  std::deque<LuapeNodePtr> cacheSequence;
  size_t maxCacheSize;

  void ensureSortedDoubleValuesAreComputed(const VectorPtr& samples, SparseDoubleVectorPtr& sortedDoubleValues);
};

typedef ReferenceCountedObjectPtr<LuapeSamplesCache> LuapeSamplesCachePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_CACHE_H_