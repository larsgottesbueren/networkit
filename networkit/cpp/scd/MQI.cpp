#include <networkit/scd/MQI.hpp>

#include <networkit/flow/EdmondsKarp.hpp>
#include <networkit/flow/PushRelabel.hpp>

namespace NetworKit {

MQI::MQI(const Graph &g) :
      SelectiveCommunityDetector(g) {}

std::pair<double, double> ComputeCutAndVolume(const Graph& graph, const std::set<node>& community) {
    double cut = 0.0, volume = 0.0;
    for (node u : community) {
        if (graph.hasNode(u)) {
            graph.forEdgesOf(u, [&](node, node v, edgeweight ew) {
              if (community.find(v) == community.end()) {
                  cut += ew;
              }
              volume += ew;
              if (u == v) {
                  volume += ew;
              }
            });
        }
    }
    return std::make_pair(cut, volume);
}

std::set<node> MQI::expandOneCommunity(const std::set<node> &s) {
    std::set<node> sink_side = s;

    size_t iteration = 0;
    while (true) {
        double cut, volume;
        std::tie(cut, volume) = ComputeCutAndVolume(*g, sink_side);
        INFO("MQI iteration ", ++iteration, " cut ", cut, " volume ", volume, " community size ", sink_side.size());

        std::vector<node> local_to_global(sink_side.begin(), sink_side.end());
        std::vector<node> global_to_local(g->upperNodeIdBound(), none);
        for (node i = 0; i < local_to_global.size(); ++i) {
            global_to_local[local_to_global[i]] = i;
        }

        Graph flow_network(local_to_global.size() + 2, true, true, false);
        node source = flow_network.numberOfNodes() - 1;
        node sink = source - 1;
        node counter = 0;
        size_t cut_counter = 0;
        for (node u_global : local_to_global) {
            const node u_local = global_to_local[u_global];
            assert(counter == u_local);
            counter++;
            index incoming_cut = 0;
            g->forNeighborsOf(u_global, [&](node v_global) {
              const node v_local = global_to_local[v_global];
              if (v_local != none) {
                  // assumes that the opposite edge is inserted too
                  flow_network.addPartialOutEdge(Unsafe{}, u_local, v_local, volume);
              } else {
                  cut_counter++;
                  incoming_cut++;
              }
            });

            if (incoming_cut > 0) {
                flow_network.addPartialOutEdge(Unsafe{}, source, u_local, volume * incoming_cut);
                flow_network.addPartialOutEdge(Unsafe{}, u_local, source, 0.0);
            }

            edgeweight node_volume = g->weightedDegree(u_global, true);
            flow_network.addPartialOutEdge(Unsafe{}, u_local, sink, cut * node_volume);
            flow_network.addPartialOutEdge(Unsafe{}, sink, u_local, 0.0);
        }

        assert(cut_counter == cut);

        flow_network.indexEdges();

        Aux::Timer timer;
        timer.start();

        PushRelabel max_flow(flow_network, source, sink);
        max_flow.run();
        timer.stop();
        INFO("flow algo finished. flow = ", max_flow.getMaxFlow(), " cut * volume = ", cut * volume, " took ", timer.elapsedTag());
        if (max_flow.getMaxFlow() == cut * volume) {  // TODO floating point imprecision?
            break;
        }


        std::vector<node> source_side_sparse = max_flow.getSourceSet();
        INFO("Source set size ", source_side_sparse.size());
        for (node x : source_side_sparse) {
            assert(x != sink);
            if (x != source) {
                sink_side.erase(local_to_global[x]);
            }
        }

        /*
                std::vector<node> sink_side_vec = max_flow.getSinkSet();
                std::set<node> sink_side2;
                for (node x : sink_side_vec) {
                    if (x != sink) sink_side2.emplace(local_to_global[x]);
                }
                INFO("Sink set size " , sink_side2.size());
                assert(sink_side2 == sink_side);

         */





    }

    return sink_side;
}

}
