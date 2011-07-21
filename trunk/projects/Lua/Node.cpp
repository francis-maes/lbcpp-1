/*-----------------------------------------.---------------------------------.
| Filename: Node.cpp                       | Lua AST                         |
| Author  : Francis Maes                   |                                 |
| Started : 21/07/2011 15:31               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "Node.h"
#include "Visitor.h"
#include "PrettyPrinterVisitor.h"
using namespace lbcpp::lua;

String Node::print() const
  {return PrettyPrinterVisitor::print(*this);}

#define NODE_ACCEPT_FUNCTION(Class) \
  void Class ::accept(Visitor& visitor) \
    {return visitor.visit(*this);}

NODE_ACCEPT_FUNCTION(List)
NODE_ACCEPT_FUNCTION(Block)

// statements
NODE_ACCEPT_FUNCTION(Do)
NODE_ACCEPT_FUNCTION(Set)
NODE_ACCEPT_FUNCTION(While)
NODE_ACCEPT_FUNCTION(Return)
NODE_ACCEPT_FUNCTION(CallStatement)

// expressions
NODE_ACCEPT_FUNCTION(Nil)
NODE_ACCEPT_FUNCTION(Dots)
NODE_ACCEPT_FUNCTION(True)
NODE_ACCEPT_FUNCTION(False)
NODE_ACCEPT_FUNCTION(LiteralNumber)
NODE_ACCEPT_FUNCTION(LiteralString)
NODE_ACCEPT_FUNCTION(Function)
NODE_ACCEPT_FUNCTION(UnaryOperation)
NODE_ACCEPT_FUNCTION(BinaryOperation)
NODE_ACCEPT_FUNCTION(Parenthesis)
NODE_ACCEPT_FUNCTION(Call)
NODE_ACCEPT_FUNCTION(Identifier)
NODE_ACCEPT_FUNCTION(Index)

ExpressionPtr lbcpp::lua::multiply(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() * rightNumber->getValue());
  
  if ((leftNumber && !rightNumber) || (!leftNumber && rightNumber))
  {
    double number = (leftNumber ? leftNumber->getValue() : rightNumber->getValue());
    ExpressionPtr expr = (leftNumber ? right : left);
    if (number == 0.0)
      return new LiteralNumber(0.0);
    else if (number == 1.0)
      return expr;
    else if (number == -1.0)
      return new UnaryOperation(unmOp, expr);
  }

  return new BinaryOperation(mulOp, left, right);
}

ExpressionPtr lbcpp::lua::add(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() + rightNumber->getValue());

  // x + 0 = x, 0 + x = x
  if ((leftNumber && !rightNumber) || (!leftNumber && rightNumber))
  {
    double number = (leftNumber ? leftNumber->getValue() : rightNumber->getValue());
    ExpressionPtr expr = (leftNumber ? right : left);
    if (number == 0.0)
      return expr;
  }

  return new Parenthesis(new BinaryOperation(addOp, left, right));
}

ExpressionPtr lbcpp::lua::sub(const ExpressionPtr& left, const ExpressionPtr& right)
{
  LiteralNumberPtr leftNumber = left.dynamicCast<LiteralNumber>();
  LiteralNumberPtr rightNumber = right.dynamicCast<LiteralNumber>();

  if (leftNumber && rightNumber)
    return new LiteralNumber(leftNumber->getValue() - rightNumber->getValue());

  // x - 0 = x
  if (!leftNumber && rightNumber->getValue() == 0.0)
    return left;

  // 0 - x = -x
  if (!rightNumber && leftNumber->getValue() == 0.0)
    return new UnaryOperation(unmOp, right);

  return new Parenthesis(new BinaryOperation(subOp, left, right));
}
