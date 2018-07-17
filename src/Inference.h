//
// Created by Tuncel  Mustafa Anil on 7/17/18.
//

#ifndef SC_DNA_INFERENCE_H
#define SC_DNA_INFERENCE_H

#include "Tree.h"
#include <vector>

class Inference {
/*
 * Contains functionality to perform monte carlo markov chains (mcmc) inference
 * */
private:
    Tree *t;
    Tree *t_prime;
    std::vector<std::vector<double>> t_scores;
    std::vector<double> t_sums;

    std::vector<std::vector<double>> t_prime_scores;
    std::vector<double> t_prime_sums;

public:
    Inference(u_int ploidy=2);

    virtual ~Inference();

    void compute_t_table(const vector<vector<int>> &D, const vector<int>& r);


    void prune_reattach(const vector<vector<int>> &D, const vector<int>& r);
    void w_prune_reattach();

    void swap();
    void w_swap();


    void random_initialize(); // randomly initializes a tree and copies it into the other
    void test_initialize(); // initializes the trees based on the test example
};



void Inference::random_initialize() {

}

void Inference::test_initialize() {

    // build tree
    t->random_insert({{0, 1}, {1, 1}});
    t->insert_at(1,{{1, 1}, {2, 1}});
    t->insert_at(2,{{0, -1}});
    t->insert_at(2,{{3, -1}});
    t->insert_at(1,{{1, 1}});

    t_prime = t;
}

Inference::Inference(u_int ploidy) {
    t = new Tree(ploidy);
    t_prime = new Tree(ploidy);


}

Inference::~Inference() {
    t->destroy();
    t_prime->destroy();
}

void Inference::prune_reattach(const vector<vector<int>> &D, const vector<int>& r) {
    auto attached_node = t_prime->prune_reattach();

    if (attached_node != nullptr)
    {
        for (auto const &d: D)
        {
            int sum_d = accumulate( d.begin(), d.end(), 0.0);;
            t_prime->compute_stack(attached_node, d, sum_d,r);
        }
        auto c_ids = t_prime->get_children_ids(attached_node);

        cout << "debug";

        // TODO get scores for given c_ids
        // TODO perform comparisons
    }







}

void Inference::compute_t_table(const vector<vector<int>> &D, const vector<int>& r) {

    int n = static_cast<int>(D.size());
    for (int i = 0; i < n; ++i)
    {
        this->t->compute_tree(D[i], r);
        auto scores_vec = this->t->get_scores();
        this->t_scores.push_back(scores_vec);
        this->t_sums.push_back(MathOp::log_sum(scores_vec));
    }


};

#endif //SC_DNA_INFERENCE_H
