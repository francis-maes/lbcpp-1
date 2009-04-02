/*-----------------------------------------.---------------------------------.
| Filename: LabeledContentGraph.cpp        | A labeled graph of content      |
| Author  : Francis Maes                   |  elements                       |
| Started : 23/03/2009 18:55               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "GeneratedCode/LabeledContentGraph.lh"
using namespace cralgo;

/*
class InstanceSet;
typedef ReferenceCountedObjectPtr<InstanceSet> InstanceSetPtr;
class VectorInstanceSet;
typedef ReferenceCountedObjectPtr<VectorInstanceSet> VectorInstanceSetPtr;

class InstanceSet : public Object
{
public:
  virtual size_t getNumInstances() const = 0;
  virtual ObjectPtr getInstance(size_t i) const = 0;
  
  template<class T>
  ReferenceCountedObjectPtr<T> getInstanceCast(size_t i) const
    {return getInstance(i).dynamicCast<T>();}
};

class VectorInstanceSet : public InstanceSet
{
public:
  virtual size_t getNumInstances() const 
    {return instances.size();}
    
  virtual ObjectPtr getInstance(size_t i) const 
    {return instances[i];}

  void append(ObjectPtr instance)
    {instances.push_back(instance);}
    
protected:
  std::vector<ObjectPtr> instances;
  size_t index;
  
  ObjectPtr getCurrentInstance() const
    {return index < instances.size() ? instances[index] : ObjectPtr();}
};
*/

LabeledContentGraph::~LabeledContentGraph()
{
  //  std::cout << "delete labeled content graph: " << this << std::endl;
}

class ContentFileParser : public TextFileParser
{
public:
  ContentFileParser(LabeledContentGraphPtr res, FeatureDictionaryPtr features, std::map<std::string, size_t>& nodeIdentifiers, size_t maxNodes)
    : res(res), features(features), nodeIdentifiers(nodeIdentifiers), maxNodes(maxNodes) {}
    
  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    if (columns.size() < 2)
    {
      Object::error("ContentFileParser::parseDataLine", "Invalid number of columns");
      return false;
    }
    SparseVectorPtr content = new SparseVector(features);
    for (size_t i = 1; i < columns.size() - 1; ++i)
    {
      size_t index = i - 1;
      if (features->getNumFeatures() <= index)
        features->addFeature("word " + cralgo::toString(index));
      
      std::string featureName;
      double featureValue;
      size_t semiColonPos = columns[i].find(':');
      if (semiColonPos != std::string::npos)
      {
        featureName = columns[i].substr(0, semiColonPos);
        if (!parse(columns[i].substr(semiColonPos + 1), featureValue))
          return false;
      }
      else
      {
        featureName = cralgo::toString(index);
        if (!parse(columns[i], featureValue))
          return false;
      }
      if (featureValue != 0.0)
        content->set(features->addFeature(featureName), featureValue);
    }
    content->set(features->addFeature("__unit__"), 1.0); // unit feature
    nodeIdentifiers[columns[0]] = res->addNode(content, res->getLabelDictionary()->add(columns.back()));
    if (maxNodes && res->getNumNodes() >= maxNodes)
      breakParsing();
    return true;
  }

private:
  LabeledContentGraphPtr res;
  FeatureDictionaryPtr features;
  std::map<std::string, size_t>& nodeIdentifiers;
  size_t maxNodes;
};

class LinkFileParser : public TextFileParser
{
public:
  LinkFileParser(LabeledContentGraphPtr res, std::map<std::string, size_t>& nodeIdentifiers, size_t maxNodes)
    : res(res), nodeIdentifiers(nodeIdentifiers), maxNodes(maxNodes) {}

  virtual bool parseDataLine(const std::vector<std::string>& columns)
  {
    if (columns.size() != 2)
    {
      Object::error("LinkFileParser::parseDataLine", "Invalid number of columns");
      return false;
    }
    std::map<std::string, size_t>::iterator it1, it2;
    it1 = nodeIdentifiers.find(columns[1]);
    it2 = nodeIdentifiers.find(columns[0]);
    if (it1 == nodeIdentifiers.end())
      invalidIdentifiers.insert(columns[1]);
    if (it2 == nodeIdentifiers.end())
      invalidIdentifiers.insert(columns[0]);
    if (it1 != nodeIdentifiers.end() && it2 != nodeIdentifiers.end())
      res->addLink(it1->second, it2->second);
    return true;
  }
  
  virtual bool parseEnd()
  {
    if (invalidIdentifiers.size())
      Object::warning("LinkFileParser::parseEnd", cralgo::toString(invalidIdentifiers.size()) + " invalid identifiers in link file");
    invalidIdentifiers.clear();
    return true;
  }
  
private:
  LabeledContentGraphPtr res;
  std::map<std::string, size_t>& nodeIdentifiers;
  std::set<std::string> invalidIdentifiers;
  size_t maxNodes;
};

