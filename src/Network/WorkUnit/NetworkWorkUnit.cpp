/*-----------------------------------------.---------------------------------.
| Filename: NetworkWorkUnit.cpp            | Network Work Unit               |
| Author  : Julien Becker                  |                                 |
| Started : 01/02/2011 19:57               |                                 |
`------------------------------------------/                                 |
                               |                                             |
                               `--------------------------------------------*/
#include "precompiled.h"
#include "../Node/NodeNetworkNotification.h"
#include "../Node/NetworkRequest.h"
#include "NetworkWorkUnit.h"
#include <lbcpp/Network/NetworkServer.h>
#include <algorithm>

using namespace lbcpp;

/*
** ManagerWorkUnit
*/

Variable ManagerWorkUnit::run(ExecutionContext& context)
{
  fileManager = new NetworkProjectFileManager(context);
  NetworkServerPtr server = new NetworkServer(context);
  if (!server->startServer(port))
  {
    context.errorCallback(T("WorkUnitManagerServer::run"), T("Not able to open port ") + String((int)port));
    return false;
  }
  
  while (true)
  {
    /* Accept client */
    NetworkClientPtr client = server->acceptClient(INT_MAX);
    if (!client)
      continue;

    String connectedHostName = client->getConnectedHostName();
    context.informationCallback(connectedHostName, T("Connected"));
    
    /* Which kind of connection ? */
    NodeNetworkInterfacePtr remoteInterface;
    if (!client->receiveObject<NodeNetworkInterface>(300000, remoteInterface) || !remoteInterface)
    {
      context.warningCallback(connectedHostName, T("Unknown NodeNetworkInterface - Need to update Manager and/or Client? Invalid NetworkInterface?"));
      client->stopClient();
      continue;
    }

    context.informationCallback(connectedHostName, T("Node name: ") + remoteInterface->getNodeName());
    context.informationCallback(connectedHostName, T("Interface: ") + remoteInterface->getClassName());

    ClassPtr type = remoteInterface->getClass();
    /* Strat communication (depending of the type) */
    NodeNetworkInterfacePtr interface;
    if (type->inheritsFrom(clientManagerNodeNetworkInterfaceClass))
    {
      interface = new FileSystemManagerNodeNetworkInterface(context, client, T("Manager"), fileManager);
      serverCommunication(context, interface);
    }
    else if (type->inheritsFrom(gridNodeNetworkInterfaceClass))
    {
      interface = new ClientGridNodeNetworkInterface(context, client, remoteInterface->getNodeName());
      clientCommunication(context, interface);
    }
    else
    {
      context.warningCallback(connectedHostName, T("Unknown NodeNetworkInterface - No communication protocol specified for this interface"));
      client->stopClient();
      continue;
    }
    
    /* Terminate the connection */
    interface->closeCommunication();
    context.informationCallback(connectedHostName, T("Disconnected"));
  }
}

void ManagerWorkUnit::serverCommunication(ExecutionContext& context, ManagerNodeNetworkInterfacePtr interface) const
{
  NetworkClientPtr client = interface->getNetworkClient();
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NotificationPtr notification;
    if (!client->receiveObject<Notification>(300000, notification) || !notification)
    {
      context.warningCallback(T("NetworkContext::run"), T("No notification received"));
      return;
    }
    notification->notify(interface);
  }
}
  
