/*-----------------------------------------.---------------------------------.
| Filename: NodeBuilderDecisionProblem.h   | Luape Node Builder              |
| Author  : Francis Maes                   |  Decision Problem               |
| Started : 25/10/2011 18:48               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_
# define LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_

# include "NodeBuilderTypeSearchSpace.h"
# include <lbcpp/DecisionProblem/DecisionProblem.h>

namespace lbcpp
{

#if 0
class LuapeNodeBuilderAction;
typedef ReferenceCountedObjectPtr<LuapeNodeBuilderAction> LuapeNodeBuilderActionPtr;

extern ClassPtr luapeNodeBuilderActionClass;

class LuapeNodeBuilderAction : public Object
{
public:
  LuapeNodeBuilderAction(size_t numNodesToRemove, const LuapeNodePtr& nodeToAdd)
    : Object(luapeNodeBuilderActionClass), numNodesToRemove(numNodesToRemove), nodeToAdd(nodeToAdd) {}
  LuapeNodeBuilderAction() : numNodesToRemove(0) {}

  static LuapeNodeBuilderActionPtr push(const LuapeNodePtr& node)
    {return new LuapeNodeBuilderAction(0, node);}

  static LuapeNodeBuilderActionPtr apply(const LuapeUniversePtr& universe, const LuapeFunctionPtr& function, const std::vector<LuapeNodePtr>& inputs)
    {return new LuapeNodeBuilderAction(inputs.size(), universe->makeFunctionNode(function, inputs));}

  static LuapeNodeBuilderActionPtr yield()
    {return new LuapeNodeBuilderAction(0, LuapeNodePtr());}

  const LuapeNodePtr& getNodeToAdd() const
    {return nodeToAdd;}

  bool isYield() const
    {return numNodesToRemove == 0 && !nodeToAdd;}
  
  virtual String toShortString() const
  {
    if (numNodesToRemove == 0)
      return nodeToAdd ? T("push(") + nodeToAdd->toShortString() + T(")") : T("yield");
    else
      return T("apply(") + nodeToAdd.staticCast<LuapeFunctionNode>()->getFunction()->toShortString() + T(")");
  }

  //bool isUseless(const LuapeGraphPtr& graph) const
  //  {return nodeToAdd && isNewNode() && graph->containsNode(nodeToAdd);}

  void perform(std::vector<LuapeNodePtr>& stack)
  {
    if (numNodesToRemove)
    {
      jassert(stack.size() >= numNodesToRemove);
      removedNodes.resize(numNodesToRemove);
      size_t firstIndex = stack.size() - numNodesToRemove;
      for (size_t i = 0; i < numNodesToRemove; ++i)
        removedNodes[i] = stack[firstIndex + i];
      stack.erase(stack.begin() + firstIndex, stack.end());
    }
    if (nodeToAdd)
      stack.push_back(nodeToAdd);
  }

  void undo(std::vector<LuapeNodePtr>& stack)
  {
    if (nodeToAdd)
    {
      jassert(stack.size() && stack.back() == nodeToAdd);
      stack.pop_back();
    }
    if (numNodesToRemove)
    {
      jassert(removedNodes.size() == numNodesToRemove);
      for (size_t i = 0; i < numNodesToRemove; ++i)
        stack.push_back(removedNodes[i]);
    }
  }

private:
  size_t numNodesToRemove;
  LuapeNodePtr nodeToAdd;
  std::vector<LuapeNodePtr> removedNodes; // use in state backup only
};
#endif // 0

class LuapeNodeBuilderState : public DecisionProblemState
{
public:
  LuapeNodeBuilderState(const LuapeInferencePtr& function, LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace, const LuapeRPNSequencePtr& subSequence = LuapeRPNSequencePtr())
    : function(function), typeSearchSpace(typeSearchSpace), typeState(typeSearchSpace->getInitialState()), numSteps(0), isAborted(false), isYielded(false)
  {
    if (subSequence)
    {
      for (size_t i = 0; i < subSequence->getLength(); ++i)
        performAction(subSequence->getElement(i));
    }
  }
  LuapeNodeBuilderState() : numSteps(0), isAborted(false), isYielded(false) {}

  virtual String toShortString() const
  {
    String seps = isFinalState() ? T("[]") : T("{}");

    String res;
    if (isAborted)
      res += T("canceled - ");
    res += seps[0];
    for (size_t i = 0; i < stack.size(); ++i)
    {
      res += stack[i]->toShortString();
      if (i < stack.size() - 1)
        res += T(", ");
    }
    res += seps[1];
    return res;
  }

  virtual TypePtr getActionType() const
    {return objectClass;} // sumType(luapeNodeClass, luapeFunctionClass)
      
  virtual ContainerPtr getAvailableActions() const
  {
    if (availableActions)
      return availableActions;

    if (!typeState)
      return ContainerPtr();

    ObjectVectorPtr res = new ObjectVector(objectClass, 0);
    const_cast<LuapeNodeBuilderState* >(this)->availableActions = res;

    if (typeState->hasPushActions())
    {
      // constants
      for (size_t i = 0; i < function->getNumConstants(); ++i)
        addPushActionIfAvailable(res, function->getConstant(i));

      // inputs
      for (size_t i = 0; i < function->getNumInputs(); ++i)
        addPushActionIfAvailable(res, function->getInput(i));

      // active variables
      const std::set<LuapeNodePtr>& activeVariables = function->getActiveVariables();
      for (std::set<LuapeNodePtr>::const_iterator it = activeVariables.begin(); it != activeVariables.end(); ++it)
        addPushActionIfAvailable(res, *it);
    }
    if (typeState->hasApplyActions())
    {
      const std::vector<std::pair<LuapeFunctionPtr, LuapeGraphBuilderTypeStatePtr> >& apply = typeState->getApplyActions();
      for (size_t i = 0; i < apply.size(); ++i)
      {
        LuapeFunctionPtr function = apply[i].first;
        if (function->acceptInputsStack(stack))
        {
          res->append(function);
          /*size_t numInputs = function->getNumInputs();
          std::vector<LuapeNodePtr> inputs(numInputs);
          for (size_t i = 0; i < numInputs; ++i)
            inputs[i] = stack[stack.size() - numInputs + i];

          LuapeNodeBuilderActionPtr action = LuapeNodeBuilderAction::apply(this->function->getUniverse(), function, inputs);
          // if (!action->isUseless(graph)) // useless == the created node already exists in the graph
            res->append(action);*/
        }
      }
    }

    if (typeState->hasYieldAction())
      res->append(ObjectPtr());
    return res;
  }

  virtual void performTransition(ExecutionContext& context, const Variable& a, double& reward, Variable* stateBackup = NULL)
  {
    const ObjectPtr& action = a.getObject();
    if (stateBackup)
      *stateBackup = action;
    performAction(action);

    reward = 0.0;
    ++numSteps;
    availableActions = ContainerPtr();
    if (action == ObjectPtr()) // yield
    {
      isYielded = true;
      typeState = LuapeGraphBuilderTypeStatePtr();
    }
    else
      updateTypeState();
  }

  virtual bool undoTransition(ExecutionContext& context, const Variable& stateBackup)
  {
    const ObjectPtr& action = stateBackup.getObject();
    isAborted = isYielded = false;
    --numSteps;

    jassert(stack.size());
    if (action.isInstanceOf<LuapeNode>()) // push action
      stack.pop_back();
    else if (action.isInstanceOf<LuapeFunction>()) // apply action
    {
      LuapeFunctionNodePtr functionNode = stack.back().staticCast<LuapeFunctionNode>();
      stack.pop_back();
      size_t n = functionNode->getNumArguments();
      for (size_t i = 0; i < n; ++i)
        stack.push_back(functionNode->getArgument(i));
    }

    availableActions = ContainerPtr();
    updateTypeState();
    return true;
  }

  virtual bool isFinalState() const
    {return isAborted || isYielded || !typeState || !typeState->hasAnyAction();}

  LuapeInferencePtr getFunction() const
    {return function;}

  size_t getCurrentStep() const
    {return numSteps;}

  size_t getStackSize() const
    {return stack.size();}

  const LuapeNodePtr& getStackElement(size_t index) const
    {jassert(index < stack.size()); return stack[index];}

  void setStackElement(size_t index, const LuapeNodePtr& node)
    {jassert(index < stack.size()); stack[index] = node;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class LuapeNodeBuilderStateClass;

  LuapeInferencePtr function;
  LuapeGraphBuilderTypeSearchSpacePtr typeSearchSpace;
  LuapeGraphBuilderTypeStatePtr typeState;
  ContainerPtr availableActions;
  //LuapeNodeKeysMapPtr nodeKeys;

  std::vector<LuapeNodePtr> stack;
  size_t numSteps;
  bool isAborted;
  bool isYielded;

  void updateTypeState()
  {
    std::vector<TypePtr> types(stack.size());
    for (size_t i = 0; i < types.size(); ++i)
      types[i] = stack[i]->getType();
    typeState = typeSearchSpace->getState(numSteps, types);
    jassert(typeState);
  }

  void addPushActionIfAvailable(const ObjectVectorPtr& availableActions, const LuapeNodePtr& node) const
  {
    if (typeState->hasPushAction(node->getType()))
      availableActions->append(node);
  }

  void performAction(ObjectPtr action)
  {
    if (action.isInstanceOf<LuapeNode>()) // push action
      stack.push_back(action.staticCast<LuapeNode>());
    else if (action.isInstanceOf<LuapeFunction>()) // apply action
    {
      const LuapeFunctionPtr& function = action.staticCast<LuapeFunction>();
      size_t numInputs = function->getNumInputs();
      jassert(stack.size() >= numInputs);
      std::vector<LuapeNodePtr> inputs(numInputs);
      for (size_t i = 0; i < numInputs; ++i)
        inputs[i] = stack[stack.size() - numInputs + i];
      stack.resize(stack.size() - numInputs + 1);
      stack.back() = this->function->getUniverse()->makeFunctionNode(function, inputs);
    }
  }
};

typedef ReferenceCountedObjectPtr<LuapeNodeBuilderState> LuapeNodeBuilderStatePtr;
extern ClassPtr luapeNodeBuilderStateClass;

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_NODE_BUILDER_DECISION_PROBLEM_H_
