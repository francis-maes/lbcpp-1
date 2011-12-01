/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraph.cpp                 | Luape Graph                     |
| Author  : Francis Maes                   |                                 |
| Started : 26/10/2011 00:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "LuapeGraph.h"
#include <algorithm>
using namespace lbcpp;

/*
** LuapeGraphUniverse
*/
/*
static void clearNode(const LuapeNodePtr& node, bool clearTrainingSamples, bool clearValidationSamples)
{
  const LuapeNodeCachePtr& cache = node->getCache();
  if (clearTrainingSamples)
    cache->clearSamples(true);
  if (clearValidationSamples)
    cache->clearSamples(false);
}

void LuapeGraphUniverse::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  for (size_t i = 0; i < inputNodes.size(); ++i)
    clearNode(inputNodes[i], clearTrainingSamples, clearValidationSamples);
  for (FunctionNodesMap::const_iterator it = functionNodes.begin(); it != functionNodes.end(); ++it)
    clearNode(it->second, clearTrainingSamples, clearValidationSamples);
}
*/
LuapeFunctionNodePtr LuapeGraphUniverse::makeFunctionNode(ClassPtr functionClass, const std::vector<Variable>& arguments, const std::vector<LuapeNodePtr>& inputs)
{
  FunctionKey key;
  key.functionClass = functionClass;
  key.arguments = arguments;
  key.inputs = inputs;
  FunctionNodesMap::const_iterator it = functionNodes.find(key);
  if (it == functionNodes.end())
  {
    LuapeFunctionPtr function = LuapeFunction::create(functionClass);
    for (size_t i = 0; i < arguments.size(); ++i)
      function->setVariable(i, arguments[i]);
    LuapeFunctionNodePtr res = new LuapeFunctionNode(function, inputs);
    functionNodes[key] = res;
    return res;
  }
  else
    return it->second;
}

LuapeFunctionNodePtr LuapeGraphUniverse::makeFunctionNode(const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
{
  std::vector<Variable> arguments(function->getNumVariables());
  for (size_t i = 0; i < arguments.size(); ++i)
    arguments[i] = function->getVariable(i);
  return makeFunctionNode(function->getClass(), arguments, inputs);
}
/*
void LuapeGraphUniverse::cacheUpdated(ExecutionContext& context, const LuapeNodePtr& node, bool isTrainingSamples)
{
  std::deque<LuapeNodePtr>& cacheSequence = isTrainingSamples ? trainingCacheSequence : validationCacheSequence;

  cacheSequence.push_back(node);
  if (cacheSequence.size() >= maxCacheSize)
  {
    cacheSequence.front()->getCache()->clearSamples(isTrainingSamples);
    cacheSequence.pop_front();
  }
}

void LuapeGraphUniverse::displayCacheInformation(ExecutionContext& context)
{
  size_t numTrainingCached = 0;
  size_t numValidationCached = 0;
  size_t numFunctionNodes = functionNodes.size();
  for (FunctionNodesMap::const_iterator it = functionNodes.begin(); it != functionNodes.end(); ++it)
  {
    const LuapeNodeCachePtr& cache = it->second->getCache();
    if (cache->getNumSamples(true) > 0)
      ++numTrainingCached;
    if (cache->getNumSamples(false) > 0)
      ++numValidationCached;
  }
  context.informationCallback(T("Train cache: ") + String((int)numTrainingCached) + T(" / ") + String((int)numFunctionNodes) +
    T(" Validation cache: ") + String((int)numValidationCached) + T(" / ") + String((int)numFunctionNodes));
}
*/
#if 0

/*
** LuapeNodeKeysMap
*/
void LuapeNodeKeysMap::clear()
{
  keyToNodes.clear();
  nodeToKeys.clear();
}

// return true if it is a new node
bool LuapeNodeKeysMap::addNodeToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodeToKeyMap::const_iterator it = nodeToKeys.find(node);
  if (it != nodeToKeys.end())
    return false; // we already know about this node

  // compute node key
  graph->updateNodeCache(context, node, true);
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();

  KeyToNodeMap::iterator it2 = keyToNodes.find(key);
  if (it2 == keyToNodes.end())
  {
    // this is a new node equivalence class
    nodeToKeys[node] = key;
    keyToNodes[key] = node;
    addSubNodesToCache(context, node);
    return true;
  }
  else
  {
    // existing node equivalence class
    //  see if new node is better than previous one to represent the class
    LuapeNodePtr previousNode = it2->second;
    if (node->getDepth() < previousNode->getDepth())
    {
      it2->second = node;
      context.informationCallback(T("Change computation of ") + previousNode->toShortString() + T(" into ") + node->toShortString());
      LuapeFunctionNodePtr sourceFunctionNode = node.dynamicCast<LuapeFunctionNode>();
      LuapeFunctionNodePtr targetFunctionNode = previousNode.dynamicCast<LuapeFunctionNode>();
      if (sourceFunctionNode && targetFunctionNode)
        sourceFunctionNode->clone(context, targetFunctionNode);
      addSubNodesToCache(context, node);
    }
    nodeToKeys[node] = it2->first;
    return false;
  }
}

