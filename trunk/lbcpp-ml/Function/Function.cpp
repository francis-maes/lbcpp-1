/*-----------------------------------------.---------------------------------.
| Filename: Function.cpp                   | Function Base Class             |
| Author  : Francis Maes                   |                                 |
| Started : 01/12/2011 13:04               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#include "precompiled.h"
#include <lbcpp-ml/Function.h>
#include <lbcpp-ml/Expression.h>
#include <lbcpp/Luape/LuapeCache.h>
using namespace lbcpp;

String Function::makeNodeName(const std::vector<ExpressionPtr>& inputs) const
{
  ClassPtr thisClass = getClass();
  String res = (thisClass->getShortName().isNotEmpty() ? thisClass->getShortName() : thisClass->getName()) + T("(");
  for (size_t i = 0; i < inputs.size(); ++i)
  {
    res += inputs[i]->toShortString();
    if (i < inputs.size() - 1)
      res += T(", ");
  }
  return res + T(")");
}

LuapeSampleVectorPtr Function::compute(ExecutionContext& context, const std::vector<LuapeSampleVectorPtr>& inputs, TypePtr outputType) const
{
  jassert(inputs.size() && inputs.size() == getNumInputs());

  size_t n = inputs[0]->size();
  VectorPtr res = vector(outputType, n);

  std::vector<LuapeSampleVector::const_iterator> it(inputs.size());
  for (size_t i = 0; i < it.size(); ++i)
    it[i] = inputs[i]->begin();

  for (size_t i = 0; i < n; ++i)
  {
    std::vector<Variable> inputValues(inputs.size());
    for (size_t j = 0; j < inputValues.size(); ++j)
    {
      inputValues[j] = *it[j];
      ++(it[j]);
    }
    res->setElement(i, compute(context, &inputValues[0]));
  }
  return new LuapeSampleVector(inputs[0]->getIndices(), res);
}

bool Function::acceptInputsStack(const std::vector<ExpressionPtr>& stack) const
{
  size_t n = getNumInputs();
  if (n > stack.size())
    return false;
  size_t stackFirstIndex = stack.size() - n;
  for (size_t i = 0; i < n; ++i)
    if (!doAcceptInputType(i, stack[stackFirstIndex + i]->getType()))
      return false;

  return true; // !!! DISCARDS FLAGS for the moment

  if (n)
  {
    if (hasFlags(commutativeFlag))
    {
      ExpressionPtr node = stack[stackFirstIndex];
      for (size_t i = 1; i < n; ++i)
      {
        ExpressionPtr newNode = stack[stackFirstIndex + i];
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