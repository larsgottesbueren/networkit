#include <networkit/scd/MQI.hpp>

#include <networkit/auxiliary/NumericTools.hpp>
#include <networkit/flow/EdmondsKarp.hpp>
#include <networkit/flow/PushRelabel.hpp>
#include <networkit/scd/SetConductance.hpp>

#include <networkit/auxiliary/SignalHandling.hpp>

#include <queue>

namespace NetworKit {

MQI::MQI(const Graph &g) :
      SelectiveCommunityDetector(g) {
    graph_volume = g.totalEdgeWeight() * 2;
}

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

edgeweight GetConductance(const Graph& g, const std::set<node>& s) {
    SetConductance cond(g, s);
    cond.run();
    return cond.getConductance();
}

std::set<node> MQI::expandOneCommunity(const std::set<node> &s) {
    std::set<node> sink_side = s;

    edgeweight prev_conductance = GetConductance(*g, sink_side);

    size_t iteration = 0;
    Aux::SignalHandler handler;
    while (true) {
        handler.assureRunning();
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
        edgeweight cut_counter = 0;
        edgeweight at_source = 0.0, at_sink = 0.0;
        for (node u_global : local_to_global) {
            const node u_local = global_to_local[u_global];
            assert(counter == u_local);
            counter++;
            edgeweight incoming_cut = 0;
            g->forNeighborsOf(u_global, [&](node, node v_global, edgeweight w) {
              const node v_local = global_to_local[v_global];
              if (v_local != none) {
                  // assumes that the opposite edge is inserted too
                  flow_network.addPartialOutEdge(Unsafe{}, u_local, v_local, volume);
              } else {
                  cut_counter += w;
                  incoming_cut += w;
              }
            });

            if (incoming_cut > 0) {
                at_source += volume * incoming_cut;
                flow_network.addPartialOutEdge(Unsafe{}, source, u_local, volume * incoming_cut);
                flow_network.addPartialOutEdge(Unsafe{}, u_local, source, 0.0);
            }

            edgeweight node_volume = g->weightedDegree(u_global, true);
            at_sink += cut * node_volume;
            flow_network.addPartialOutEdge(Unsafe{}, u_local, sink, cut * node_volume);
            flow_network.addPartialOutEdge(Unsafe{}, sink, u_local, 0.0);
        }

        assert(cut_counter == cut);
        INFO("at source = ", at_source, " at sink = ", at_sink, " difference = ", at_source - at_sink);
        assert(Aux::NumericTools::equal(at_source, at_sink, 1e-4));


        flow_network.indexEdges();

        Aux::Timer timer;
        timer.start();

        PushRelabel max_flow(flow_network, source, sink);
        // EdmondsKarp max_flow(flow_network, source, sink);

        max_flow.run();
        timer.stop();
        INFO("flow algo finished. flow = ", max_flow.getMaxFlow(), " cut * volume = ", cut * volume, " took ", timer.elapsedTag());
        if (Aux::NumericTools::equal(max_flow.getMaxFlow(), cut * volume, 1e-8) ) {
            break;
        }


        std::vector<node> source_side_sparse = max_flow.getSourceSet();
        size_t removed = 0;
        INFO("Source set size ", source_side_sparse.size());
        for (node x : source_side_sparse) {
            assert(x != sink);
            if (x != source) {
                sink_side.erase(local_to_global[x]);
                removed++;
            }
            if (x == sink) {
                ERROR("sink reached");
            }
        }

        if (removed == 0) {
            INFO("No nodes removed --> terminate");
            break;
        }

#if false
        std::vector<node> sink_side_vec = max_flow.getSinkSet();
        std::set<node> sink_side2;
        for (node x : sink_side_vec) {
            if (x != sink) sink_side2.emplace(local_to_global[x]);
            if (x == source) ERROR("source reached");
        }
        INFO("Sink set size " , sink_side2.size());
        assert(sink_side2 == sink_side);    // this one doesnt have to hold.
#endif

        edgeweight next_conductance = GetConductance(*g, sink_side);
        if (next_conductance > prev_conductance) {
            std::tie(cut, volume) = ComputeCutAndVolume(*g, sink_side);
            INFO("cut = ", cut, " volume = ", volume);
            ERROR("prev conductance = ", prev_conductance, " next conductance = ", next_conductance);
            // throw std::runtime_error("MQI increased conductance");
        }
        if (next_conductance == prev_conductance) {
            ERROR("MQI kept conductance the same, but didn't terminate ", prev_conductance, next_conductance);
        }
        prev_conductance = next_conductance;
    }

    return sink_side;
}

}
