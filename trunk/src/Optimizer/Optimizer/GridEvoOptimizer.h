/*-----------------------------------------.---------------------------------.
 | Filename: GridEvoOptimizer.h             | Optimizer using Evolutionary    |
 | Author  : Arnaud Schoofs                 | Algorithm on Grid               |
 | Started : 01/03/2010 23:45               |                                 |
 `------------------------------------------/                                 |
                                            |                                 |
                                            `--------------------------------*/

#ifndef GRID_EVO_OPTIMIZER_H_
#define GRID_EVO_OPTIMIZER_H_

// TODO use variable dynamically loaded (constructor or state e.g)
#define NB_WU_TO_EVALUATE 4
#define NB_MIN_WU_IN_PROGRESS 2
#define NB_WU_TO_UPDATE 2
#define RATIO_WU_USED_TO_UPDATE 1

# include <lbcpp/Optimizer/GridOptimizer.h>
# include <lbcpp/Network/NetworkClient.h>
# include "../src/Network/Node/ManagerNode/ManagerNodeNetworkInterface.h"
# include "../../Distribution/Builder/IndependentMultiVariateDistributionBuilder.h"

// TODO arnaud : move file to include directory ?
namespace lbcpp
{

class GridEvoOptimizerState : public GridOptimizerState {
public:
  GridEvoOptimizerState()
    {jassertfalse;} // TODO arnaud
  GridEvoOptimizerState(IndependentMultiVariateDistributionPtr distributions, IndependentMultiVariateDistributionBuilderPtr distributionsBuilder) : 
  distributions(distributions), distributionsBuilder(distributionsBuilder)
  {
    // TODO arnaud : clone init distribution ?
    totalNumberGeneratedWUs = 0;
    totalNumberEvaluatedWUs = 0;
  }
  
  virtual WorkUnitPtr generateSampleWU(ExecutionContext& context) const = 0;
  
protected:
  size_t totalNumberGeneratedWUs;
  size_t totalNumberEvaluatedWUs;
  
  std::vector<String> inProgressWUs;
  std::multimap<double, String> currentEvaluatedWUs;
  
  IndependentMultiVariateDistributionPtr distributions;
  IndependentMultiVariateDistributionBuilderPtr distributionsBuilder;
  
