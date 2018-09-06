//
// Created by Tuncel  Mustafa Anil on 7/16/18.
//

#ifndef SC_DNA_NODE_H
#define SC_DNA_NODE_H

#include <map>
#include <iostream>

using namespace std;

struct Node{
    int id = 0;
    std::map<u_int,int> c = {};
    uint64_t  c_hash = 0;
    std::map<u_int,int> c_change= {};
    double log_score = 0.0;
    int z = 0;
    unsigned n_descendents = 1; // including itself
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

    int get_n_children() const;
    bool is_leaf() const;
    vector<Node*> get_descendents(bool with_n=true) const;

    // copy constructor
    Node(Node& source_node)
    {
        id = source_node.id;
        c = source_node.c;
        c_hash = source_node.c_hash;
        c_change = source_node.c_change;
        // log scores are not copied since they rely on cells
        log_score = 0.0;
        z = source_node.z;
        n_descendents = source_node.n_descendents;
        first_child = next = parent = nullptr;
    }
    Node()
    {}
    ~Node()
    {}


    friend std::ostream& operator<<(std::ostream& os, Node& n);

};

std::ostream& operator<<(std::ostream& os, Node& n) {
    os << "node " << n.id << ": ";
    for (auto i : n.c_change)
        os << " " << i.first << ":" << i.second << ',';

    if (n.parent == nullptr)
        os << "parent: NULL";
    else
        os << "parent: " << n.parent->id;

    os << endl << "\t c values:";
    for (auto i : n.c)
        os << "\t  " << i.first << ":" << i.second << ',';

    os << endl << "\t z value: " << n.z;
    os << endl << "\t n_descendents: " << n.n_descendents;


    return os;
}

int Node::get_n_children() const{

    int n_children = 0;
    for (Node* temp = this->first_child; temp != nullptr; temp=temp->next)
    {
        n_children++;
    }
    return n_children;
}

bool Node::is_leaf() const{
    /*
     * Returns true if the node is a leaf node, e.g. has no children
     * */
    if (this->first_child == nullptr)
        return true;
    else
        return false;
}

vector<Node *> Node::get_descendents(bool with_n) const {
    /*
     * Returns the descendents of node* n in a list in a BFS fashion.
     * If with_n, then the descendents contain the node itself, otherwise not.
     * Does preserve the order (e.g. parent is before the children)
     *
     * */
    vector<Node *> descendents;

    std::stack<Node*> stk;
    stk.push(const_cast<Node*> (this)); // because this pointer is constant
    while (!stk.empty()) {
        Node* top = stk.top();
        stk.pop();
        for (Node* temp = top->first_child; temp != nullptr; temp=temp->next)
            stk.push(temp);
        descendents.push_back(top);
    }

    if (!with_n)
        descendents.erase(descendents.begin()); // erase the first node, which is n

    return descendents;
}

#endif //SC_DNA_NODE_H