void LuapeNodeKeysMap::addSubNodesToCache(ExecutionContext& context, const LuapeNodePtr& node)
{
  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (functionNode)
  {
    std::vector<LuapeNodePtr> arguments = functionNode->getArguments();
    for (size_t i = 0; i < arguments.size(); ++i)
      addNodeToCache(context, arguments[i]);
  }
}

bool LuapeNodeKeysMap::isNodeKeyNew(const LuapeNodePtr& node) const
{
  BinaryKeyPtr key = node->getCache()->makeKeyFromSamples();
  return keyToNodes.find(key) == keyToNodes.end();
}


/*
** LuapeGraph
*/
LuapeGraph::LuapeGraph(size_t maxCacheSize)
  : numYields(0), universe(new LuapeGraphUniverse(maxCacheSize))
{
}

String LuapeGraph::toShortString() const
  {return graphToString(0);}

String LuapeGraph::graphToString(size_t firstNodeIndex) const
{
  String res;
  for (size_t i = firstNodeIndex; i < nodes.size(); ++i)
    res += String((int)i) + T(" ") + nodes[i]->toShortString() + T("\n");
  return res;
}

LuapeNodePtr LuapeGraph::pushMissingNodes(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodesMap::const_iterator it = nodesMap.find(node);
  if (it != nodesMap.end())
    return node;

  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (functionNode)
  {
    size_t n = functionNode->getNumArguments();
    for (size_t i = 0; i < n; ++i)
      pushMissingNodes(context, functionNode->getArgument(i));
  }

  LuapeYieldNodePtr yieldNode = node.dynamicCast<LuapeYieldNode>();
  if (yieldNode)
    pushMissingNodes(context, yieldNode->getArgument());

  addNode(node);
  return node;
}

LuapeNodePtr LuapeGraph::pushFunctionNode(ExecutionContext& context, const LuapeFunctionPtr& function, const LuapeNodePtr& input)
{
  LuapeNodePtr node = universe->makeFunctionNode(function, std::vector<LuapeNodePtr>(1, input));
  pushNode(context, node);
  return node;
}

LuapeNodePtr LuapeGraph::pushNode(ExecutionContext& context, const LuapeNodePtr& node)
{
  NodesMap::const_iterator it = nodesMap.find(node);
  if (it != nodesMap.end())
    return node;

  LuapeInputNodePtr inputNode = node.dynamicCast<LuapeInputNode>();
  if (inputNode)
    universe->addInputNode(inputNode);

  addNode(node);
  return node;
}

void LuapeGraph::addNode(const LuapeNodePtr& node)
{
  if (node.isInstanceOf<LuapeYieldNode>())
    ++numYields;
  node->indexInGraph = nodes.size();
  nodesMap[node] = nodes.size();
  nodes.push_back(node);
}

void LuapeGraph::popNode(ExecutionContext& context)
{
  jassert(nodes.size());
  removeNode(context, nodes.size() - 1);
}

void LuapeGraph::computeNodeUseCounts(std::vector<size_t>& res) const
{
  res.clear();
  res.resize(nodes.size(), 0);
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    LuapeYieldNodePtr yieldNode = nodes[i].dynamicCast<LuapeYieldNode>();
    if (yieldNode)
    {
      ++res[yieldNode->getArgument()->getIndexInGraph()];
    }
    else
    {
      LuapeFunctionNodePtr functionNode = nodes[i].dynamicCast<LuapeFunctionNode>();
      if (functionNode)
      {
        for (size_t i = 0; i < functionNode->getNumArguments(); ++i)
          ++res[functionNode->getArgument(i)->getIndexInGraph()];
      }
    }
  }
}

