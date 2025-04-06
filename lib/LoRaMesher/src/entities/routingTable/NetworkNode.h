#ifndef _LORAMESHER_NETWORK_NODE_H
#define _LORAMESHER_NETWORK_NODE_H

#include<IPAddress.h>

#pragma pack(1)

/**
 * @brief Network Node
 *
 */
class NetworkNode {
public:
    /**
     * @brief Address
     *
     */
    uint16_t address = 0;

    /**
     * @brief Metric, how many hops to reach the previous address
     *
     */
    uint8_t metric = 0;

    /**
     * @brief Role of the Node
     *
     */
    uint8_t role = 0;

    /**
     * @brief IP address of the MQTT broker associated with this node
     */
    IPAddress mqttBrokerIP = IPAddress(0, 0, 0, 0);

    /**
     * @brief IP address of the MQTT broker associated with this node
     */
    uint8_t BLE_CONN_ID[6] = {0};  // initialize with zeros

    NetworkNode() {};

    // With address and metric (Standard)
    NetworkNode(uint16_t address_, uint8_t metric_, uint8_t role_): address(address_), metric(metric_), role(role_) {};

    // With IP address (optional BLE conn ID)
    NetworkNode(uint16_t address_, uint8_t metric_, uint8_t role_, IPAddress brokerIP)
        : address(address_), metric(metric_), role(role_), mqttBrokerIP(brokerIP) {}

    // With BLE conn ID and IP address 
    NetworkNode(uint16_t address_, uint8_t metric_, uint8_t role_, IPAddress brokerIP, const uint8_t bleConnId_[6])
        : address(address_), metric(metric_), role(role_), mqttBrokerIP(brokerIP) {
        memcpy(BLE_CONN_ID, bleConnId_, 6);  // copy MAC address into class field
    }
};

#pragma pack()

#endif