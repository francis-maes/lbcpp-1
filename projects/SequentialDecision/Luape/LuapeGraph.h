/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.h                   | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 25/10/2011 18:49               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_H_
# define LBCPP_LUAPE_GRAPH_H_

namespace lbcpp
{

class LuapeGraph;
typedef ReferenceCountedObjectPtr<LuapeGraph> LuapeGraphPtr;

class LuapeGraphCallback
{
public:
  virtual ~LuapeGraphCallback() {}

  virtual void valueYielded(const Variable& value) {}
};

typedef LuapeGraphCallback* LuapeGraphCallbackPtr;

//// CACHE

class LuapeNodeCache : public Object
{
public:
  LuapeNodeCache();

  void initialize(TypePtr type);
  void clear();
  
  /*
  ** Examples
  */
  void resizeSamples(bool isTrainingSamples, size_t size);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const Variable& value);

  size_t getNumTrainingSamples() const
    {return trainingSamples->getNumElements();}

  size_t getNumValidationSamples() const
    {return validationSamples->getNumElements();}

  size_t getNumSamples(bool isTrainingSamples) const
    {return (isTrainingSamples ? trainingSamples : validationSamples)->getNumElements();}

  Variable getTrainingSample(size_t index) const
    {return trainingSamples->getElement(index);}

  Variable getSample(bool isTrainingSample, size_t index) const
    {return (isTrainingSample ? trainingSamples : validationSamples)->getElement(index);}

  VectorPtr getSamples(bool isTrainingSample) const
    {return isTrainingSample ? trainingSamples : validationSamples;}

  const VectorPtr& getTrainingSamples() const
    {return trainingSamples;}

  const VectorPtr& getValidationSamples() const
    {return validationSamples;}

  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  /*
  ** Double values
  */
  bool isConvertibleToDouble() const
    {return convertibleToDouble;}

  const std::vector< std::pair<size_t, double> >& getSortedDoubleValues() const;

  /*
  ** Score
  */
  bool isScoreComputed() const
    {return scoreComputed;}

  double getScore() const
    {return score;}

  void setScore(double score)
    {this->score = score; scoreComputed = true;}

  void clearScore()
    {scoreComputed = false; score = 0.0;}

protected:
  VectorPtr trainingSamples;
  VectorPtr validationSamples;

  bool convertibleToDouble;
  std::vector< std::pair<size_t, double> > sortedDoubleValues; // only if isConvertibleToDouble

  bool scoreComputed;
  double score; // only for 'yield' nodes
};

typedef ReferenceCountedObjectPtr<LuapeNodeCache> LuapeNodeCachePtr;

typedef std::vector<juce::int64> LuapeNodeKey;

class LuapeGraphCache : public Object
{
public:
  LuapeNodeCachePtr getNodeCache(const LuapeNodeKey& key, const TypePtr& nodeType);
  
  size_t getNumCachedNodes() const
    {ScopedLock _(lock); return m.size();}

  void clearScores();

protected:
  typedef std::map<LuapeNodeKey, LuapeNodeCachePtr> CacheMap;

  CriticalSection lock;
  CacheMap m; 
};
typedef ReferenceCountedObjectPtr<LuapeGraphCache> LuapeGraphCachePtr;


//////  GRAPH NODES

class LuapeNode;
typedef ReferenceCountedObjectPtr<LuapeNode> LuapeNodePtr;

class LuapeNode : public NameableObject
{
public:
  LuapeNode(const TypePtr& type, const String& name);
  LuapeNode() {}

  const TypePtr& getType() const
    {return type;}

  virtual bool initialize(ExecutionContext& context, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const = 0;
  virtual void fillKey(LuapeNodeKey& res) const = 0;
  virtual size_t getDepth() const = 0;

  virtual void updateCache(ExecutionContext& context, bool isTrainingSamples)
    {}

  VariableSignaturePtr getSignature() const
    {return new VariableSignature(type, name);}

  const LuapeNodeCachePtr& getCache() const
    {return cache;}

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  friend class LuapeNodeClass;

  TypePtr type;
  LuapeNodeCachePtr cache;
};

extern ClassPtr luapeNodeClass;

class LuapeInputNode : public LuapeNode
{
public:
  LuapeInputNode(const TypePtr& type, const String& name, size_t inputIndex);
  LuapeInputNode() {}

  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(LuapeNodeKey& res) const;
  virtual size_t getDepth() const
    {return 0;}

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const;

protected:
  size_t inputIndex;
};

typedef ReferenceCountedObjectPtr<LuapeInputNode> LuapeInputNodePtr;

class LuapeFunctionNode;
typedef ReferenceCountedObjectPtr<LuapeFunctionNode> LuapeFunctionNodePtr;

class LuapeFunctionNode : public LuapeNode
{
public:
  LuapeFunctionNode(const FunctionPtr& function, const std::vector<LuapeNodePtr>& arguments);
  LuapeFunctionNode(const FunctionPtr& function, LuapeNodePtr argument);
  LuapeFunctionNode() {}