void LuapeGraph::removeNode(ExecutionContext& context, size_t nodeIndex, bool alsoRemoveNewUselessNodes, bool verbose)
{
  jassert(nodeIndex < nodes.size());
  LuapeNodePtr node = nodes[nodeIndex];
  if (verbose)
    context.informationCallback(T("Remove node: ") + node->toShortString());

  if (node.isInstanceOf<LuapeYieldNode>())
    --numYields;
  nodes.erase(nodes.begin() + nodeIndex);
  nodesMap.erase(node);
  for (size_t i = nodeIndex; i < nodes.size(); ++i)
    nodes[i]->indexInGraph = i;

  if (alsoRemoveNewUselessNodes)
  {
    std::vector<size_t> useCounts;

    size_t numNodes = nodes.size();
    while (true)
    {
      computeNodeUseCounts(useCounts);
      size_t offset = 0;
      for (size_t i = 0; i < useCounts.size(); ++i)
        if (useCounts[i] == 0 && nodes[i - offset].isInstanceOf<LuapeFunctionNode>())
        {
          removeNode(context, i - offset, false, verbose);
          ++offset;
        }
      if (numNodes == nodes.size())
        break;
      numNodes = nodes.size();
    }
  }
}

void LuapeGraph::removeYieldNode(ExecutionContext& context, size_t yieldIndex)
{
  size_t index = 0;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    const LuapeNodePtr& node = nodes[i];
    if (node.isInstanceOf<LuapeYieldNode>())
    {
      if (index == yieldIndex)
      {
        removeNode(context, i, true, true);
        return;
      }
      ++index;
    }
  }
  jassert(false);
}

size_t LuapeGraph::getNumTrainingSamples() const
  {return nodes.size() ? nodes[0]->getCache()->getNumTrainingSamples() : 0;}

size_t LuapeGraph::getNumValidationSamples() const
  {return nodes.size() ? nodes[0]->getCache()->getNumValidationSamples() : 0;}

size_t LuapeGraph::getNumSamples(bool isTrainingSamples) const
  {return nodes.size() ? nodes[0]->getCache()->getNumSamples(isTrainingSamples) : 0;}

void LuapeGraph::resizeSamples(bool isTrainingSamples, size_t numSamples)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->getCache()->resizeSamples(isTrainingSamples, numSamples);
}

void LuapeGraph::resizeSamples(size_t numTrainingSamples, size_t numValidationSamples)
{
  for (size_t i = 0; i < nodes.size(); ++i)
    nodes[i]->getCache()->resizeSamples(numTrainingSamples, numValidationSamples);
}

void LuapeGraph::setSample(bool isTrainingSample, size_t index, const std::vector<Variable>& example)
{
  jassert(example.size() <= nodes.size());
  for (size_t i = 0; i < example.size(); ++i)
  {
    LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
    jassert(node);
    node->getCache()->setSample(isTrainingSample, index, example[i]);
  }
}

void LuapeGraph::setSample(bool isTrainingSample, size_t index, const ObjectPtr& example)
{
  ContainerPtr container = example.dynamicCast<Container>();
  if (container)
  {
    size_t n = container->getNumElements();
    jassert(n <= nodes.size());
    for (size_t i = 0; i < n; ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->getCache()->setSample(isTrainingSample, index, container->getElement(i));
    }
  }
  else
  {
    size_t n = example->getNumVariables();
    jassert(n <= nodes.size());
    for (size_t i = 0; i < n; ++i)
    {
      LuapeInputNodePtr node = nodes[i].dynamicCast<LuapeInputNode>();
      jassert(node);
      node->getCache()->setSample(isTrainingSample, index, example->getVariable(i));
    }
  }
}

void LuapeGraph::clearSamples(bool clearTrainingSamples, bool clearValidationSamples)
{
  getUniverse()->clearSamples(clearTrainingSamples, clearValidationSamples);
}

VectorPtr LuapeGraph::updateNodeCache(ExecutionContext& context, const LuapeNodePtr& node, bool isTrainingSamples, SparseDoubleVectorPtr* sortedDoubleValues)
{
  LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
  if (!functionNode)
  {
    if (sortedDoubleValues && node->getCache())
      *sortedDoubleValues = node->getCache()->getSortedDoubleValues();
    return node->getCache()->getSamples(isTrainingSamples); // only function nodes are subject to caching
  }

  LuapeNodeCachePtr cache = functionNode->getCache();
  VectorPtr res = cache->getSamples(isTrainingSamples);
  if (res)
  {
    if (sortedDoubleValues)
      *sortedDoubleValues = cache->getSortedDoubleValues();
  }
  else
  {
    std::vector<VectorPtr> inputs(functionNode->getNumArguments());
    for (size_t i = 0; i < inputs.size(); ++i)
      inputs[i] = updateNodeCache(context, functionNode->getArgument(i), isTrainingSamples);

    res = functionNode->getFunction()->compute(context, inputs, functionNode->getType());
    cache->setSamples(isTrainingSamples, res);
    if (sortedDoubleValues)
      *sortedDoubleValues = cache->getSortedDoubleValues();

    getUniverse()->cacheUpdated(context, node, isTrainingSamples);
  }
  return res;   
}

