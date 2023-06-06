#include <limits>
#include <queue>
#include <stdexcept>

#include <networkit/flow/PushRelabel.hpp>

namespace NetworKit {

PushRelabel::PushRelabel(const Graph &graph, node source, node sink)
    : graph(&graph), source(source), sink(sink) {}


void PushRelabel::run() {
    if (hasRun) { throw std::runtime_error("Flow algo already ran."); }
    if (!graph->hasEdgeIds()) { throw std::runtime_error("edges have not been indexed - call indexEdges first"); }

    max_level = graph->numberOfNodes();
    flow.assign(graph->upperEdgeIdBound(), 0.0);
    excess.assign(graph->upperNodeIdBound(), 0.0);
    reverseEdges.resize(graph->upperEdgeIdBound());
    cap.resize(graph->upperEdgeIdBound());
    graph->forEdges([&](node u, node v, edgeweight w, edgeid eid) {
      index reverse = graph->edgeId(v, u);
      reverseEdges[eid] = reverse;
      cap[eid] = w;
    });
    level.assign(graph->upperNodeIdBound(), 0);
    level[source] = max_level;


    // saturate source edges
    graph->forNeighborsOf(source, [&](node, node v, edgeweight w, edgeid eid) {
        push(source, v, eid, w);
    });

    work = std::numeric_limits<count>::max();   // start with relabeling
    global_relabel_threshold = (6 * graph->numberOfNodes() + graph->numberOfEdges()) / 5;

    while (!active_nodes.empty()) {
        if (work > global_relabel_threshold) {
            globalRelabel();
            work = 0;
        }
        node u = active_nodes.front();
        active_nodes.pop();
        discharge(u);
    }

    flowValue = excess[sink];
    hasRun = true;
}

void PushRelabel::discharge(node u) {
    while (excess[u] > 0 && level[u] < max_level) {
        int min_level = max_level;
        graph->forNeighborsOf(u, [&](node , node v, edgeid eid) {
            edgeweight rcap = std::min(excess[u], cap[eid] - flow[eid]);
            if (rcap > 0) {
                assert(level[u] <= level[v] + 1);
                if (level[u] == level[v] + 1) {
                    push(u, v, eid, rcap);
                } else {
                    min_level = std::min(min_level, level[v] + 1);
                }
            }
        });
        if (excess[u] > 0) {
            level[u] = min_level;
        }
        work += graph->degree(u) + 2;
    }
}

void PushRelabel::globalRelabel() {
    level.assign(level.size(), max_level);
    level[sink] = 0;
    bfs_queue.clear();
    bfs_queue.push_back(sink);
    size_t first = 0;
    size_t last = bfs_queue.size();
    int dist = 1;
    while (first != last) {
        for ( ; first < last; ++first) {
            node u = bfs_queue[first];
            graph->forNeighborsOf(u, [&](node , node v, edgeweight , edgeid eid) {
                if (level[v] == max_level) {
                    edgeid rev = reverseEdges[eid];
                    edgeweight rcap = cap[rev] - flow[rev];
                    if (rcap > 0) {
                        bfs_queue.push_back(v);
                        level[v] = dist;
                    }
                }
            });
        }
        last = bfs_queue.size();
        dist++;
    }
}

edgeweight PushRelabel::getMaxFlow() const {
    assureFinished();
    return flowValue;
}

std::vector<node> PushRelabel::getSinkSet() {
    assureFinished();
    globalRelabel();
    return bfs_queue;
}

std::vector<node> PushRelabel::getSourceSet() const {
    assureFinished();
    // perform bfs from source
    std::vector<bool> visited(graph->upperNodeIdBound(), false);
    std::vector<node> sourceSet;

    std::queue<node> Q;
    Q.push(source);
    visited[source] = true;
    graph->forNodes([&](node u) {
        if (excess[u] > 0 && u != source && u != sink) {
            Q.push(u);
            visited[u] = true;
        }
    });
    while (!Q.empty()) {
        node u = Q.front();
        Q.pop();
        sourceSet.push_back(u);

        graph->forNeighborsOf(u, [&](node, node v, edgeweight weight, edgeid eid) {
            if (!visited[v] && flow[eid] < weight) {
                Q.push(v);
                visited[v] = true;
            }
        });

    }

    return sourceSet;
}

edgeweight PushRelabel::getFlow(node u, node v) const {
    return flow[graph->edgeId(u, v)];
}

const std::vector<edgeweight> &PushRelabel::getFlowVector() const {
    assureFinished();
    return flow;
}

} /* namespace NetworKit */
