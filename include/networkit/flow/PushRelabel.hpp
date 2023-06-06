#ifndef NETWORKIT_FLOW_PUSH_RELABEL_HPP_
#define NETWORKIT_FLOW_PUSH_RELABEL_HPP_

#include <vector>

#include <networkit/base/Algorithm.hpp>
#include <networkit/graph/Graph.hpp>

namespace NetworKit {

/**
 * @ingroup flow
 * The PushRelabel class implements the maximum flow algorithm by Edmonds and Karp.
 */
class PushRelabel final : public Algorithm {
    const Graph *graph;

    node source, sink;
    index max_level;

    std::vector<edgeweight> flow, cap, excess;
    edgeweight flowValue;
    std::vector<index> reverseEdges;
    std::vector<int> level;
    std::queue<node> active_nodes;

    count work = 0;
    count global_relabel_threshold = 0;
    std::vector<node> bfs_queue;

public:
    /**
     * Constructs an instance of the PushRelabel algorithm for the given graph, source and sink
     * @param graph The graph.
     * @param source The source node.
     * @param sink The sink node.
     */
    PushRelabel(const Graph &graph, node source, node sink);

    /**
     * Computes the maximum flow, executes the PushRelabel algorithm.
     */
    void run() override;

    /**
     * Returns the value of the maximum flow from source to sink.
     *
     * @return The maximum flow value
     */
    edgeweight getMaxFlow() const;

    /**
     * Returns the set of the nodes on the source side of the flow/minimum cut.
     *
     * @return The set of nodes that form the (smallest) source side of the flow/minimum cut.
     */
    std::vector<node> getSourceSet() const;

    std::vector<node> getSinkSet();

    /**
     * Get the flow value between two nodes @a u and @a v.
     * @warning The running time of this function is linear in the degree of u.
     *
     * @param u The first node
     * @param v The second node
     * @return The flow between node u and v.
     */
    edgeweight getFlow(node u, node v) const;

    /**
     * Get the flow value of an edge.
     *
     * @param eid The id of the edge
     * @return The flow on the edge identified by eid
     */
    edgeweight getFlow(edgeid eid) const {
        assureFinished();
        return flow[eid];
    };

    /**
     * Return a copy of the flow values of all edges.
     * @note Instead of copying all values you can also use the inline function "getFlow(edgeid)" in
     * order to access the values efficiently.
     *
     * @return The flow values of all edges
     */
    const std::vector<edgeweight> &getFlowVector() const;

private:
    void push(node u, node v, edgeid eid, edgeweight delta) {
        if (delta <= 0) return;
        if (excess[v] == 0 && v != sink) {
            active_nodes.push(v);
        }
        excess[u] -= delta;
        excess[v] += delta;
        flow[eid] += delta;
        flow[reverseEdges[eid]] -= delta;
    }

    void discharge(node u);
    void globalRelabel();
};

} /* namespace NetworKit */

#endif // NETWORKIT_FLOW_PUSH_RELABEL_HPP_
