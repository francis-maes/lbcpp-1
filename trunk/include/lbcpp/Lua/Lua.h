/*-----------------------------------------.---------------------------------.
| Filename: Lua.h                          | Lua Include File                |
| Author  : Francis Maes                   |                                 |
| Started : 23/06/2011 20:03               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_LUA_H_
# define LBCPP_LUA_H_

# include "../Core/predeclarations.h"

struct lua_State;
typedef struct lua_State lua_State;

namespace lbcpp
{

typedef int (*LuaFunction) (lua_State* L);

enum LuaType
{
  luaTypeNone = -1,
  luaTypeNil = 0,
  luaTypeBoolean,
  luaTypeLightUserData,
  luaTypeNumber,
  luaTypeString,
  luaTypeTable,
  luaTypeFunction,
  luaTypeUserData,
  luaTypeThread
};

class LuaState
{
public:
  LuaState(ExecutionContext& context, bool initializeLuaLibraries = true, bool initializeLBCppLibrary = true);
  LuaState(lua_State* L);
  virtual ~LuaState();

  operator lua_State*()
    {return L;}

  bool execute(const char* code, const char* codeName = "code");

  void createTable();
  void setTableField(const char *name, double value);

  void setGlobal(const char* name);

  void pushString(const char* value);
  void pushNumber(double value);
  void pushFunction(LuaFunction function);
  void pushVariable(const Variable& variable);
  void pushObject(ObjectPtr object);

  int getTop() const;

  LuaType getType(int index) const;

  bool isString(int index) const;

  bool checkBoolean(int index);
  double checkNumber(int index);
  const char* checkString(int index);
  ObjectPtr checkObject(int index, TypePtr expectedType);
  ObjectPtr checkObject(int index);
  Variable checkVariable(int index);
  
  bool newMetaTable(const char* name);
  
protected:
  lua_State* L;
  ExecutionContext& context;
  bool owned;
};

}; /* namespace lbcpp */

#endif // !LBCPP_LUA_H_
