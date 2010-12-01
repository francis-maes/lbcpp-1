
#ifndef LBCPP_NETWORK_SERVER_H_
# define LBCPP_NETWORK_SERVER_H_

# include "NetworkClient.h"

namespace lbcpp
{

class NetworkServer : public InterprocessConnectionServer
{
public:
  NetworkServer(ExecutionContext& context) : context(context) {}
  
  bool startServer(int port);
  void stopServer();

  NetworkClientPtr acceptClient(bool blocking);

  juce_UseDebuggingNewOperator

protected:
  /* InterprocessConnectionServer */
  virtual InterprocessConnection* createConnectionObject();

protected:
  enum {magicNumber = 0xdeadface};
  ExecutionContext& context;
  std::deque<NetworkClientPtr> acceptedClients;
  CriticalSection lock;
  
  NetworkClientPtr lockedPop();
};

typedef ReferenceCountedObjectPtr<NetworkServer> NetworkServerPtr;

}; /* namespace lbcpp */

#endif //!LBCPP_NETWORK_SERVER_H_
