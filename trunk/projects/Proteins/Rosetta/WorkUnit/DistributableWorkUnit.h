/*-----------------------------------------.---------------------------------.
| Filename: DistributableWorkUnit.h        | Distributable WorkUnit          |
| Author  : Alejandro Marcos Alvarez       |                                 |
| Started : Dec 21, 2011  3:02:51 PM       |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/

#ifndef LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_
# define LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_

# include <lbcpp/Execution/WorkUnit.h>

namespace lbcpp
{

class DistributableExecutionContextCallback : public ExecutionContextCallback
{
public:
  DistributableExecutionContextCallback(ExecutionContext& context)
    : context(context) {}

  virtual void workUnitFinished(const WorkUnitPtr& workUnit, const Variable& result)
  {
    //TODO save result
    context.informationCallback(T("TestExecutionContextCallback::workUnitFinished"), result.toString());
  }

  static Variable gatherResults()
  {

  }

protected:
  ExecutionContext& context;
};

typedef ReferenceCountedObjectPtr<DistributableExecutionContextCallback> DistributableExecutionContextCallbackPtr;

class TestDumbWorkUnit : public WorkUnit
{
public:
  TestDumbWorkUnit() {}
  TestDumbWorkUnit(size_t j) : j(j) {}

  virtual Variable run(ExecutionContext& context)
  {
    context.enterScope(T("DumbWorkUnit ") + String((int)j));

    juce::RelativeTime t1 = juce::RelativeTime::milliseconds(juce::Time::currentTimeMillis());

    RandomGeneratorPtr random = new RandomGenerator((juce::uint32)j);
    DenseDoubleVectorPtr values = new DenseDoubleVector(100, -1);

    for (size_t i = 0; i < 100; ++i)
    {
      values->setValue(i, (double)random->sampleInt(0, 100));
      if ((i + 1) % 10 == 0)
        context.progressCallback(new ProgressionState(i + 1, 100, T("It")));
      juce::Thread::sleep(context.getRandomGenerator()->sampleInt(10, 30));
    }

    juce::RelativeTime t2 = juce::RelativeTime::milliseconds(juce::Time::currentTimeMillis());
    juce::RelativeTime t3 = t2 - t1;
    double elapsed = t3.inSeconds();

    context.leaveScope(Variable(elapsed));
    return Variable(values);
  }

protected:
  friend class TestDumbWorkUnitClass;

  size_t j;
};

class DistributableWorkUnit : public WorkUnit
{
public:
  DistributableWorkUnit() : name(T("DistributableWorkUnit")) {}
  DistributableWorkUnit(String name) : name(name) {}

  /**
   * Each subworkunit should open a scope for itself to avoid undesired effects
   * in traces.
   *
   * workUnits = new CompositeWorkUnit(T("blabla"));
   * workUnits->addWorkUnit(new WorkUnit());
   *
   */
  virtual void initializeWorkUnits(ExecutionContext& context)=0;

  /**
   * Manage the VariableVector output by run.
   * Should enter its own scope.
   */
  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
    {return Variable(result);}

  /**
   * Manage the VariableVector output by run.
   * Should enter its own scope.
   */
  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
    {return Variable();}

  virtual CompositeWorkUnitPtr getWorkUnits()
    {return workUnits;}

  virtual Variable run(ExecutionContext& context)
  {
    // initialization
    context.enterScope(T("Initializing distributable work unit::local"));
    initializeWorkUnits(context);
    context.leaveScope();
    if (workUnits->getNumWorkUnits() == 0)
    {
      context.informationCallback(T("No work units to treat."));
      return Variable();
    }

    // computing time
    context.enterScope(name);
    context.informationCallback(T("Treating ") + String((int)workUnits->getNumWorkUnits())
        + T(" work units."));
    Variable result = context.run(workUnits, true);
    context.leaveScope();

    // show results
    context.enterScope(T("Results"));
    for (size_t i = 0; i < workUnits->getNumWorkUnits(); i++)
    {
      context.enterScope(workUnits->getWorkUnit(i));
      context.leaveScope(singleResultCallback(context,
          result.getObjectAndCast<VariableVector> ()->getElement(i)));
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("Managing results"));
    Variable gatheredResult = multipleResultCallback(context, *result.getObjectAndCast<
        VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("local"), T("Distributable work unit done."));

    return gatheredResult;
  }

  String getName()
    {return name;}

protected:
  friend class DistributableWorkUnitClass;

  String name;
  CompositeWorkUnitPtr workUnits;
};

