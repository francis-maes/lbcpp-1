/*-----------------------------------------.---------------------------------.
| Filename: ExecutionTrace.h               | Stores an Execution Trace       |
| Author  : Francis Maes                   |                                 |
| Started : 02/12/2010 18:00               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_EXECUTION_TRACE_H_
# define LBCPP_EXECUTION_TRACE_H_

# include "ExecutionContext.h"
# include "WorkUnit.h"
# include "../Core/Vector.h"
# include "../Core/Pair.h"
# include "../Core/DynamicObject.h"

namespace lbcpp
{

class ExecutionTraceItem : public Object
{
public:
  ExecutionTraceItem(double time)
    : time(time) {}
  ExecutionTraceItem() {}

  virtual String getPreferedXmlTag() const = 0;
  virtual String getPreferedIcon() const = 0;

  double getTime() const
    {return time;}

  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  friend class ExecutionTraceItemClass;

  double time;
};

class MessageExecutionTraceItem : public ExecutionTraceItem
{
public:
  MessageExecutionTraceItem(double time, ExecutionMessageType messageType, const String& what, const String& where = String::empty);
  MessageExecutionTraceItem() {}

  virtual String toString() const;
  virtual String getPreferedXmlTag() const;
  virtual String getPreferedIcon() const;

  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  friend class MessageExecutionTraceItemClass;

  ExecutionMessageType messageType;
  String what;
  String where;
};

class ExecutionTraceNode : public ExecutionTraceItem
{
public:
  ExecutionTraceNode(const String& description, const WorkUnitPtr& workUnit, double startTime);
  ExecutionTraceNode() {}

  virtual String toString() const;
  virtual String getPreferedXmlTag() const
    {return T("node");}
  virtual String getPreferedIcon() const;

  /*
  ** Sub Items
  */
  void appendSubItem(const ExecutionTraceItemPtr& item);
  ExecutionTraceNodePtr findSubNode(const String& description, const WorkUnitPtr& workUnit) const;

  /*
  ** Results
  */
  void setResult(const String& name, const Variable& value);
  std::vector< std::pair<String, Variable> > getResults() const;

  ObjectPtr getResultsObject(ExecutionContext& context);

  /*
  ** Progression
  */
  void setProgression(const ProgressionStatePtr& progression)
    {this->progression = progression;}

  ProgressionStatePtr getProgression() const
    {return progression;}

  /*
  ** Time
  */
  void setEndTime(double endTime)
    {timeLength = endTime - time; jassert(timeLength >= 0.0);}

  double getEndTime() const
    {return time + timeLength;}

  double getTimeLength() const
    {return timeLength;}

  /*
  ** WorkUnit
  ** The work unit is only stored during execution
  ** After execution, the pointer is reset in order to save memory
  */
  const WorkUnitPtr& getWorkUnit() const
    {return workUnit;}

  void removeWorkUnit()
    {workUnit = WorkUnitPtr();}

  virtual void saveToXml(XmlExporter& exporter) const;
  void saveSubItemsToXml(XmlExporter& exporter) const;

protected:
  friend class ExecutionTraceNodeClass;

  String description;

  CriticalSection subItemsLock;
  std::vector<ExecutionTraceItemPtr> subItems;

  CriticalSection resultsLock;
  UnnamedDynamicClassPtr resultsClass;
  std::vector< std::pair<String, Variable> > results;

  ProgressionStatePtr progression;
  double timeLength;

  WorkUnitPtr workUnit;
};

typedef ReferenceCountedObjectPtr<ExecutionTraceNode> ExecutionTraceNodePtr;

class ExecutionTrace : public Object
{
public:
  ExecutionTrace(ExecutionContextPtr context);
  ExecutionTrace() {}

  virtual juce::Component* createComponent() const;

  ExecutionContext& getContext() const
    {jassert(context); return *context;}

  virtual String toString() const
    {return context->getClassName() + T(" Execution Trace");}

  ExecutionTraceNodePtr findNode(const ExecutionStackPtr& stack) const;

  Time getStartTime() const
    {return startTime;}

  virtual void saveToXml(XmlExporter& exporter) const;

protected:
  friend class ExecutionTraceClass;

  ExecutionContextPtr context;
  ExecutionTraceNodePtr root;
  
  Time startTime;
};

}; /* namespace lbcpp */

#endif //!LBCPP_EXECUTION_CONTEXT_H_
