/*-----------------------------------------.---------------------------------.
| Filename: MCAlgorithm.h                  | Monte Carlo Algorithms          |
| Author  : Francis Maes                   |                                 |
| Started : 07/04/2012 15:01               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUAPE_MC_ALGORITHM_H_
# define LBCPP_LUAPE_MC_ALGORITHM_H_

# include "MCObjective.h"
# include "../Core/SinglePlayerMCTSOptimizer.h"

namespace lbcpp
{

/*
** Search Algorithm
*/
class MCAlgorithm : public Object
{
public:
  double search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState, std::vector<Variable>* actions, DecisionProblemStatePtr& finalState)
  {
    bestScore = -DBL_MAX;
    bestActions.clear();
    bestFinalState = DecisionProblemStatePtr();

    //String dbg = initialState->toShortString();

    /*ContainerPtr dbgActions = initialState->getAvailableActions();
    if (!dbgActions || !dbgActions->getNumElements())
      return bestScore;*/

    search(context, objective, initialState);
    jassert((bestScore == -DBL_MAX) == (bestFinalState == DecisionProblemStatePtr()));
    jassert((bestScore == -DBL_MAX) == bestActions.empty());
    //String dbg2 = initialState->toShortString();
    //jassert(dbg == dbg2);

    if (actions)
    {
      actions->reserve(actions->size() + bestActions.size());
      for (size_t i = 0; i < bestActions.size(); ++i)
        actions->push_back(bestActions[i]);
    }
    
    finalState = bestFinalState;
    /*if (bestScore != -DBL_MAX)
    {
      size_t i;
      for (i = 0; i < dbgActions->getNumElements(); ++i)
        if (firstAction.toShortString() == dbgActions->getElement(i).toShortString())
          break;
      jassert(i < dbgActions->getNumElements());
    }*/
    return bestScore;
  }    

  virtual void reset(ExecutionContext& context) {}

protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState) = 0;

  double bestScore;
  DecisionProblemStatePtr bestFinalState;
  std::vector<Variable> bestActions;

  double submitFinalState(ExecutionContext& context, MCObjectivePtr objective, const std::vector<Variable>& actions, DecisionProblemStatePtr state, double score = -DBL_MAX)
  {
    if (score == -DBL_MAX)
    {
      score = objective->evaluate(context, state);
      //context.informationCallback(state->toShortString() + T(" => ") + String(score));
    }
    if (score > bestScore)
    {
      bestScore = score;
      bestFinalState = state;
      bestActions = actions;
      jassert(actions.size());
    }
    return score;
  }
};

typedef ReferenceCountedObjectPtr<MCAlgorithm> MCAlgorithmPtr;
extern ClassPtr mcAlgorithmClass;

class RolloutMCAlgorithm : public MCAlgorithm
{
protected:
  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
    size_t t = 0;
    std::vector<Variable> actions;
    while (!state->isFinalState() && !objective->shouldStop())
    {
      ContainerPtr availableActions = state->getAvailableActions();
      size_t n = availableActions->getNumElements();
      Variable action = availableActions->getElement(context.getRandomGenerator()->sampleSize(n));
      actions.push_back(action);
      double reward;
      state->performTransition(context, action, reward);
      ++t;
    }
    if (state->isFinalState())
      submitFinalState(context, objective, actions, state);
  }
};

extern ClassPtr rolloutMCAlgorithmClass;

class DecoratorMCAlgorithm : public MCAlgorithm
{
public:
  DecoratorMCAlgorithm(MCAlgorithmPtr algorithm)
    : algorithm(algorithm) {}
  DecoratorMCAlgorithm() {}

  virtual void clone(ExecutionContext& context, const ObjectPtr& target) const
  {
    MCAlgorithm::clone(context, target);
    if (algorithm)
      target.staticCast<DecoratorMCAlgorithm>()->algorithm = algorithm->cloneAndCast<MCAlgorithm>();
  }

  const MCAlgorithmPtr& getAlgorithm() const
    {return algorithm;}

  virtual void reset(ExecutionContext& context)
    {if (algorithm) algorithm->reset(context);}

protected:
  friend class DecoratorMCAlgorithmClass;

  MCAlgorithmPtr algorithm;

  double subSearch(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr state, std::vector<Variable>& actions, DecisionProblemStatePtr& finalState)
  {
    if (state->isFinalState())
      return submitFinalState(context, objective, actions, state);
    else
    {
      double res = algorithm->search(context, objective, state, &actions, finalState);
      if (res != -DBL_MAX)
        submitFinalState(context, objective, actions, finalState, res);
      return res;
    }
  }
};

typedef ReferenceCountedObjectPtr<DecoratorMCAlgorithm> DecoratorMCAlgorithmPtr;

class IterateMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  IterateMCAlgorithm(MCAlgorithmPtr algorithm, size_t numIterations)
    : DecoratorMCAlgorithm(algorithm), numIterations(numIterations) {}
  IterateMCAlgorithm() {}

  size_t getNumIterations() const
    {return numIterations;}

