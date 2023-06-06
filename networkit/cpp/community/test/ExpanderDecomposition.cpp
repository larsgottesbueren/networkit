#include <gtest/gtest.h>

#include <networkit/auxiliary/Random.hpp>
#include <networkit/io/METISGraphReader.hpp>
#include <networkit/structures/Partition.hpp>
#include <networkit/scd/PageRankNibble.hpp>
#include <networkit/scd/MQI.hpp>
#include <networkit/scd/CombinedSCD.hpp>


namespace NetworKit {


class ExpanderDecompositionBenchmark : public testing::Test {
public:
    void SetUp() {
        Aux::Random::setSeed(435913, false);
    }

    ExpanderDecompositionBenchmark() {
        std::string graphPath;
        if(const char* env_p = std::getenv("GRAPH_PATH")) {
            graphPath = std::string(env_p);
        } else {
            std::cout << "[INPUT] graph file path (metis format) > " << std::endl;
            std::getline(std::cin, graphPath);
        }

        METISGraphReader reader;
        graph = reader.read(graphPath);
    }

    void run() {
        INFO("n = ", graph.numberOfNodes(), " n* = ",  graph.upperNodeIdBound(), " m = ", graph.numberOfEdges());
        PageRankNibble prn(graph, 0.1, 0.0001);
        MQI mqi(graph);
        CombinedSCD prn_then_mqi(graph, prn, mqi);
        prn_then_mqi.run({0});
    }

    Graph graph;
};

TEST_F(ExpanderDecompositionBenchmark, bench) {
    run();
}

} /* namespace NetworKit */
