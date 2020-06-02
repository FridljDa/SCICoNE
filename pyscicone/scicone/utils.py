import numpy as np
import pandas as pd
from scipy.cluster.hierarchy import ward, leaves_list
from scipy.spatial.distance import pdist
from copy import deepcopy
from pybiomart import Server

def sort_chromosomes(chromosome_list):
    """
    Sorts a list of unordered chromosome names
    :param chromosome_list: list of unordered characters denoting chromosomes '1', '2', ..., 'X', 'Y'
    """
    # Replace X and Y with 23 and 24
    sorted_chromosome_list = np.array(chromosome_list)
    sorted_chromosome_list[np.where(sorted_chromosome_list == "X")[0]] = 23
    sorted_chromosome_list[np.where(sorted_chromosome_list == "Y")[0]] = 24

    # Convert everything to integer
    sorted_chromosome_list = sorted_chromosome_list.astype(int)

    # Sort
    sorted_chromosome_list = np.sort(sorted_chromosome_list)

    # Convert back to string
    sorted_chromosome_list = sorted_chromosome_list.astype(str)
    sorted_chromosome_list[np.where(sorted_chromosome_list == "23")[0]] = "X"
    sorted_chromosome_list[np.where(sorted_chromosome_list == "24")[0]] = "Y"

    return sorted_chromosome_list

def set_region_neutral_states(all_region_stops, known_region_stops, known_region_neutral_states):
    """
        Creates a numpy array of size n_regions with the neutral state of each region.
    """
    if all_region_stops[-1] > known_region_stops[-1]:
        known_region_stops = list(known_region_stops)
        known_region_stops.append(all_region_stops[-1])
    elif all_region_stops[-1] < known_region_stops[-1]:
        all_region_stops = list(all_region_stops)
        all_region_stops.append(known_region_stops[-1])

    all_region_stops = np.array(all_region_stops).ravel().astype(int)
    known_region_stops = np.array(known_region_stops).ravel().astype(int)
    known_region_neutral_states = np.array(known_region_neutral_states).ravel().astype(int)
    all_region_neutral_states = np.zeros(all_region_stops.shape).astype(int)

    start_region = 0
    for i, known_stop in enumerate(known_region_stops): # known stops
        # Region with breakpoint matching the known stop
        end_region = np.where(all_region_stops==known_stop)[0][0].astype(int)

        # Set all the regions between the last known stop and the current stop to the known neutral state
        all_region_neutral_states[start_region:end_region+1] = known_region_neutral_states[i]
        start_region = end_region + 1

    return all_region_neutral_states

def cluster_clones(data, labels, within_clone=True):
    data_ = np.array(data, copy=True)
    labels = np.array(labels).ravel()
    labels_ = np.array(labels, copy=True)

    unique_labels = np.unique(labels_)

    # Cluster data by label
    start = 0
    for label in unique_labels:
        idx = np.where(labels == label)[0]
        end = start + len(idx)
        data_[start:end] = data[idx]
        labels_[start:end] = labels[idx]
        start = end

    if within_clone:
        # Cluster the data within each label
        for label in unique_labels:
            idx = np.where(labels_ == label)[0]
            if len(idx) > 1:
                Z = ward(pdist(data_[idx]))
                hclust_index = leaves_list(Z)
                data_[idx] = data_[idx][hclust_index]

    return data_, labels_

def mapd(data, read_depths=None):
    pd = np.zeros(data.shape)
    cell_means = np.mean(data, axis=1)
    for i in range(data.shape[1] - 1):
        pd[:,i] = (data[:,i] - data[:,i+1])/cell_means
    mapd = np.median(np.abs(pd - np.median(pd, axis=1).reshape(-1,1)), axis=1)

    if read_depths is not None:
        mapd = mapd * np.sqrt(read_depths)
    return mapd

def create_fusion_tree(learned_tree, region_neutral_states):
    fusion_tree = deepcopy(learned_tree)

    # Adds 1 node below each node of the learned tree with an added neutral genome
    new_event_dict = dict()
    for i, state in enumerate(region_neutral_states):
        new_event_dict[str(i)]=str(int(state))

    fusion_tree.read_tree_str(learned_tree.tree_str)

    for node in list(fusion_tree.node_dict):
        new_node_id = str(int(node) + 1000)
        fusion_tree.node_dict[new_node_id] = dict(parent_id=node, event_dict=new_event_dict)

    fusion_tree.update_tree_str()

    return fusion_tree