protected:
  friend class IterateMCAlgorithmClass;

  size_t numIterations;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    for (size_t i = 0; !numIterations || i < numIterations; ++i)
    {
      if (objective->shouldStop())
        break;

      std::vector<Variable> actions;
      DecisionProblemStatePtr finalState;
      subSearch(context, objective, initialState, actions, finalState);
    }
  }
};

extern ClassPtr iterateMCAlgorithmClass;

class LookAheadMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  LookAheadMCAlgorithm(MCAlgorithmPtr algorithm, double numActions = 1.0)
    : DecoratorMCAlgorithm(algorithm), numActions(numActions) {}
  LookAheadMCAlgorithm() : numActions(0.0) {}

protected:
  friend class LookAheadMCAlgorithmClass;

  double numActions;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr state)
  {
    ContainerPtr actions = state->getAvailableActions();
    size_t n = actions->getNumElements();

    std::vector<size_t> order;
    context.getRandomGenerator()->sampleOrder(n, order);
    std::vector<Variable> selectedActions((size_t)(juce::jmax(1.0, n * numActions)));
    for (size_t i = 0; i < selectedActions.size(); ++i)
      selectedActions[i] = actions->getElement(order[i]);

    for (size_t i = 0; i < selectedActions.size(); ++i)
    {
      if (objective->shouldStop())
        break;
      Variable action = selectedActions[i];
      double reward;
      Variable stateBackup;
      state->performTransition(context, action, reward, &stateBackup);

      std::vector<Variable> actions(1, action);
      DecisionProblemStatePtr finalState;
      subSearch(context, objective, state, actions, finalState);

      state->undoTransition(context, stateBackup);
    }
  }
};

extern ClassPtr lookAheadMCAlgorithmClass;

class StepByStepMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  StepByStepMCAlgorithm(MCAlgorithmPtr algorithm, bool useGlobalBest)
    : DecoratorMCAlgorithm(algorithm), useGlobalBest(useGlobalBest) {}
  StepByStepMCAlgorithm() : useGlobalBest(false) {}

protected:
  friend class StepByStepMCAlgorithmClass;

  bool useGlobalBest;

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    DecisionProblemStatePtr state = initialState->cloneAndCast<DecisionProblemState>();
    std::vector<Variable> actions;
    //context.informationCallback(T("----"));
    while (!state->isFinalState() && !objective->shouldStop())
    {
      std::vector<Variable> bestActions(actions.size());
      for (size_t i = 0; i < actions.size(); ++i)
        bestActions[i] = actions[i];
      
      DecisionProblemStatePtr bestFinalState;
      /*double score = */subSearch(context, objective, state, bestActions, bestFinalState);

      /*if (bestFinalState)
      {
        String info = "Best Actions:";
        for (size_t i = 0; i < bestActions.size(); ++i)
          info += " " + bestActions[i].toShortString();
        context.informationCallback(info + T(" (") + String(score) + T(")"));
      }
      else
        context.informationCallback("No Best Final State");*/

      Variable selectedAction;
      if (useGlobalBest && this->bestFinalState)
        selectedAction = this->bestActions[actions.size()];
      else if (bestFinalState)
        selectedAction = bestActions[actions.size()];
      
      if (!selectedAction.exists())
      {
        //context.informationCallback(T("Break"));
        break;
      }

      actions.push_back(selectedAction);

      double reward;
      state->performTransition(context, selectedAction, reward);
      
      while (!state->isFinalState())
      {
        ContainerPtr actions = state->getAvailableActions();
        if (actions->getNumElements() > 1)
          break;
        state->performTransition(context, actions->getElement(0), reward);
      }
    }
  }
};

extern ClassPtr stepByStepMCAlgorithmClass;

// politique en dur:  rk + 2 / tk
class SelectMCAlgorithm : public DecoratorMCAlgorithm
{
public:
  SelectMCAlgorithm(MCAlgorithmPtr algorithm, double explorationCoefficient = 0.5)
    : DecoratorMCAlgorithm(algorithm), explorationCoefficient(explorationCoefficient) {}
  SelectMCAlgorithm() {}

  virtual void search(ExecutionContext& context, MCObjectivePtr objective, DecisionProblemStatePtr initialState)
  {
    SinglePlayerMCTSNodePtr& root = trees[initialState->toShortString()];
    if (!root)
      root = new SinglePlayerMCTSNode(initialState->cloneAndCast<DecisionProblemState>());

    SinglePlayerMCTSNodePtr leaf = root->select(context);

    //std::cout << "Selected node: " << leaf->toShortString() << std::flush;

    if (!leaf->isExpanded())
      leaf->expand(context);

    std::vector<Variable> bestActions;
    for (SinglePlayerMCTSNodePtr ptr = leaf; ptr != root; ptr = ptr->getParentNode())
      bestActions.insert(bestActions.begin(), ptr->getLastAction());

/*    size_t initialDepth = initialState.staticCast<LuapeNodeBuilderState>()->getCurrentStep();
    size_t leafDepth = leaf->getState().staticCast<LuapeNodeBuilderState>()->getCurrentStep();
    jassert(leafDepth - initialDepth == bestActions.size());*/

    DecisionProblemStatePtr bestFinalState;
    double reward = subSearch(context, objective, leaf->getState(), bestActions, bestFinalState);

    
   // std::cout << " ==> reward = " << reward << std::endl;
    leaf->backPropagate(reward);
  }