void ManagerWorkUnit::clientCommunication(ExecutionContext& context, GridNodeNetworkInterfacePtr interface)
{
  String nodeName = interface->getNodeName();
  if (nodeName == String::empty)
  {
    interface->getContext().warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("Fail - Empty node name"));
    return;
  }

  /* Send new requests */
  std::vector<NetworkRequestPtr> waitingRequests;
  fileManager->getWaitingRequests(nodeName, waitingRequests);
  if (waitingRequests.size())
  {
    size_t maxblocklength = 200;
    for (size_t i = 0; i < ceil(waitingRequests.size()/(double) maxblocklength)-1; ++i) {
      size_t nbElements = std::min((i+1)*maxblocklength, waitingRequests.size()) - i*maxblocklength;
      ObjectVectorPtr vec = objectVector(networkRequestClass, nbElements);
      for (size_t x = i*maxblocklength; x < i*maxblocklength+nbElements ; ++x)
        vec->set(x-i*maxblocklength, waitingRequests[x]);
      ContainerPtr results = interface->pushWorkUnits(vec);
      if (!results || results->getNumElements() != nbElements)
      {
        context.warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("PushWorkUnits - No acknowledgement received."));
        std::vector<NetworkRequestPtr> errorRequests(waitingRequests.begin()+i*maxblocklength, waitingRequests.begin()+i*maxblocklength+nbElements);        
        fileManager->setAsWaitingRequests(errorRequests);
        //return; // TODO arnaud
      }
      else
      {
        size_t n = results->getNumElements();
        for (size_t x = 0; x < n; ++x)
        {
          String result = results->getElement(x).getString();
          if (result == T("Error"))
            fileManager->crachedRequest(waitingRequests[i*maxblocklength+x]);
        }
      }
    }
  }

  /* Get trace */
  ContainerPtr networkResponses = interface->getFinishedExecutionTraces();
  if (!networkResponses)
    return;
  
  size_t n = networkResponses->getNumElements();
  for (size_t i = 0; i < n; ++i)
  {
    NetworkResponsePtr response = networkResponses->getElement(i).getObjectAndCast<NetworkResponse>();
    if (!response)
    {
      context.warningCallback(interface->getNetworkClient()->getConnectedHostName(), T("GetFinishedExecutionTraces - NetworkResponsePtr()"));
      continue;
    }
    NetworkRequestPtr request = fileManager->getRequest(response->getIdentifier());
    if (!request)
      continue;
    fileManager->archiveRequest(new NetworkArchive(request, response));
  }
}

/*
** GridWorkUnit
*/

Variable GridWorkUnit::run(ExecutionContext& context)
{
  if (gridEngine != T("SGE") && gridEngine != T("BOINC"))
    return false;
  
  /* Establishing a connection */
  NetworkClientPtr client = blockingNetworkClient(context, 3);
  if (!client->startClient(hostName, port))
  {
    context.warningCallback(T("NetworkContext::run"), T("Connection to ") + hostName.quoted() + (" fail !"));
    client->stopClient();
    return Variable();
  }
  context.informationCallback(T("NetworkContext::run"), T("Connected to ") + hostName + T(":") + String((int)port));
  
  NodeNetworkInterfacePtr interface;
  if (gridEngine == T("SGE"))
    interface = new SgeGridNodeNetworkInterface(context, client, gridName);
  else if (gridEngine == T("BOINC"))
    interface = new BoincGridNodeNetworkInterface(context, client, gridName);
  else
  {
    jassertfalse;
    return Variable();
  }
  
  /* Slave mode - Execute received commands */
  interface->sendInterfaceClass();
  while (client->isConnected() || client->hasVariableInQueue())
  {
    NotificationPtr notification;
    if (!client->receiveObject<Notification>(300000, notification) || !notification)
    {
      context.warningCallback(T("NetworkContext::run"), T("No notification received"));
      return false;
    }
    notification->notify(interface);
  }
  return true;
}

/*
** DumbWorkUnit
*/

Variable DumbWorkUnit::run(ExecutionContext& context)
{
  for (size_t i = 0; i < 10; ++i)
  {
    context.progressCallback(new ProgressionState(i+1, 10, T("DumbWorkUnit")));
    juce::Thread::sleep(1000);
  }
  return Variable();
}

/*
** GetTraceWorkUnit
*/
Variable GetTraceWorkUnit::run(ExecutionContext& context)
{
  NetworkClientPtr client = blockingNetworkClient(context);
  if (!client->startClient(hostName, port))
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("Not connected !"));
    return false;
  }
  context.informationCallback(hostName, T("Connected !"));
  
  ManagerNodeNetworkInterfacePtr interface = new ClientManagerNodeNetworkInterface(context, client, T("Client"));
  interface->sendInterfaceClass();
  
  if (!interface->isFinished(workUnitIdentifier))
  {
    context.informationCallback(T("GetTraceWorkUnit::run"), T("The Work Unit (") + workUnitIdentifier + T(") is not finished !"));
    return true;
  }
  
  NetworkResponsePtr res = interface->getExecutionTrace(workUnitIdentifier);
  if (!res)
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("No NetworkResponse received !"));
    return false;
  }

  ExecutionTracePtr trace = res->getExecutionTrace(context);
  if (!trace)
  {
    context.errorCallback(T("GetTraceWorkUnit::run"), T("No ExecutionTrace available ! Maybe due to a crash on the grid."));
    return false;
  }
  
  File dst = context.getFile(workUnitIdentifier + T(".trace"));
  trace->saveToFile(context, dst);
  context.informationCallback(T("The trace has been saved to ") + dst.getFullPathName().quoted());
  
  interface->closeCommunication();
  return true;
}