LabeledContentGraphPtr LabeledContentGraph::parseGetoorGraph(const std::string& contentFile, const std::string& linkFile, FeatureDictionaryPtr features, StringDictionaryPtr labels, size_t maxNodes)
{
  LabeledContentGraphPtr res = new LabeledContentGraph(labels);
  std::map<std::string, size_t> nodeIdentifiers;
  ContentFileParser contentParser(res, features, nodeIdentifiers, maxNodes);
  LinkFileParser linkParser(res, nodeIdentifiers, maxNodes);
  if (!contentParser.parseFile(contentFile) || !linkParser.parseFile(linkFile))
    return LabeledContentGraphPtr();
  return res;
}

LabeledContentGraphPtr LabeledContentGraph::randomizeOrder() const
{
  std::vector<size_t> order;
  Random::getInstance().sampleOrder(getNumNodes(), order);
  std::vector<size_t> invOrder(order.size());
  for (size_t i = 0; i < order.size(); ++i)
    invOrder[order[i]] = i;
  
  ContentGraphPtr newGraph = new ContentGraph();
  newGraph->reserveNodes(order.size());
  LabelSequencePtr newLabels = new LabelSequence(getLabelDictionary(), order.size());
  
  // order[i] -> i
  // i -> invOrder[i]
  for (size_t i = 0; i < order.size(); ++i)
  {
    newGraph->addNode(graph->getNode(order[i]));
    newLabels->set(i, labels->get(order[i]));
  }
  for (size_t i = 0; i < order.size(); ++i)
  {
    size_t idx = order[i];
    size_t n = graph->getNumSuccessors(idx);
    for (size_t j = 0; j < n; ++j)
      newGraph->addLink(i, invOrder[graph->getSuccessor(idx, j)]);
  }
  return new LabeledContentGraph(newGraph, newLabels);
}

void LabeledContentGraph::makeFolds(size_t numFolds, bool removeTrainTestLinks, std::vector<LabeledContentGraphPtr>& trainGraphs, std::vector<LabelsFold>& testGraphs)
{
  size_t numNodes = getNumNodes();
  assert(numFolds > 1 && numNodes);
  trainGraphs.reserve(trainGraphs.size() + numFolds);
  testGraphs.reserve(testGraphs.size() + numFolds);
  double foldMeanSize = numNodes / (double)numFolds;

  for (size_t i = 0; i < numFolds; ++i)
  {
    //    std::cout << "makeFolds fold " << i << std::endl;
    size_t foldBegin = (size_t)(i * foldMeanSize);
    size_t foldEnd = (size_t)((i + 1) * foldMeanSize);
    std::pair<LabeledContentGraphPtr, LabelsFold> graphs = makeFold(foldBegin, foldEnd, removeTrainTestLinks);
    trainGraphs.push_back(graphs.first);
    testGraphs.push_back(graphs.second);
  }
}

std::pair<LabeledContentGraphPtr, LabeledContentGraph::LabelsFold> LabeledContentGraph::makeFold(size_t testBegin, size_t testEnd, bool removeTrainTestLinks)
{
  size_t numNodes = getNumNodes();
  assert(numNodes && testBegin < testEnd && testEnd <= numNodes);    
  std::pair<LabeledContentGraphPtr, LabelsFold> res;

  /*
  ** Train graph
  */
  LabeledContentGraphPtr trainGraph = new LabeledContentGraph(getLabelDictionary()); // all but the current fold
  
  // create train nodes
  for (size_t j = 0; j < numNodes; ++j)
    if (j < testBegin || j >= testEnd)
      trainGraph->addNode(getNode(j), getLabel(j));
  
  // create train links
  for (size_t j = 0; j < numNodes; ++j)
    if (j < testBegin || j >= testEnd)
    {
      for (size_t k = 0; k < getNumSuccessors(j); ++k)
      {
        size_t succ = getSuccessor(j, k);
        if (succ < testBegin || succ >= testEnd)
        {
          size_t mappedSourceIndex = j < testBegin ? j : j - (testEnd - testBegin);
          size_t mappedSuccIndex = succ < testBegin ? succ : succ - (testEnd - testBegin);
          trainGraph->addLink(mappedSourceIndex, mappedSuccIndex);
        }
      }
    }
  res.first = trainGraph;
  
  /*
  ** Test graph
  */
  LabelsFold testGraph;
  if (removeTrainTestLinks)
  {
    testGraph.graph = new LabeledContentGraph(getLabelDictionary()); // current fold
    testGraph.foldBegin = 0;
    testGraph.foldEnd = testEnd - testBegin;
    
    // create test nodes
    for (size_t j = testBegin; j < testEnd; ++j)
      testGraph.graph->addNode(getNode(j), getLabel(j));

    // create test links
    for (size_t j = testBegin; j < testEnd; ++j)
    {
      size_t n = getNumSuccessors(j);
      for (size_t k = 0; k < n; ++k)
      {
        size_t succ = getSuccessor(j, k);
        if (succ >= testBegin && succ < testEnd)
          testGraph.graph->addLink(j - testBegin, succ - testBegin);
      }
    }
  }
  else
  {
    testGraph.graph = LabeledContentGraphPtr(this);
    testGraph.foldBegin = testBegin;
    testGraph.foldEnd = testEnd;
  }
  res.second = testGraph;
  
  return res;
}