  virtual void reset(ExecutionContext& context)
  {
    trees.clear();
    DecoratorMCAlgorithm::reset(context);
  }

private:
  friend class SelectMCAlgorithmClass;

  double explorationCoefficient;

  std::map<String, SinglePlayerMCTSNodePtr> trees;
};

extern ClassPtr selectMCAlgorithmClass;

/*
** Constructors
*/
inline MCAlgorithmPtr rollout()
  {return new RolloutMCAlgorithm();}

inline MCAlgorithmPtr iterate(MCAlgorithmPtr algorithm, size_t numIterations)
  {return new IterateMCAlgorithm(algorithm, numIterations);}

inline MCAlgorithmPtr step(MCAlgorithmPtr algorithm, bool useGlobalBest = true)
  {return new StepByStepMCAlgorithm(algorithm, useGlobalBest);}

inline MCAlgorithmPtr lookAhead(MCAlgorithmPtr algorithm, double numActions = 1.0)
  {return new LookAheadMCAlgorithm(algorithm, numActions);}

inline MCAlgorithmPtr select(MCAlgorithmPtr algorithm)
  {return new SelectMCAlgorithm(algorithm);}

/*
** MCAlgorithmSet
*
class MCAlgorithmSet : public Object
{
public:
  MCAlgorithmSet(size_t maxSize)
  {
    addAlgorithm(rollout());
    for (size_t i = 1; i < maxSize; ++i)
      addAlgorithmSuccessors();
    pruneIterateAlgorithms();
  }

  MCAlgorithmSet() {}

  void addAlgorithm(MCAlgorithmPtr algorithm)
  {
    algorithm = canonizeAlgorithm(algorithm);
    String str = algorithm->toShortString();
    AlgorithmMap::const_iterator it = m.find(str);
    if (it == m.end())
      m[str] = algorithm;
  }

  void addAlgorithmSuccessors()
  {
    AlgorithmMap m = this->m;
    for (AlgorithmMap::const_iterator it = m.begin(); it != m.end(); ++it)
      expandAlgorithm(it->second);
  }

  void pruneIterateAlgorithms()
  {
    AlgorithmMap::iterator it, nxt;
    for (it = m.begin(); it != m.end(); it = nxt)
    {
      nxt = it; ++nxt;
      if (it->second.isInstanceOf<IterateMCAlgorithm>())
        m.erase(it);
    }
  }

  size_t getNumAlgorithms() const
    {return m.size();}

  void getAlgorithms(std::vector<MCAlgorithmPtr>& res)
  {
    res.clear();
    res.reserve(m.size());
    for (AlgorithmMap::const_iterator it = m.begin(); it != m.end(); ++it)
      res.push_back(it->second);
  }

protected:
  typedef std::map<String, MCAlgorithmPtr> AlgorithmMap;

  AlgorithmMap m;

  void expandAlgorithm(MCAlgorithmPtr algorithm)
  {
    addAlgorithm(iterate(algorithm, 2));
    addAlgorithm(iterate(algorithm, 5));
    addAlgorithm(step(algorithm, false));
    addAlgorithm(step(algorithm, true));
    addAlgorithm(lookAhead(algorithm, 0.1));
    addAlgorithm(lookAhead(algorithm, 0.5));
    addAlgorithm(lookAhead(algorithm, 1.0));
  }

  MCAlgorithmPtr canonizeAlgorithm(MCAlgorithmPtr algorithm)
  {
    DecoratorMCAlgorithmPtr decorator = algorithm.dynamicCast<DecoratorMCAlgorithm>();
    if (!decorator)
      return algorithm;
    MCAlgorithmPtr subAlgorithm = decorator->getAlgorithm();
    if (decorator.isInstanceOf<IterateMCAlgorithm>() && subAlgorithm.isInstanceOf<IterateMCAlgorithm>())
    {
      
      return iterate(subAlgorithm.staticCast<IterateMCAlgorithm>()->getAlgorithm(),
                decorator.staticCast<IterateMCAlgorithm>()->getNumIterations() * 
                      subAlgorithm.staticCast<IterateMCAlgorithm>()->getNumIterations());
    }

    return algorithm;
  }
};*/

}; /* namespace lbcpp */

#endif // !LBCPP_LUAPE_MC_ALGORITHM_H_