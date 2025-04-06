#ifndef _LORAMESHER_ROUTE_PACKET_H
#define _LORAMESHER_ROUTE_PACKET_H

#include "PacketHeader.h"
#include "entities/routingTable/NetworkNode.h"
#include <IPAddress.h>

#pragma pack(1)
class RoutePacket final: public PacketHeader {
public:

    /**
     * @brief Node Role
     *
     */
    uint8_t nodeRole = 0;

    /**
     * @brief Broker IP Address
     */
    IPAddress brokerIP;

    /**
     * @brief BLE connection ID
     */
    uint8_t BLE_CONN_ID[6] = {0};  // BLE MAC (6 bytes)

    /**
     * @brief Network nodes
     *
     */
    NetworkNode networkNodes[];

    /**
     * @brief Get the Number of Network Nodes
     *
     * @return size_t Number of Network Nodes inside the packet
     */
    //size_t getNetworkNodesSize() { return (this->packetSize - sizeof(RoutePacket)) / sizeof(NetworkNode); }

    size_t getNetworkNodesSize() {
        // Correct: subtract only the fixed part (up to the FAM)
        size_t baseSize = sizeof(PacketHeader) + sizeof(nodeRole) + sizeof(brokerIP) + sizeof(BLE_CONN_ID);
        return (this->packetSize - baseSize) / sizeof(NetworkNode);
    }
};

#pragma pack()

#endif