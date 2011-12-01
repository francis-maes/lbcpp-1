/*-----------------------------------------.---------------------------------.
| Filename: LuapeFunction.cpp              | Luape Function                  |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include "LuapeFunction.h"
#include "LuapeNode.h"
using namespace lbcpp;

String LuapeFunction::toShortString(const std::vector<LuapeNodePtr>& inputs) const
{
  String res = getClass()->getShortName() + T("(");
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    res += inputs[i]->toShortString();
    if (i < inputs.size() - 1)
      res += T(", ");
  }
  return res + T(")");
}

VectorPtr LuapeFunction::compute(ExecutionContext& context, const std::vector<VectorPtr>& inputs, TypePtr outputType) const
{
  jassert(inputs.size() == getNumInputs());
  jassert(inputs.size());

  size_t n = inputs[0]->getNumElements();
  VectorPtr res = vector(outputType, n);
  for (size_t i = 0; i < n; ++i)
  {
    std::vector<Variable> inputValues(inputs.size());
    for (size_t j = 0; j < inputValues.size(); ++j)
      inputValues[j] = inputs[j]->getElement(i);
    res->setElement(i, compute(context, &inputValues[0]));
  }
  return res;
}

bool LuapeFunction::acceptInputsStack(const std::vector<LuapeNodePtr>& stack) const
{
  size_t n = getNumInputs();
  if (n > stack.size())
    return false;
  size_t stackFirstIndex = stack.size() - n;
  for (size_t i = 0; i < n; ++i)
    if (!doAcceptInputType(i, stack[stackFirstIndex + i]->getType()))
      return false;

  if (n)
  {
    if (hasFlags(commutativeFlag))
    {
      LuapeNodePtr node = stack[stackFirstIndex];
      for (size_t i = 1; i < n; ++i)
      {
        LuapeNodePtr newNode = stack[stackFirstIndex + i];
        if (newNode->getAllocationIndex() < node->getAllocationIndex())
          return false;
        node = newNode;
      }
    }
    if (hasFlags(allSameArgIrrelevantFlag))
    {
      bool ok = false;
      for (size_t i = 1; i < n; ++i)
        if (stack[stackFirstIndex + i] != stack[stackFirstIndex])
        {
          ok = true;
          break;
        }
      if (!ok)
        return false;
    }
  }
  return true;
}