  virtual bool initialize(ExecutionContext& context, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(LuapeNodeKey& res) const;
  virtual size_t getDepth() const;
  virtual void updateCache(ExecutionContext& context, bool isTrainingSamples);

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

protected:
  friend class LuapeFunctionNodeClass;

  FunctionPtr function;
  std::vector<LuapeNodePtr> arguments;
};

class LuapeYieldNode : public LuapeNode
{
public:
  LuapeYieldNode(const LuapeNodePtr& argument = LuapeNodePtr());

  virtual bool initialize(ExecutionContext& context, const LuapeGraphCachePtr& cache);
  virtual Variable compute(ExecutionContext& context, const std::vector<Variable>& state, LuapeGraphCallbackPtr callback) const;
  virtual void fillKey(LuapeNodeKey& res) const;
  virtual size_t getDepth() const;

  virtual String toShortString() const;
  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeNodePtr& getArgument() const
    {return argument;}

protected:
  friend class LuapeYieldNodeClass;

  LuapeNodePtr argument;
};

typedef ReferenceCountedObjectPtr<LuapeYieldNode> LuapeYieldNodePtr;
extern ClassPtr luapeYieldNodeClass;

//////  GRAPH

class LuapeGraph : public Object
{
public:
  LuapeGraph(bool useCache = false);

  size_t getNumNodes() const
    {return nodes.size();}

  LuapeNodePtr getNode(size_t index) const
    {jassert(index < nodes.size()); return nodes[index];}

  const std::vector<LuapeNodePtr>& getNodes() const
    {return nodes;}

  TypePtr getNodeType(size_t index) const
    {return getNode(index)->getType();}

  LuapeNodeKey getNodeKey(size_t nodeIndex) const
    {jassert(nodeIndex < nodes.size()); LuapeNodeKey key; nodes[nodeIndex]->fillKey(key); return key;}

  LuapeNodePtr getLastNode() const
    {return nodes.back();}

  LuapeNodePtr pushNode(ExecutionContext& context, const LuapeNodePtr& node);
  LuapeNodePtr pushNodes(ExecutionContext& context, const LuapeNodeKey& key, size_t& keyPosition, size_t keyEnd);
  void popNode();

  size_t getNumTrainingSamples() const;
  size_t getNumValidationSamples() const;
  size_t getNumSamples(bool isTrainingSamples) const;
  void resizeSamples(bool isTrainingSamples, size_t numSamples);
  void resizeSamples(size_t numTrainingSamples, size_t numValidationSamples);
  void setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example);
  void setSample(bool isTrainingSample, size_t index, const ObjectPtr& example);
  void clearSamples(bool clearTrainingSamples = true, bool clearValidationSamples = true);

  void compute(ExecutionContext& context, std::vector<Variable>& state, size_t firstNodeIndex = 0, LuapeGraphCallbackPtr callback = 0) const;

  virtual String toShortString() const;
  String graphToString(size_t firstNodeIndex = 0) const;

  virtual void clone(ExecutionContext& context, const ObjectPtr& t) const;

  const LuapeGraphCachePtr& getCache() const
    {return cache;}

  void setCache(LuapeGraphCachePtr cache)
    {this->cache = cache;}

  void clearScores();

protected:
  friend class LuapeGraphClass;

  typedef std::map<LuapeNodeKey, size_t> NodeKeyToIndexMap;

  std::vector<LuapeNodePtr> nodes;
  NodeKeyToIndexMap nodeKeyToIndex;
  size_t numExamples;
  LuapeGraphCachePtr cache;
};

extern ClassPtr luapeGraphClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_H_
