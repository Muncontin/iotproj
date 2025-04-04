#include "RoutingTableService.h"

size_t RoutingTableService::routingTableSize() {
    return routingTableList->getLength();
}

RouteNode* RoutingTableService::findNode(uint16_t address) {
    routingTableList->setInUse();

    if (routingTableList->moveToStart()) {
        do {
            RouteNode* node = routingTableList->getCurrent();

            if (node->networkNode.address == address) {
                routingTableList->releaseInUse();
                return node;
            }

        } while (routingTableList->next());
    }

    routingTableList->releaseInUse();
    return nullptr;
}

RouteNode* RoutingTableService::getBestNodeByRole(uint8_t role) {
    RouteNode* bestNode = nullptr;

    routingTableList->setInUse();

    if (routingTableList->moveToStart()) {
        do {
            RouteNode* node = routingTableList->getCurrent();

            if ((node->networkNode.role & role) == role &&
                (bestNode == nullptr || node->networkNode.metric < bestNode->networkNode.metric)) {
                bestNode = node;
            }

        } while (routingTableList->next());
    }

    routingTableList->releaseInUse();
    return bestNode;
}

bool RoutingTableService::hasAddressRoutingTable(uint16_t address) {
    RouteNode* node = findNode(address);
    return node != nullptr;
}

uint16_t RoutingTableService::getNextHop(uint16_t dst) {
    RouteNode* node = findNode(dst);

    if (node == nullptr)
        return 0;

    return node->via;
}

uint8_t RoutingTableService::getNumberOfHops(uint16_t address) {
    RouteNode* node = findNode(address);

    if (node == nullptr)
        return 0;

    return node->networkNode.metric;
}

void RoutingTableService::processRoute(RoutePacket* p, int8_t receivedSNR) {
    if ((p->packetSize - sizeof(RoutePacket)) % sizeof(NetworkNode) != 0) {
        ESP_LOGE(LM_TAG, "Invalid route packet size");
        return;
    }

    size_t numNodes = p->getNetworkNodesSize();
    ESP_LOGI(LM_TAG, "Route packet from %X with size %d", p->src, numNodes);

    // NetworkNode* receivedNode = new NetworkNode(p->src, 1, p->nodeRole);
    // processRoute(p->src, receivedNode);
    // delete receivedNode;

    // Extract broker IP from the RoutePacket
    IPAddress senderBrokerIP = p->brokerIP;
    ESP_LOGI(LM_TAG, "Received broker IP %s from %X", senderBrokerIP.toString().c_str(), p->src);

    // Create & process sender node (Node that sent this route update)
    NetworkNode* receivedNode = new NetworkNode(p->src, 1, p->nodeRole, senderBrokerIP, p->ble_conn_id);
    processRoute(p->src, receivedNode, senderBrokerIP);
    delete receivedNode;

    resetReceiveSNRRoutePacket(p->src, receivedSNR);

    for (size_t i = 0; i < numNodes; i++) {
        NetworkNode* node = &p->networkNodes[i];
        node->metric++;
        processRoute(p->src, node, senderBrokerIP);
    }

    printRoutingTable();
}

void RoutingTableService::resetReceiveSNRRoutePacket(uint16_t src, int8_t receivedSNR) {
    RouteNode* rNode = findNode(src);
    if (rNode == nullptr)
        return;

    ESP_LOGI(LM_TAG, "Reset Receive SNR from %X: %d", src, receivedSNR);

    rNode->receivedSNR = receivedSNR;
}

void RoutingTableService::processRoute(uint16_t via, NetworkNode* node, IPAddress brokerIP) {
    if (node->address != WiFiService::getLocalAddress()) {

        RouteNode* rNode = findNode(node->address);

        if (rNode == nullptr) {
            // Inject broker IP into the node before saving
            node->mqttBrokerIP = brokerIP;
            addNodeToRoutingTable(node, via);
            return;
        }

        if (node->metric < rNode->networkNode.metric) {
            rNode->networkNode.metric = node->metric;
            rNode->via = via;
            resetTimeoutRoutingNode(rNode);

            // Update IP on better route
            rNode->networkNode.mqttBrokerIP = brokerIP;

            ESP_LOGI(LM_TAG, "Found better route for %X via %X metric %d", node->address, via, node->metric);
        }
        else if (node->metric == rNode->networkNode.metric) {
            resetTimeoutRoutingNode(rNode);
        }

        if (getNextHop(node->address) == via && node->role != rNode->networkNode.role) {
            ESP_LOGI(LM_TAG, "Updating role of %X to %d", node->address, node->role);
            rNode->networkNode.role = node->role;
        }

        if (getNextHop(node->address) == via && rNode->networkNode.mqttBrokerIP != brokerIP) {
            ESP_LOGI(LM_TAG, "Updating broker IP of %X to %s", node->address, brokerIP.toString().c_str());
            rNode->networkNode.mqttBrokerIP = brokerIP;
        }

        if (getNextHop(node->address) == via && rNode->networkNode.ble_conn_id != node->ble_conn_id) {
            ESP_LOGI(LM_TAG, "Updating BLE conn ID of %X to %u", node->address, node->ble_conn_id);
            rNode->networkNode.ble_conn_id = node->ble_conn_id;
        }        
    }
}



