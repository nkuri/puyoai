#ifndef CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_
#define CORE_SERVER_CONNECTOR_CONNECTOR_MANAGER_H_

#include <string>
#include <vector>

class Connector;
struct FrameResponse;

class ConnectorManager {
public:
    // Receives decision and messages from clients.
    // Returns false when disconnected.
    virtual bool receive(int frameId, std::vector<FrameResponse> data[2]) = 0;

    virtual Connector* connector(int i) = 0;
    virtual void setWaitTimeout(bool) = 0;
};

#endif
