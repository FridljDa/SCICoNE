//
// Created by Tuncel  Mustafa Anil on 7/16/18.
//

#ifndef SC_DNA_NODE_H
#define SC_DNA_NODE_H

#include <unordered_map>

struct Node{
    int id = 0;
    std::unordered_map<u_int,int> c = {};
    std::unordered_map<u_int,int> c_change= {};
    double log_score = 0.0;
    int z = 0;
    Node* first_child = nullptr;
    Node* next = nullptr;
    Node* parent = nullptr;

    bool operator<(const Node &rhs) const {
        return id < rhs.id;
    }

    bool operator>(const Node &rhs) const {
        return rhs < *this;
    }

    bool operator<=(const Node &rhs) const {
        return !(rhs < *this);
    }

    bool operator>=(const Node &rhs) const {
        return !(*this < rhs);
    }

    bool operator==(const Node &rhs) const {
        return id == rhs.id;
    }

    bool operator!=(const Node &rhs) const {
        return !(rhs == *this);
    }

    // copy constructor
    Node(Node& source_node)
    {
        id = source_node.id;
        c = source_node.c;
        c_change = source_node.c_change;
        // log scores are not copied since they rely on cells
        log_score = 0.0;
        z = source_node.z;
        first_child = next = parent = nullptr;
    }
    Node()
    {}
    ~Node()
    {}
};

#endif //SC_DNA_NODE_H
