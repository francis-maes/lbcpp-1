/*-----------------------------------------.---------------------------------.
| Filename: ExecutionContext.h             | Execution Context Base Class    |
| Author  : Francis Maes                   |                                 |
| Started : 24/11/2010 17:39               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_CONTEXT_H_
# define LBCPP_EXECUTION_CONTEXT_H_

# include "ExecutionCallback.h"

namespace lbcpp
{

class ExecutionContext : public CompositeExecutionCallback
{
public:
  ExecutionContext();

  virtual bool isMultiThread() const = 0;

  /*
  ** Checks
  */
#ifdef JUCE_DEBUG
  bool checkInheritance(TypePtr type, TypePtr baseType);
  bool checkInheritance(const Variable& variable, TypePtr baseType);
#else
  inline bool checkInheritance(TypePtr type, TypePtr baseType) {return true;}
  inline bool checkInheritance(const Variable& variable, TypePtr baseType) {return true;}
#endif // JUCE_DEBUG

  /*
  ** Current State
  */
  virtual bool isCanceled() const = 0;
  virtual bool isPaused() const = 0;

  ExecutionStackPtr getStack() const
    {return stack;}

  void setStack(const ExecutionStackPtr& stack)
    {this->stack = stack;}

  void enterScope(const String& description, const WorkUnitPtr& workUnit = WorkUnitPtr());
  void enterScope(const WorkUnitPtr& workUnit);
  void leaveScope(bool result = true);

  /*
  ** Work Units
  */
  virtual bool run(const WorkUnitPtr& workUnit, bool pushIntoStack = true);
  virtual bool run(const CompositeWorkUnitPtr& workUnits, bool pushIntoStack = true) = 0;

  // multi-thread
  virtual void pushWorkUnit(const WorkUnitPtr& workUnit)
    {jassert(isMultiThread());}
  virtual void waitUntilAllWorkUnitsAreDone()
    {jassert(isMultiThread());}

  /*
  ** Access to files
  */
  File getFile(const String& path);
  String getFilePath(const File& file) const;

  File getProjectDirectory() const
    {return projectDirectory;}

  void setProjectDirectory(const File& projectDirectory)
    {this->projectDirectory = projectDirectory;}

  lbcpp_UseDebuggingNewOperator

protected:
  friend class ExecutionContextClass;

  ExecutionStackPtr stack;
  File projectDirectory;
};

extern ExecutionContext& defaultExecutionContext();
extern void setDefaultExecutionContext(ExecutionContextPtr defaultContext);

extern ExecutionContextPtr singleThreadedExecutionContext();
extern ExecutionContextPtr multiThreadedExecutionContext(size_t numThreads);

extern ExecutionContextPtr defaultConsoleExecutionContext(bool noMultiThreading = false);

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