Variable LuapeGraph::compute(ExecutionContext& context, const LuapeInstanceCachePtr& cache) const
{
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    if (i < nodes.size() - 1)
      cache->computeNode(context, nodes[i]);
    else
      return cache->computeNode(context, nodes[i]);
  }
  return Variable();
}

void LuapeGraph::clone(ExecutionContext& context, const ObjectPtr& t) const
{
  const LuapeGraphPtr& target = t.staticCast<LuapeGraph>();
  target->nodes = nodes;
  target->nodesMap = nodesMap;
  target->universe = universe;
}

static String textToXmlText(const String& str)
{
  String res;
  for (int i = 0; i < str.length(); ++i)
  {
    if (str[i] == '>')
      res += "&gt;";
    else if (str[i] == '<')
      res += "&lt;";
    else if (str[i] == '&')
      res += "&amp;";
    else if (str[i] == '\'')
      res += "&apos;";
    else if (str[i] == '"')
      res += "&quot;";
    else
      res += str[i];
  }
  return res;
}

static void writeGraphMLEdge(OutputStream* ostr, size_t sourceIndex, size_t destIndex, const String& text)
{
  *ostr << "<edge source=\"node" << String((int)sourceIndex) << "\" target=\"node" << String((int)destIndex) << "\">\n";
  if (text.isNotEmpty())
    *ostr <<"  <data key=\"d2\"><y:PolyLineEdge><y:EdgeLabel>" << textToXmlText(text) << "</y:EdgeLabel></y:PolyLineEdge></data>\n";
  *ostr << "</edge>\n";
}

bool LuapeGraph::saveToGraphML(ExecutionContext& context, const File& file) const
{
  if (file.exists())
    file.deleteFile();
  OutputStream* ostr = file.createOutputStream();
  if (!ostr)
  {
    context.errorCallback(T("Could not write to file ") + file.getFullPathName());
    return false;
  }

  *ostr << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  *ostr << "<graphml xmlns=\"http://graphml.graphdrawing.org/xmlns\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xmlns:y=\"http://www.yworks.com/xml/graphml\" xmlns:yed=\"http://www.yworks.com/xml/yed/3\" xsi:schemaLocation=\"http://graphml.graphdrawing.org/xmlns http://www.yworks.com/xml/schema/graphml/1.1/ygraphml.xsd\">\n";
  *ostr << "  <key id=\"d1\" for=\"node\" yfiles.type=\"nodegraphics\"/>\n";
  *ostr << "  <key id=\"d2\" for=\"edge\" yfiles.type=\"edgegraphics\"/>\n";
  *ostr << "  <graph id=\"G\" edgedefault=\"directed\">\n";
  
  size_t yieldIndex = 0;
  for (size_t i = 0; i < nodes.size(); ++i)
  {
    const LuapeNodePtr& node = nodes[i];
    String id = "node" + String((int)i);
    String text;
    LuapeInputNodePtr inputNode = node.dynamicCast<LuapeInputNode>();
    LuapeFunctionNodePtr functionNode = node.dynamicCast<LuapeFunctionNode>();
    LuapeYieldNodePtr yieldNode = node.dynamicCast<LuapeYieldNode>();
    jassert(inputNode || functionNode || yieldNode);
    
    if (inputNode)
      text = "input " + inputNode->getName();
    else
      text = functionNode ? functionNode->getFunction()->toShortString() : ("yield " + String((int)yieldIndex++));
    
    *ostr << "<node id=\"" << id << "\">\n";
    *ostr << "  <data key=\"d1\"><y:ShapeNode>";
    *ostr << "<y:Geometry height=\"30.0\" width=\"75\"/>";
    *ostr << "<y:NodeLabel>" << textToXmlText(text) << "</y:NodeLabel>";
    *ostr << "</y:ShapeNode></data>\n";
    *ostr << "</node>\n";

    if (functionNode)
    {
      for (size_t j = 0; j < functionNode->getNumArguments(); ++j)
        writeGraphMLEdge(ostr, functionNode->getArgument(j)->getIndexInGraph(), i, String::empty);
    }
    else if (yieldNode)
    writeGraphMLEdge(ostr, yieldNode->getArgument()->getIndexInGraph(), i, String::empty);
  }

  *ostr << "  </graph>\n";
  *ostr << "</graphml>\n";
  delete ostr;
  return true;
}

#endif // 0