
-- note: includes gg/mlp Lua parsing Libraries taken from Metalua.
require "Language.lexer"
require "Language.gg"
require "Language.mlp_lexer"
require "Language.mlp_misc"
require "Language.mlp_table"
require "Language.mlp_meta"
require "Language.mlp_expr"
require "Language.mlp_stat"
require "Language.mlp_ext"
mlc = {} -- make gg happy
local mlp = assert(_G.mlp)

module ("LuaChunk", package.seeall)

_M.parseExpression = mlp.expr
_M.parseStatement = mlp.stat
_M.parseStatementBlock = mlp.block

function _M.stringLexer(src, filename)
  return mlp.lexer:newstream(src, filename)
end

function _M.fileLexer(filename)
  -- FIXME return mlp.lexer:newstream(src, filename)
end


local function convertUnaryOp(opid)
  opids = {['not'] = 0, len = 1, unm = 2}
  return opids[opid]
end

local function convertBinaryOp(opid)
  opids = {add = 0, sub = 1, mul = 2, div = 3,
           mod = 4, pow = 5, concat = 6, eq = 7,
           lt = 8, le = 9, ['and'] = 10, ['or'] = 11}
  return opids[opid]
end

local function convertLineInfo(lineinfo) -- not used yet, to be finished
  local comments = (lineinfo.comments and lineinfo.comments or "")
  return lbcpp.Object.create("lua::LineInfo", lineinfo[1], lineinfo[2], lineinfo[4], comments)
end

local function makeObjectVector(class, inputTable, startIndex)
  local res = lbcpp.Object.create('ObjectVector<' .. class .. '>')
  for i=startIndex,#inputTable do
    res:append(inputTable[i])
  end
  return res
end

local function createBlock(statements)

  local function ensureIsStatement(node)
    local cl = node.className
    if cl == "lua::Call" or cl == "lua::Invoke" then
      return lbcpp.Object.create("lua::ExpressionStatement", node)
    else
      return node
    end
  end

  if type(statements) == 'table' then
    for i=1,#statements do
      statements[i] = ensureIsStatement(statements[i])
    end
  else 
    local tmp = {}
    for i=1,statements:size() do
      table.insert(tmp, ensureIsStatement(statements:get(i - 1)))
    end
    statements = tmp
  end
  return lbcpp.Object.create("lua::Block", makeObjectVector("lua::Statement", statements, 1))
end