  friend class GridEvoOptimizerStateClass;
  friend class GridEvoOptimizer;
  
};
typedef ReferenceCountedObjectPtr<GridEvoOptimizerState> GridEvoOptimizerStatePtr;  
  
  
class GridEvoOptimizer : public GridOptimizer
{
public:
  virtual Variable optimize(ExecutionContext& context, const GridOptimizerStatePtr& state_, const FunctionPtr& getVariableFromTrace, const FunctionPtr& getScoreFromTrace) const
  {
    // TODO add callbacks infos
    // TODO add some state-saveToFile()
    
    
    GridEvoOptimizerStatePtr state = state_.dynamicCast<GridEvoOptimizerState>();
    
    while (state->totalNumberEvaluatedWUs < NB_WU_TO_EVALUATE) 
    {
      // don't stress the manager
      juce::Thread::sleep(10000);
      
      // Send WU's on network
      if (state->inProgressWUs.size() < NB_MIN_WU_IN_PROGRESS) 
      {
        // Establish network connection
        ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
        if (!interface)
          continue;
        
        // Send WU's
        while (state->inProgressWUs.size() < NB_MIN_WU_IN_PROGRESS) 
        {
          WorkUnitPtr wu = state->generateSampleWU(context);
          String res = sendWU(context, wu, interface);
          if (res == T("Error"))
          {
            context.errorCallback(T("SendWorkUnit::run"), T("Trouble - We didn't correclty receive the acknowledgement"));
            break;
          }
          context.resultCallback(T("WorkUnitIdentifier"), res);
          state->inProgressWUs.push_back(res);
          state->totalNumberGeneratedWUs++;
        }
        interface->closeCommunication();
        continue;
      }
      
      
      // don't stress the manager
      juce::Thread::sleep(10000);
      

      // handle finished WUs
      ManagerNodeNetworkInterfacePtr interface = getNetworkInterfaceAndConnect(context);
      if (!interface) 
        continue;
      
      std::vector<String>::iterator it;
      for(it = state->inProgressWUs.begin(); it != state->inProgressWUs.end(); )
      {
        if(interface->isFinished(*it))
        {
          NetworkResponsePtr res = interface->getExecutionTrace(*it);
          if (res)
          {  
            ExecutionTracePtr trace = interface->getExecutionTrace(*it)->getExecutionTrace(context);
            trace->saveToFile(context,File::getCurrentWorkingDirectory().getChildFile(String(*it) + T(".trace")));  // TODO arnaud : project directory ?
            double score = getScoreFromTrace->compute(context, trace).getDouble();
            state->currentEvaluatedWUs.insert(std::pair<double, String>(score,*it));
            state->totalNumberEvaluatedWUs++; // TODO arnaud : here or when updating distri ?
            state->inProgressWUs.erase(it);
          }
          else
            ++it;
        }
        else 
          ++it;
      }
      interface->closeCommunication();
      
      // enough WUs evaluated -> update distribution (with best results)
      if (state->currentEvaluatedWUs.size() >= NB_WU_TO_UPDATE) 
      {
        size_t nb = 0;
        std::multimap<double, String>::reverse_iterator it;
        // best results : use them then delete
        for (it = state->currentEvaluatedWUs.rbegin(); it != state->currentEvaluatedWUs.rend() && nb < state->currentEvaluatedWUs.size()/RATIO_WU_USED_TO_UPDATE; it++)
        {
          ExecutionTracePtr trace = Object::createFromFile(context, File::getCurrentWorkingDirectory().getChildFile(String((*it).second) + T(".trace"))).staticCast<ExecutionTrace>();
          state->distributionsBuilder->addElement(getVariableFromTrace->compute(context, trace));
          File::getCurrentWorkingDirectory().getChildFile(String((*it).second) + T(".trace")).deleteFile();
          nb++;
        }
        state->distributions = state->distributionsBuilder->build(context);
        state->distributionsBuilder->clear();
        
        // other results : delete them
        for ( ; it != state->currentEvaluatedWUs.rend(); it++) {
          File::getCurrentWorkingDirectory().getChildFile(String((*it).second) + T(".trace")).deleteFile();
        }
        
        state->currentEvaluatedWUs.clear(); // clear map
      }
    }
    
    return Variable();  // TODO arnaud : store best results ?
  }
    
protected:  
  friend class GridEvoOptimizerClass;
  
private:
  ManagerNodeNetworkInterfacePtr getNetworkInterfaceAndConnect(ExecutionContext& context) const
  {       
    // TODO arnaud : move outside:
    String projectName(T("BoincFirstStage"));
    String source(T("boincadm@boinc.run"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    size_t managerPort = 1664;
    //size_t requiredCpus = 1;
    //size_t requiredMemory = 2;
    //size_t requiredTime = 10;
    
    NetworkClientPtr client = blockingNetworkClient(context);
    if (!client->startClient(managerHostName, managerPort))
    {
      context.errorCallback(T("SendWorkUnit::run"), T("Not connected !"));
      return NULL;
    }
    context.informationCallback(managerHostName, T("Connected !"));
    ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, source);
    interface->sendInterfaceClass();
    return interface;
  }
  
  String sendWU(ExecutionContext& context, WorkUnitPtr wu, ManagerNodeNetworkInterfacePtr interface) const
  {    
    // TODO arnaud : move outside:
    String projectName(T("BoincFirstStage"));
    String source(T("boincadm@boinc.run"));
    String destination(T("boincadm@boinc.run"));
    String managerHostName(T("monster24.montefiore.ulg.ac.be"));
    //size_t managerPort = 1664;
    size_t requiredCpus = 1;
    size_t requiredMemory = 2;
    size_t requiredTime = 10;
    
    NetworkRequestPtr request = new NetworkRequest(context, projectName, source, destination, wu, requiredCpus, requiredMemory, requiredTime);
    return interface->pushWorkUnit(request);
  }
};
  
typedef ReferenceCountedObjectPtr<GridEvoOptimizer> GridEvoOptimizerPtr;

}; /* namespace lbcpp */

#endif // !GRID_EVO_OPTIMIZER_H_