def get_bin_gene_region_df(
    bin_size,
    chr_stops,
    region_stops,
    excluded_bins
    ):
    """
        Creates a pands.DataFrame with the gene and region corresponding to each bin
        :param bin_size: number of base pairs in a bin
        :param chromosome_stops: dictionary indicating final unfiltered bin of each chromosome
        :param region_stops: df indicating the final filtered bin of each region
        :param excluded_bins: list of excluded bins
        :return: DataFrame of (gene, chr, region, filtered_bin)
    """

    # Pull genome annotations using pybiomart
    server = Server("www.ensembl.org", use_cache=False)
    dataset = server.marts["ENSEMBL_MART_ENSEMBL"].datasets["hsapiens_gene_ensembl"]
    gene_coordinates = dataset.query(attributes=["chromosome_name", "start_position", "end_position", "external_gene_name"],
                        filters={'chromosome_name':[1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,'X','Y']},
                        use_attr_names=True)

    bin_gene_region_df = pd.DataFrame(index=range(chr_stops['Y']+1))
    chr_stops_df = pd.DataFrame({'chr':list(chr_stops.keys()),
                                'stop':list(chr_stops.values())})

    bin_gene_region_df["region"] = None
    bin_gene_region_df["gene"] = [list() for _ in range(bin_gene_region_df.shape[0])]
    bin_gene_region_df["chr"] = [list() for _ in range(bin_gene_region_df.shape[0])]

    # for each gene
    for index, row in gene_coordinates.iterrows():
        start_bin = int(row["start_position"] / bin_size)
        stop_bin = int(row["end_position"] / bin_size)
        chromosome = str(row["chromosome_name"])

        if chromosome != "1":  # coordinates are given by chromosome
            chr_start = (
                chr_stops_df.iloc[np.where(chr_stops_df["chr"] == chromosome)[0][0] - 1].stop
                + 1
            )
            start_bin = start_bin + chr_start
            stop_bin = stop_bin + chr_start

        gene = row["external_gene_name"]
        for bin in range(start_bin, stop_bin + 1):
            bin_gene_region_df.loc[bin, "gene"].append(gene)
            bin_gene_region_df.loc[bin, "chr"].append(chromosome)

    # Turn columns of lists into columns of strings with comma-separated values
    bin_gene_region_df["gene"] = [
        ",".join(map(str, l)) for l in bin_gene_region_df["gene"]
    ]
    bin_gene_region_df["chr"] = [
        ",".join(map(str, l)) for l in bin_gene_region_df["chr"]
    ]

    # Indicate original_bin-filtered_bin correspondence
    bin_is_excluded = np.zeros(list(chr_stops.values())[-1]+1)
    bin_is_excluded[excluded_bins] = 1
    bin_is_excluded = bin_is_excluded.astype(bool)
    bin_gene_region_df["filtered_bin"] = None
    bin_gene_region_df["filtered_bin"].iloc[
        np.where(np.array(bin_is_excluded) == False)[0]
    ] = np.arange(np.count_nonzero(np.array(bin_is_excluded) == False))

    # Get the regions
    region_stops = list(region_stops)
    last_bin = np.count_nonzero(np.array(bin_is_excluded) == False) - 1
    region_stops.append(last_bin)
    start_bin = 0
    for i, stop_bin in enumerate(region_stops):
        original_start_bin = np.where(bin_gene_region_df["filtered_bin"] == start_bin)[0][0]
        original_stop_bin = np.where(bin_gene_region_df["filtered_bin"] == stop_bin)[0][0]
        bin_gene_region_df.loc[original_start_bin:original_stop_bin + 1, "region"] = i  # regions are 0 indexed
        start_bin = stop_bin

    return bin_gene_region_df

def get_region_gene_map(bin_size, chr_stops, region_stops, excluded_bins, filter_list=None):
    df = get_bin_gene_region_df(bin_size, chr_stops, region_stops, excluded_bins)

    region_gene_map = dict()
    for region in range(len(region_stops)):
        gene_list = []
        for row in df.loc[df['region']==region]['gene']:
            r = row.split(',')
            gene_list = list(set(gene_list + r))

        if filter_list is not None:
            gene_list = [gene for gene in gene_list if np.any(gene==np.array(filter_list))]

        region_gene_map[region] = gene_list

    return region_gene_map