function _M.metaLuaAstToLbcppAst(ast)

  local function convertSubNodes(ast)
    local res = {}
    for i=1,#ast do
      if type(ast[i]) == "table" then
        table.insert(res, _M.metaLuaAstToLbcppAst(ast[i]))
      end
    end
    return res
  end


  local function makeBlock(subAst)
    return createBlock(convertSubNodes(subAst))
  end

  local function getTag(ast)
    return ast.tag and ast.tag or "List"
  end

  if ast==nil then
    return nil
  end

  -- convert sub nodes  
  local subNodes = convertSubNodes(ast)
   
  -- create this node
  if ast.tag == nil then
    return lbcpp.Object.create("lua::List", makeObjectVector("lua::Node", subNodes, 1))
  end

  --print ("tag: " .. ast.tag .. " num sub nodes = " .. (#subNodes))

  -- statements
  if ast.tag == "Do" then
    return lbcpp.Object.create("lua::" .. ast.tag, subNodes[1])
  elseif ast.tag == "Set" then
    return lbcpp.Object.create("lua::Set", subNodes[1], subNodes[2])
  elseif ast.tag == "While" then
    --print ("While: num sub nodes " .. #subNodes .. " subtags = " .. getTag(ast[1]) .. " " .. getTag(ast[2]))
    return lbcpp.Object.create("lua::While", subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Repeat" then
    return lbcpp.Object.create("lua::Repeat", makeBlock(ast[1]), subNodes[2])
  elseif ast.tag == "If" then
    local conditions = lbcpp.Object.create("ObjectVector<lua::Expression>")
    local blocks = lbcpp.Object.create("ObjectVector<lua::Block>")
    local i = 1
    assert (#subNodes == #ast)
    while i <= #subNodes do
      if i < #subNodes then
        conditions:append(subNodes[i])
        blocks:append(makeBlock(ast[i + 1]))
      else
        blocks:append(makeBlock(ast[i]))
      end
      i = i + 2
    end
    return lbcpp.Object.create("lua::If", conditions, blocks)
  elseif ast.tag == "ForNum" then
    if #subNodes == 5 then
      return lbcpp.Object.create("lua::ForNum", subNodes[1], subNodes[2], subNodes[3], subNodes[4], makeBlock(ast[5]))
    else
      assert (#subNodes == 4)
      return lbcpp.Object.create("lua::ForNum", subNodes[1], subNodes[2], subNodes[3], nil, makeBlock(ast[4]))
    end
  elseif ast.tag == "ForIn" then
    return lbcpp.Object.create("lua::ForIn", subNodes[1], subNodes[2], makeBlock(ast[3]))
  elseif ast.tag == "Local" then
    if #subNodes == 2 then
      return lbcpp.Object.create("lua::Local", subNodes[1], subNodes[2])
    else
      assert(#subNodes == 1)
      return lbcpp.Object.create("lua::Local", subNodes[1])
    end
  elseif ast.tag == "Localrec" then
    assert (#subNodes == 2)
    return lbcpp.Object.create("lua::Local", subNodes[1], subNodes[2], true)
  elseif ast.tag == "Break" then
    return lbcpp.Object.create("lua::Break");
  elseif ast.tag == "Return" then
    return lbcpp.Object.create("lua::Return", makeObjectVector("lua::Expression", subNodes, 1))
  elseif ast.tag == "Parameter" then
    return lbcpp.Object.create("lua::Parameter", subNodes[1], subNodes[2])
  
  -- expressions  
  elseif ast.tag == "Nil" or ast.tag == "Dots" then
    return lbcpp.Object.create("lua::" .. ast.tag)
  elseif ast.tag == "False" or ast.tag == "True" then
    return lbcpp.Object.create("lua::LiteralBoolean", ast.tag == "True")
  elseif ast.tag == "Id" then
    return lbcpp.Object.create("lua::Identifier", ast[1])
  elseif ast.tag == "Derivable" then
    return lbcpp.Object.create("lua::Identifier", ast[1][1], true)
  elseif ast.tag == "Number" or ast.tag == "String" then
    return lbcpp.Object.create("lua::Literal" .. ast.tag, ast[1])
  elseif ast.tag == "Function" then
    return lbcpp.Object.create("lua::Function", subNodes[1], makeBlock(ast[2]))
  elseif ast.tag == "Table" then
    return lbcpp.Object.create("lua::Table", makeObjectVector("lua::Expression", subNodes, 1))
  elseif ast.tag == "Pair" then
    return lbcpp.Object.create("lua::Pair", subNodes[1], subNodes[2])
  elseif ast.tag == "Op" then
    if #subNodes == 2 then
      return lbcpp.Object.create("lua::BinaryOperation", convertBinaryOp(ast[1]), subNodes[1], subNodes[2])
    elseif #subNodes == 1 then
      return lbcpp.Object.create("lua::UnaryOperation", convertUnaryOp(ast[1]), subNodes[1])
    else
      error("invalid number of operands: " .. #subNodes)
    end
  elseif ast.tag == "Paren" then
    return lbcpp.Object.create("lua::Parenthesis", subNodes[1])
  elseif ast.tag == "Call" then
    return lbcpp.Object.create("lua::Call", subNodes[1], makeObjectVector("lua::Expression", subNodes, 2))
  elseif ast.tag == "Invoke" then
    return lbcpp.Object.create("lua::Invoke", subNodes[1], subNodes[2], makeObjectVector("lua::Expression", subNodes, 3))
  elseif ast.tag == "Index" then
    return lbcpp.Object.create("lua::Index", subNodes[1], subNodes[2])
  elseif ast.tag == "Subspecified" then
    return lbcpp.Object.create("lua::Subspecified", subNodes[1])
  else
    error("unknown tag " .. ast.tag .. " (numSubNodes = " .. #subNodes .. " numAttributes = " .. (#ast - #subNodes) .. ")")
  end
end

function _M.parse(codeType, lexer)
  if (codeType == 0) then
    return _M.metaLuaAstToLbcppAst(_M.parseExpression(lexer))
  elseif (codeType == 1) then
    return _M.metaLuaAstToLbcppAst(_M.parseStatement(lexer))
  elseif (codeType == 2) then
    -- transform result into block
    local list = _M.metaLuaAstToLbcppAst(_M.parseStatementBlock(lexer))
    return createBlock(list.nodes)
  else
    error("Unknown code type")
  end
end

function _M.parseFromString(codeType, code, codeName)
  local lexer = _M.stringLexer(code, codeName)
  return _M.parse(codeType, lexer)
end

-- 'derivable' extension
mlp.lexer:add{ "derivable" }
mlp.func_param_content:add{ "derivable", mlp.id, builder = "Derivable" }

-- 'subspecified' extension
mlp.lexer:add{ "subspecified", "parameter" }
mlp.expr:add{ "subspecified", mlp.expr, builder = "Subspecified" }
mlp.stat:add{ "parameter", mlp.id, "=", mlp.table, builder = "Parameter" }
   
local function subspecified_funcdef_builder(x)
   local name, method, func = x[1], x[2], x[3]
   if method then 
      name = { tag="Index", name, method, lineinfo = {
         first = name.lineinfo.first,
         last  = method.lineinfo.last } }
      _G.table.insert (func[1], 1, {tag="Id", "self"}) 
   end
   func = {tag = "Subspecified", func}
   local r = { tag="Set", {name}, {func} } 
   r[1].lineinfo = name.lineinfo
   r[2].lineinfo = func.lineinfo
   return r
end 

mlp.stat:add{"subspecified", "function", mlp.func_name, mlp.method_name, mlp.func_val, builder=subspecified_funcdef_builder }



-- error handler
function errorHandler(msg)
  context:error(msg)
  return "toto"
end