typedef ReferenceCountedObjectPtr<DistributableWorkUnit> DistributableWorkUnitPtr;
extern ClassPtr distributableWorkUnitClass;

class TestDistribWorkUnit : public DistributableWorkUnit
{
public:
  TestDistribWorkUnit() : DistributableWorkUnit() {}

  virtual void initializeWorkUnits(ExecutionContext& context)
  {
    workUnits = new CompositeWorkUnit(name);
    for (size_t i = 0; i < 10; i++)
      workUnits->addWorkUnit(new TestDumbWorkUnit(i));
  }

  virtual Variable singleResultCallback(ExecutionContext& context, Variable& result)
  {
    size_t numElements = result.getObjectAndCast<DenseDoubleVector> ()->getNumValues();

    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Value"), Variable(
          result.getObjectAndCast<DenseDoubleVector> ()->getValue(i)));
      context.leaveScope();
    }

    return (result);
  }

  virtual Variable multipleResultCallback(ExecutionContext& context, VariableVector& results)
  {
    //    double j = 0;
    //
    //    for (size_t i = 0; i < results.getNumElements(); i++)
    //      j += results.getElement(i).getDouble();
    //    j /= (double)results.getNumElements();
    //
    //    context.informationCallback(T("Average is ") + String(j));
    size_t numElements =
        results.getElement(0).getObjectAndCast<DenseDoubleVector> ()->getNumValues();
    size_t numResults = results.getNumElements();

    DenseDoubleVectorPtr mean = new DenseDoubleVector(numElements, -1);

    for (size_t j = 0; j < numResults; j++)
      for (size_t i = 0; i < numElements; i++)
        mean->setValue(i, mean->getValue(i) + results.getElement(j).getObjectAndCast<
            DenseDoubleVector> ()->getValue(i) / (double)numResults);

    for (size_t i = 0; i < numElements; i++)
    {
      context.enterScope(T("Result"));
      context.resultCallback(T("Step"), Variable((int)i));
      context.resultCallback(T("Value"), Variable(mean->getValue(i)));
      context.leaveScope();
    }

    return Variable(mean);
  }

protected:
  friend class TestDistribWorkUnitClass;
};

extern DistributableWorkUnitPtr testDistribWorkUnit();

class DistributeToClusterWorkUnit : public WorkUnit
{
public:
  virtual Variable run(ExecutionContext& context)
  {
    ExecutionContextPtr remoteContext = distributedExecutionContext(context, remoteHostName,
        remoteHostPort, distributable->getName(), localHostLogin, clusterLogin,
        fixedResourceEstimator(1, memory, time), true);

    // initialization
    context.enterScope(T("Initializing distributable work unit::distributed"));
    distributable->initializeWorkUnits(context);
    context.leaveScope();
    CompositeWorkUnitPtr units = distributable->getWorkUnits();
    if (units->getNumWorkUnits() == 0)
    {
      context.informationCallback(T("No work units to treat."));
      return Variable();
    }

    // computing time
    context.enterScope(distributable->getName());
    context.informationCallback(T("Treating ") + String((int)units->getNumWorkUnits())
        + T(" work units."));
    Variable result = remoteContext->run(units);
    context.leaveScope();

    // show results
    context.enterScope(T("Results"));
    for (size_t i = 0; i < distributable->getWorkUnits()->getNumWorkUnits(); i++)
    {
      context.enterScope(distributable->getWorkUnits()->getWorkUnit(i));
      context.leaveScope(distributable->singleResultCallback(context, result.getObjectAndCast<
          VariableVector> ()->getElement(i)));
    }
    context.leaveScope();

    // managing results
    context.enterScope(T("Managing results"));
    Variable gatheredResult = distributable->multipleResultCallback(context,
        *result.getObjectAndCast<VariableVector> ());
    context.leaveScope(gatheredResult);

    context.informationCallback(T("cluster"), T("Distributable work unit done."));

    return gatheredResult;
  }

protected:
  friend class DistributeToClusterWorkUnitClass;

  String clusterLogin; // nic3
  size_t memory; // in MB
  size_t time; // in hours
  DistributableWorkUnitPtr distributable;

  String localHostLogin; // me
  String remoteHostName; // monster
  size_t remoteHostPort; // 1664
};

}; /* namespace lbcpp */

#endif //! LBCPP_PROTEINS_ROSETTA_WORKUNIT_DISTRIBUTABLEWORKUNIT_H_