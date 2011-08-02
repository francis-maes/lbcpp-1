/*-----------------------------------------.---------------------------------.
| Filename: Rewriter.h                     | Base class for writing rewriters|
| Author  : Francis Maes                   |                                 |
| Started : 22/07/2011 04:38               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_REWRITER_H_
# define LBCPP_LUA_REWRITER_H_

# include "Visitor.h"

namespace lbcpp {
namespace lua {

class Rewriter : public Visitor
{
public:
  Rewriter(ExecutionContextPtr context = ExecutionContextPtr())
    : Visitor(context) {}

  NodePtr rewrite(const NodePtr& node)
    {NodePtr res = node; accept(res); return res;}

protected:
  NodePtr result;

  void setResult(NodePtr result)
    {this->result = result;}

  virtual void accept(NodePtr& node)
  {
    NodePtr prevResult = result;
    result = NodePtr();
    node->accept(*this);
    if (result)
    {
      if (!result->getScope() && node->getScope())
        result->setScope(node->getScope()); // forward scope information
      node = result;
    }
    result = prevResult;
  }
};

// A fake node that can be returned to remove a statement from a block
class EmptyNode : public Node
{
public:
  virtual String getTag() const
    {return "Empty";}

  virtual LuaChunkType getType() const
    {return luaOtherChunk;}

  virtual size_t getNumSubNodes() const
    {return 0;}

  virtual NodePtr& getSubNode(size_t index)
    {jassert(false); return *(NodePtr* )0;}

  virtual void accept(Visitor& visitor)
    {jassert(false);}
};

class DefaultRewriter : public DefaultVisitorT<Rewriter>
{
public:
  DefaultRewriter(ExecutionContextPtr context = ExecutionContextPtr())
    : DefaultVisitorT<Rewriter>(context) {}

  virtual void visit(Block& block)
  {
    size_t n = block.getNumSubNodes();
    NodePtr prevResult = result;
    bool hasModifications = false;
    for (size_t i = 0; i < n; ++i)
    {
      result = NodePtr();
      NodePtr& subNode = block.getSubNode(i);
      if (!subNode)
        continue; // skip non existing sub trees
      subNode->accept(*this);
      if (result)
      {
        subNode = result;
        hasModifications = true;
      }
    }
    result = prevResult;

    // flatten blocks
    if (hasModifications)
    {
      std::vector<StatementPtr> statements;
      fillStatementsRecursively(&block, statements);
      block.setStatements(statements);
    }
  }

private:
  void fillStatementsRecursively(const BlockPtr& block, std::vector<StatementPtr>& res)
  {
    size_t n = block->getNumSubNodes();
    for (size_t i = 0; i < n; ++i)
    {
      NodePtr node = block->getSubNode(i);
      BlockPtr subBlock = node.dynamicCast<Block>();
      if (subBlock)
        fillStatementsRecursively(subBlock, res);
      else
      {
        StatementPtr statement = node.dynamicCast<Statement>();
        if (statement)
          res.push_back(statement);
        else
          jassert(node.isInstanceOf<EmptyNode>());
      }
    }
  }
};

}; /* namespace lua */
}; /* namespace lbcpp */

#endif // !LBCPP_LUA_REWRITER_H_

