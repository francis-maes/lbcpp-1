/*-----------------------------------------.---------------------------------.
| Filename: LuapeGraphBuilderTypeSearchSpace.h | Luape Graph Builder         |
| Author  : Francis Maes                   |  Search space over              |
| Started : 20/11/2011 16:40               |    step - stack types states    |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_GRAPH_BUILDER_TYPE_SEARCH_SPACE_H_
# define LBCPP_LUAPE_GRAPH_BUILDER_TYPE_SEARCH_SPACE_H_

# include "LuapeGraph.h"
# include "LuapeProblem.h"

namespace lbcpp
{

class LuapeGraphBuilderTypeState;
typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeState> LuapeGraphBuilderTypeStatePtr;

class LuapeGraphBuilderTypeState : public Object
{
public:
  LuapeGraphBuilderTypeState(size_t depth, const std::vector<TypePtr>& stack)
    : depth(depth), stack(stack), numNodeTypesWhenBuilt(0), canBePruned(false), canBePrunedComputed(false) {}
  LuapeGraphBuilderTypeState() : depth(0), numNodeTypesWhenBuilt(0), canBePruned(false), canBePrunedComputed(false) {}

  virtual String toShortString() const
  {
    String res = T("[") + String((int)depth) + T("] {");
    for (size_t i = 0; i < stack.size(); ++i)
    {
      res += stack[i]->getName();
      if (i < stack.size() - 1)
        res += T(", ");
    }
    return res + T("}");
  }

  size_t getDepth() const
    {return depth;}

  const std::vector<TypePtr>& getStack() const
    {return stack;}

  size_t getStackSize() const
    {return stack.size();}

  bool hasPushActions() const
    {return push.size() > 0;}

  bool canTypeBePushed(const TypePtr& type) const
  {
    for (size_t i = 0; i < push.size(); ++i)
      if (push[i].first == type)
        return true;
    return false;
  }

  LuapeGraphBuilderTypeStatePtr getPushTransition(const TypePtr& type) const
  {
    for (size_t i = 0; i < push.size(); ++i)
      if (push[i].first == type)
        return push[i].second;
    return LuapeGraphBuilderTypeStatePtr();
  }

  bool hasApplyActions() const
    {return apply.size() > 0;}

  const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& getApplyActions() const
    {return apply;}

  bool hasYieldAction() const
    {return stack.size() == 1 && (
      stack[0]->inheritsFrom(booleanType) ||
      stack[0]->inheritsFrom(doubleType) ||
      (!stack[0].isInstanceOf<Enumeration>() && stack[0]->inheritsFrom(integerType)));}

  bool hasAnyAction() const
    {return hasPushActions() || hasApplyActions() || hasYieldAction();}

private:
  friend class LuapeGraphBuilderTypeStateClass;
  friend class LuapeGraphBuilderTypeSearchSpace;

  size_t depth;
  std::vector<TypePtr> stack;
  std::vector<std::pair<TypePtr, LuapeGraphBuilderTypeStatePtr> > push;
  std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> > apply;
  
  size_t numNodeTypesWhenBuilt;
  bool canBePruned;
  bool canBePrunedComputed;
  
  void setPushTransition(const TypePtr& type, const LuapeGraphBuilderTypeStatePtr& nextState)
  {
    for (size_t i = 0; i < push.size(); ++i)
      if (push[i].first == type)
      {
        jassert(push[i].second == nextState);
        return;
      }
    push.push_back(std::make_pair(type, nextState));
  }
  
  void setApplyTransition(const LuapeFunctionPtr& function, const LuapeGraphBuilderTypeStatePtr& nextState)
  {
    for (size_t i = 0; i < apply.size(); ++i)
      if (apply[i].first == function)
      {
        jassert(apply[i].second == nextState);
        return;
      }
    apply.push_back(std::make_pair(function, nextState));
  }
};


class LuapeGraphBuilderTypeSearchSpace : public Object
{
public:
  LuapeGraphBuilderTypeSearchSpace(const LuapeProblemPtr& problem, size_t maxDepth)
  {
    std::vector<TypePtr> nodeTypes;
    for (size_t i = 0; i < problem->getNumInputs(); ++i)
      insertType(nodeTypes, problem->getInput(i)->getType());
    insertType(nodeTypes, booleanType); // automatically included since boosting methods may create stump nodes that output booleans

    LuapeGraphBuilderTypeStatePtr root = getOrCreateState(0, std::vector<TypePtr>());
    size_t numTypes = nodeTypes.size();
    while (true)
    {
      buildSuccessors(problem, root, nodeTypes, maxDepth);
      size_t newNumTypes = nodeTypes.size();
      if (newNumTypes != numTypes)
      { 
        jassert(newNumTypes > numTypes);
        numTypes = newNumTypes;
      }
      else
        break;
    }
  }

  LuapeGraphBuilderTypeSearchSpace()
  {
  }

  void pruneStates(ExecutionContext& context)
  {
//    context.informationCallback(T("Num states before pruning: ") + String((int)states.size()));
    prune(getInitialState());
    //jassert(!isRootPrunable);

    StateMap::iterator it, nxt;
    for (it = states.begin(); it != states.end(); it = nxt)
    {
      nxt = it; ++nxt;
      jassert(it->second->canBePrunedComputed);
      if (it->second->canBePruned)
        states.erase(it);
    }
    context.enterScope(T("Type states"));
    for (it = states.begin(); it != states.end(); ++it)
      context.informationCallback(it->second->toShortString());
    context.leaveScope(states.size());
  }

  LuapeGraphBuilderTypeStatePtr getInitialState() const
    {return states.find(StateKey())->second;}

  LuapeGraphBuilderTypeStatePtr getState(size_t depth, const std::vector<TypePtr>& stack) const
  {
    StateMap::const_iterator it = states.find(std::make_pair(depth, stack));
    return it == states.end() ? LuapeGraphBuilderTypeStatePtr() : it->second;
  }

private:
  typedef std::pair<size_t, std::vector<TypePtr> > StateKey;
  typedef std::map<StateKey, LuapeGraphBuilderTypeStatePtr> StateMap;

  StateMap states;

  LuapeGraphBuilderTypeStatePtr getOrCreateState(size_t depth, const std::vector<TypePtr>& stack)
  {
    StateKey key(depth, stack);
    StateMap::const_iterator it = states.find(key);
    if (it == states.end())
    { 
      LuapeGraphBuilderTypeStatePtr res = new LuapeGraphBuilderTypeState(depth, stack);
      states[key] = res;
      return res;
    }
    else
      return it->second;
  }

  static void insertType(std::vector<TypePtr>& types, const TypePtr& type)
  {
    for (size_t i = 0; i < types.size(); ++i)
      if (types[i] == type)
        return;
    types.push_back(type);
  }

  void buildSuccessors(const LuapeProblemPtr& problem, const LuapeGraphBuilderTypeStatePtr& state, std::vector<TypePtr>& nodeTypes, size_t maxDepth)
  {
    if (maxDepth == state->getDepth())
    {
      state->stack.clear();
      return;
    }

    if (nodeTypes.size() == state->numNodeTypesWhenBuilt)
      return;
    state->numNodeTypesWhenBuilt = nodeTypes.size();

    size_t numRemainingSteps = maxDepth - state->getDepth();
    if (numRemainingSteps > state->getStackSize())
    {
      for (size_t i = 0; i < nodeTypes.size(); ++i)
      {
        TypePtr type = nodeTypes[i];
        LuapeGraphBuilderTypeStatePtr nextState = state->getPushTransition(type);
        if (nextState)
          buildSuccessors(problem, nextState, nodeTypes, maxDepth);
        else
        {
          // compute new stack
          std::vector<TypePtr> newStack = state->getStack();
          newStack.push_back(type);

          // create successor state and call recursively
          LuapeGraphBuilderTypeStatePtr newState = getOrCreateState(state->getDepth() + 1, newStack);
          state->setPushTransition(type, newState);
          buildSuccessors(problem, newState, nodeTypes, maxDepth);
        }
      }
    }

    for (size_t i = 0; i < problem->getNumFunctions(); ++i)
    {
      LuapeFunctionPtr function = problem->getFunction(i);
      if (acceptInputTypes(function, state->getStack()))
      {
        size_t numInputs = function->getNumInputs();
        std::vector<Variable> tmp(function->getNumVariables());

        std::vector<TypePtr> inputTypes(numInputs);
        for (size_t i = 0; i < numInputs; ++i)
          inputTypes[i] = state->stack[state->getStackSize() - numInputs + i];

        std::vector<LuapeFunctionPtr> functions;
        enumerateFunctionVariables(function, inputTypes, tmp, 0, functions);

        for (size_t j = 0; j < functions.size(); ++j)
          applyFunctionAndBuildSuccessor(problem, state, functions[j], nodeTypes, maxDepth);
      }
    }
  }

  void enumerateFunctionVariables(const LuapeFunctionPtr& function, const std::vector<TypePtr>& inputTypes, std::vector<Variable>& variables, size_t variableIndex, std::vector<LuapeFunctionPtr>& res)
  {
    if (variableIndex == variables.size())
    {
      LuapeFunctionPtr fun = function->cloneAndCast<LuapeFunction>();
      for (size_t i = 0; i < variables.size(); ++i)
        fun->setVariable(i, variables[i]);
      res.push_back(fun);
    }
    else
    {
      ContainerPtr values = function->getVariableCandidateValues(variableIndex, inputTypes);
      if (values)
      {
        size_t n = values->getNumElements();
        for (size_t i = 0; i < n; ++i)
        {
          variables[variableIndex] = values->getElement(i);
          enumerateFunctionVariables(function, inputTypes, variables, variableIndex + 1, res);
        }
      }
    }
  }

  void applyFunctionAndBuildSuccessor(const LuapeProblemPtr& problem, const LuapeGraphBuilderTypeStatePtr& state, const LuapeFunctionPtr& function, std::vector<TypePtr>& nodeTypes, size_t maxDepth)
  {
    // compute output type given input types
    size_t numInputs = function->getNumInputs();
    std::vector<TypePtr> inputs(numInputs);
    for (size_t i = 0; i < numInputs; ++i)
      inputs[i] = state->stack[state->getStackSize() - numInputs + i];

    TypePtr outputType = function->getOutputType(inputs);
    insertType(nodeTypes, outputType); // the function output type can become a graph node in later episodes

    // compute new stack and new graph node types
    std::vector<TypePtr> newStack(state->getStackSize() - numInputs + 1);
    for (size_t i = 0; i < state->getStackSize() - numInputs; ++i)
      newStack[i] = state->stack[i];
    newStack.back() = outputType;

    // create successor state and call recursively
    LuapeGraphBuilderTypeStatePtr newState = getOrCreateState(state->getDepth() + 1, newStack);
    state->setApplyTransition(function, newState);
    buildSuccessors(problem, newState, nodeTypes, maxDepth);
  }

  bool acceptInputTypes(const LuapeFunctionPtr& function, const std::vector<TypePtr>& stack) const
  {
    size_t numInputs = function->getNumInputs();
    if (numInputs > stack.size())
      return false;
    size_t stackFirstIndex = stack.size() - numInputs;
    for (size_t i = 0; i < numInputs; ++i)
      if (!function->doAcceptInputType(i, stack[stackFirstIndex + i]))
        return false;
    return true;
  }

  bool prune(LuapeGraphBuilderTypeStatePtr state) // return true if state is prunable
  {
    if (state->canBePrunedComputed)
      return state->canBePruned;

    {
      std::vector<std::pair<TypePtr, LuapeGraphBuilderTypeStatePtr> > remainingTransitions;
      for (size_t i = 0; i < state->push.size(); ++i)
        if (!prune(state->push[i].second))
          remainingTransitions.push_back(state->push[i]);
      state->push.swap(remainingTransitions);
    }
    {
      std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> > remainingTransitions;
      for (size_t i = 0; i < state->apply.size(); ++i)
        if (!prune(state->apply[i].second))
          remainingTransitions.push_back(state->apply[i]);
      state->apply.swap(remainingTransitions);
    }
    state->canBePruned = !state->hasAnyAction();
    state->canBePrunedComputed = true;
    return state->canBePruned;
  }
};

typedef ReferenceCountedObjectPtr<LuapeGraphBuilderTypeSearchSpace> LuapeGraphBuilderTypeSearchSpacePtr;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_GRAPH_BUILDER_TYPE_SEARCH_SPACE_H_