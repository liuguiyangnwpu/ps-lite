/**
 *  Copyright (c) 2015 by Contributors
 */
#ifndef PS_INTERNAL_MESSAGE_H_
#define PS_INTERNAL_MESSAGE_H_
#include <vector>
#include <limits>
#include <string>
#include <sstream>
#include "ps/sarray.h"

namespace ps {

enum DataType {
    CHAR, INT8, INT16, INT32, INT64,
    UINT8, UINT16, UINT32, UINT64,
    FLOAT, DOUBLE, OTHER
};

static const char* DataTypeName[] = {
    "CHAR", "INT8", "INT16", "INT32", "INT64",
    "UINT8", "UINT16", "UINT32", "UINT64",
    "FLOAT", "DOUBLE", "OTHER"
};

template<typename V, typename W>
inline bool SameType() {
    return std::is_same<typename std::remove_cv<V>::type, W>::value;
}

template<typename V>
DataType GetDataType() {
    if (SameType<V, int8_t>()) {
        return INT8;
    } else if (SameType<V, int16_t>()) {
        return INT16;
    } else if (SameType<V, int32_t>()) {
        return INT32;
    } else if (SameType<V, int64_t>()) {
        return INT64;
    } else if (SameType<V, uint8_t>()) {
        return UINT8;
    } else if (SameType<V, uint16_t>()) {
        return UINT16;
    } else if (SameType<V, uint32_t>()) {
        return UINT32;
    } else if (SameType<V, uint64_t>()) {
        return UINT64;
    } else if (SameType<V, float>()) {
        return FLOAT;
    } else if (SameType<V, double>()) {
        return DOUBLE;
    } else {
        return OTHER;
    }
}

struct Node {
    static const int kEmpty;
    Node() : id(kEmpty), port(kEmpty), is_recovery(false) {}
    enum Role { SERVER, WORKER, SCHEDULER };

    std::string DebugString() const {
        std::stringstream ss;
        ss << "role=" << (role == SERVER ? "server" : (role == WORKER ? "worker" : "scheduler"))
            << (id != kEmpty ? ", id=" + std::to_string(id) : "")
            << ", ip=" << hostname << ", port=" << port << ", is_recovery=" << is_recovery;

        return ss.str();
    }

    std::string ShortDebugString() const {
        std::string str = role == SERVER ? "S" : (role == WORKER ? "W" : "H");
        if (id != kEmpty) str += "[" + std::to_string(id) + "]";
        return str;
    }

    Role role;
    int id;
    std::string hostname;
    int port;
    /* whether this node is created by failover */
    bool is_recovery;
};

struct Control {
    Control() : cmd(EMPTY) { }
    inline bool empty() const { return cmd == EMPTY; }
    std::string DebugString() const {
        if (empty()) return "";
        std::vector<std::string> cmds = {
            "EMPTY", "TERMINATE", "ADD_NODE", "BARRIER", "ACK", "HEARTBEAT"
        };
        std::stringstream ss;
        ss << "cmd=" << cmds[cmd];
        if (node.size()) {
            ss << ", node={";
            for (const Node& n : node) 
                ss << " " << n.DebugString();
            ss << " }";
        }
        if (cmd == BARRIER) 
            ss << ", barrier_group=" << barrier_group;
        if (cmd == ACK) 
            ss << ", msg_sig=" << msg_sig;
        return ss.str();
    }
    enum Command { EMPTY, TERMINATE, ADD_NODE, BARRIER, ACK, HEARTBEAT };
    Command cmd;
    std::vector<Node> node;
    int barrier_group;
    uint64_t msg_sig;
};

struct Meta {
    static const int kEmpty;
    Meta() : head(kEmpty), customer_id(kEmpty), timestamp(kEmpty),
            sender(kEmpty), recver(kEmpty),
            request(false), push(false), simple_app(false) {}
    std::string DebugString() const {
        std::stringstream ss;
        if (sender == Node::kEmpty) {
            ss << "?";
        } else {
            ss << sender;
        }
        ss <<  " => " << recver;
        ss << ". Meta: request=" << request;
        if (timestamp != kEmpty) 
            ss << ", timestamp=" << timestamp;
        if (!control.empty()) {
            ss << ", control={ " << control.DebugString() << " }";
        } else {
            ss << ", customer_id=" << customer_id << ", simple_app=" << simple_app << ", push=" << push;
        }
        if (head != kEmpty) 
            ss << ", head=" << head;
        if (body.size()) 
            ss << ", body=" << body;
        if (data_type.size()) {
            ss << ", data_type={";
            for (auto d : data_type) 
                ss << " " << DataTypeName[static_cast<int>(d)];
            ss << " }";
        }
        return ss.str();
    }

    int head;
    /* the unique id of the customer is messsage is for*/
    int customer_id;
    /* the timestamp of this message */
    int timestamp;
    /* the node id of the sender of this message */
    int sender;
    /* the node id of the receiver of this message */
    int recver;
    /* whether or not this is a request message*/
    bool request;
    /* whether or not a push message */
    bool push;
    /* whether or not it's for SimpleApp */
    bool simple_app;
    /* an string body */
    std::string body;
    /* data type of message.data[i] */
    std::vector<DataType> data_type;
    /* system control message */
    Control control;
};

struct Message {
    /* the meta info of this message */
    Meta meta;
    /* the large chunk of data of this message */
    std::vector<SArray<char> > data;
    /* push array into data, and add the data type */
    template <typename V>
    void AddData(const SArray<V>& val) {
        CHECK_EQ(data.size(), meta.data_type.size());
        meta.data_type.push_back(GetDataType<V>());
        data.push_back(SArray<char>(val));
    }
    std::string DebugString() const {
        std::stringstream ss;
        ss << meta.DebugString();
        if (data.size()) {
            ss << " Body:";
            for (const auto& d : data) 
                ss << " data_size=" << d.size();
        }
        return ss.str();
    }
};
}  // namespace ps
#endif  // PS_INTERNAL_MESSAGE_H_