void RoutingTableService::addNodeToRoutingTable(NetworkNode* node, uint16_t via) {
    if (routingTableList->getLength() >= RTMAXSIZE) {
        ESP_LOGW(LM_TAG, "Routing table max size reached, not adding route and deleting it");
        return;
    }

    if (calculateMaximumMetricOfRoutingTable() < node->metric) {
        ESP_LOGW(LM_TAG, "Trying to add a route with a metric higher than the maximum of the routing table, not adding route and deleting it");
        return;
    }

    RouteNode* rNode = new RouteNode(node->address, node->metric, node->role, via);
    rNode->networkNode.mqttBrokerIP = node->mqttBrokerIP; //Added to save mqtt broker ip

    //Reset the timeout of the node
    resetTimeoutRoutingNode(rNode);

    routingTableList->setInUse();

    routingTableList->Append(rNode);

    routingTableList->releaseInUse();

    //ESP_LOGI(LM_TAG, "New route added: %X via %X metric %d, role %d", node->address, via, node->metric, node->role);
    ESP_LOGI(LM_TAG, "New route added: %X via %X metric %d, role %d, broker %s",
        node->address, via, node->metric, node->role, rNode->networkNode.mqttBrokerIP.toString().c_str());
}

NetworkNode* RoutingTableService::getAllNetworkNodes() {
    routingTableList->setInUse();

    int routingSize = routingTableSize();

    // If the routing table is empty return nullptr
    if (routingSize == 0) {
        routingTableList->releaseInUse();
        return nullptr;
    }

    NetworkNode* payload = new NetworkNode[routingSize];

    if (routingTableList->moveToStart()) {
        for (int i = 0; i < routingSize; i++) {
            RouteNode* currentNode = routingTableList->getCurrent();
            payload[i] = currentNode->networkNode;

            if (!routingTableList->next())
                break;
        }
    }

    routingTableList->releaseInUse();

    return payload;
}

void RoutingTableService::resetTimeoutRoutingNode(RouteNode* node) {
    node->timeout = millis() + DEFAULT_TIMEOUT * 1000;
}

void RoutingTableService::printRoutingTable() {
    ESP_LOGI(LM_TAG, "Current routing table:");

    routingTableList->setInUse();

    if (routingTableList->moveToStart()) {
        size_t position = 0;

        do {
            RouteNode* node = routingTableList->getCurrent();

            // ESP_LOGI(LM_TAG, "%d - %X via %X metric %d Role %d", position,
            //     node->networkNode.address,
            //     node->via,
            //     node->networkNode.metric,
            //     node->networkNode.role);

            IPAddress ip = node->networkNode.mqttBrokerIP;
            Serial.printf(
                "%d | %04X | via %04X | metric %d | Role %d | Broker_IP %d.%d.%d.%d | BLE_Conn_ID %d\n",
                position,
                node->networkNode.address,
                node->via,
                node->networkNode.metric,
                node->networkNode.role,
                ip[0], ip[1], ip[2], ip[3],
                node->networkNode.ble_conn_id
            );
            
            position++;
        } while (routingTableList->next());
    }

    routingTableList->releaseInUse();
}

void RoutingTableService::manageTimeoutRoutingTable() {
    ESP_LOGI(LM_TAG, "Checking routes timeout");

    routingTableList->setInUse();

    if (routingTableList->moveToStart()) {
        do {
            RouteNode* node = routingTableList->getCurrent();

            if (node->timeout < millis()) {
                ESP_LOGW(LM_TAG, "Route timeout %X via %X", node->networkNode.address, node->via);

                delete node;
                routingTableList->DeleteCurrent();
            }

        } while (routingTableList->next());
    }

    routingTableList->releaseInUse();

    printRoutingTable();
}

uint8_t RoutingTableService::calculateMaximumMetricOfRoutingTable() {
    routingTableList->setInUse();

    uint8_t maximumMetricOfRoutingTable = 0;

    if (routingTableList->moveToStart()) {
        do {
            RouteNode* node = routingTableList->getCurrent();

            if (node->networkNode.metric > maximumMetricOfRoutingTable)
                maximumMetricOfRoutingTable = node->networkNode.metric;

        } while (routingTableList->next());
    }

    routingTableList->releaseInUse();

    return maximumMetricOfRoutingTable + 1;
}

LM_LinkedList<RouteNode>* RoutingTableService::routingTableList = new LM_LinkedList<RouteNode>();