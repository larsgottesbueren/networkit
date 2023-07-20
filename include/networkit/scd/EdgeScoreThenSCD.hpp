#ifndef NETWORKIT_EDGESCORETHENSCD_HPP
#define NETWORKIT_EDGESCORETHENSCD_HPP

#include <networkit/edgescores/EdgeScore.hpp>
#include <networkit/scd/SelectiveCommunityDetector.hpp>

namespace NetworKit {
/**
 * Helper for running a selective community detection algorithm on a graph with modified edge scores
 */
class EdgeScoreThenSCD : public SelectiveCommunityDetector {
public:
    /**
     * Initialize the combined algorithm with the given graph and the given two algorithms.
     *
     * @param G The graph to work on.
     * @param scd The community detection algorithm.
     * @param edge_score The edge score algorithm.
     */
    EdgeScoreThenSCD(const Graph &g, SelectiveCommunityDetector &scd,
                     EdgeScore<double> &edge_score);

    /**
     * Expand a community with the given seed node.
     *
     * @param s The seed node to start with.
     * @return The community found.
     */
    std::set<node> expandOneCommunity(node s) override;

    /**
     * Expand a community with the given seed nodes.
     *
     * @param s The seed nodes to start with.
     * @return The community found.
     */
    std::set<node> expandOneCommunity(const std::set<node> &s) override;

    void setGraph(const Graph& new_g) override;

protected:
    SelectiveCommunityDetector &scd;
    EdgeScore<double> &edge_score;
    Graph reweighted_graph;
};

} /* namespace NetworKit */


#endif // NETWORKIT_EDGESCORETHENSCD_HPP
