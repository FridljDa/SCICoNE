//
// Created by Tuncel  Mustafa Anil on 9/27/18.
//

#include <iostream>
#include <string>
#include "Simulation.h"
#include <cxxopts.hpp>

using namespace std;



int main(int argc, char* argv[])
{

    // the default values
    int n_regions = 50;
    int n_nodes = 50;
    double lambda_r = 0.1;
    double lambda_c = 0.2;
    int n_cells = 500;
    int n_bins = 10000;
    int n_reads = 10000;
    int n_iters = 5000;
    int n_repetitions = 1;
    int max_region_size = 10;
    int ploidy = 2;
    int verbosity = 0;

    string f_name_postfix = "";


    cxxopts::Options options("Mcmc simulations", "simulates cnv values, infers them and benchmarks");
    options.add_options()
            ("n_bins", "Number of bins in the input matrix", cxxopts::value(n_bins))
            ("n_nodes", "Number of nodes of the tree", cxxopts::value(n_nodes))
            ("n_regions", "Number of regions", cxxopts::value(n_regions))
            ("n_iters", "Number of iterations", cxxopts::value(n_iters))
            ("n_rep", "Number of repetitions", cxxopts::value(n_repetitions))
            ("n_reads", "Number of reads per cell", cxxopts::value(n_reads))
            ("ploidy", "ploidy", cxxopts::value(ploidy))
            ("verbosity", "verbosity", cxxopts::value(verbosity))
            ("postfix", "postfix", cxxopts::value(f_name_postfix));

    auto result = options.parse(argc, argv);

    if (result.count("n_bins"))
    {
        n_bins = result["n_bins"].as<int>();
    }
    if (result.count("n_nodes"))
    {
        n_nodes = result["n_nodes"].as<int>();
    }
    if (result.count("n_regions"))
    {
        n_regions = result["n_regions"].as<int>();
    }
    if (result.count("n_reads"))
    {
        n_reads = result["n_reads"].as<int>();
    }
    if (result.count("n_iters"))
    {
        n_iters = result["n_iters"].as<int>();
    }
    if (result.count("n_rep"))
    {
        n_repetitions = result["n_rep"].as<int>();
    }
    if (result.count("ploidy"))
    {
        ploidy = result["ploidy"].as<int>();
    }
    if (result.count("verbosity"))
    {
        verbosity = result["verbosity"].as<int>();
    }
    if (result.count("postfix"))
    {
        f_name_postfix = result["postfix"].as<string>();
    }


    // create the D matrix
//    vector<vector<double>> D(n_cells, vector<double>(n_regions)); // initialize with the default value
//
//    vector<int> region_sizes(n_regions); // sampling the region sizes

    Simulation sim(n_regions, n_bins, n_nodes, lambda_r, lambda_c, n_cells, n_reads, max_region_size, ploidy, verbosity);
    //double delta_random_init = sim.random_cnvs_inference();

    sim.sample_region_sizes(n_bins);
    sim.simulate_count_matrix(false, verbosity);
    sim.split_regions_to_bins();

    sim.infer_cnvs(n_iters, verbosity); // n_iters: 50000
    cout << "delta from our method: " << sim.delta << endl;

    // cout << "delta from random method: " << delta_random_init << endl;

    sim.write_d_vector(f_name_postfix);

    //simulate(n_regions, n_nodes, lambda_r, lambda_c, n_cells, n_reads, D, region_sizes); // initializes D and region_sizes

    // initialize the tree and infer the CNV profiles

    return EXIT_SUCCESS;


}

