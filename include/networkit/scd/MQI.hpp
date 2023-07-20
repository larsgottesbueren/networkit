#ifndef NETWORKIT_MQI_HPP
#define NETWORKIT_MQI_HPP

#include <networkit/scd/SelectiveCommunityDetector.hpp>

namespace NetworKit {

/**
 *   Kevin Lang, Satish Rao (2004).
 *   A Flow-Based Method for Improving the Expansion or Conductance of Graph Cuts
 *   IPCO 2004
 *   https://link.springer.com/chapter/10.1007/978-3-540-25960-2_25
 */
class MQI : public SelectiveCommunityDetector {
private:
    edgeweight graph_volume = 0.0;

public:
    /**
     * Constructs the Local Tightness Expansion algorithm.
     *
     * @param[in] G The graph to detect communities on
     */
    MQI(const Graph &g);

    /**
     * Expands a set of seed nodes into a community.
     *
     * @param[in] s The seed nodes
     * @return A community of the seed nodes
     */
    std::set<node> expandOneCommunity(const std::set<node> &s) override;

    using SelectiveCommunityDetector::expandOneCommunity;

    void setGraph(const Graph& new_g) override;
};

} // namespace NetworKit



#endif // NETWORKIT_MQI_HPP
