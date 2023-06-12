#include <gtest/gtest.h>

#include <networkit/auxiliary/Random.hpp>
#include <networkit/io/METISGraphReader.hpp>
#include <networkit/structures/Partition.hpp>
#include <networkit/scd/PageRankNibble.hpp>
#include <networkit/scd/MQI.hpp>
#include <networkit/scd/CombinedSCD.hpp>
#include <networkit/auxiliary/SignalHandling.hpp>

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

    void run(const std::string& path) {
        METISGraphReader reader;
        // graph = reader.read(path);

        INFO("n = ", graph.numberOfNodes(), " n* = ",  graph.upperNodeIdBound(), " m = ", graph.numberOfEdges());
        PageRankNibble prn(graph, 0.1, 0.0001);
        MQI mqi(graph);
        CombinedSCD prn_then_mqi(graph, prn, mqi);
        prn_then_mqi.run({0});
    }

    Graph graph;
};

TEST_F(ExpanderDecompositionBenchmark, bench) {
    run("..");
    std::exit(0);

    std::vector<std::string> paths;
    std::string path_file = "paths.txt";
    std::ifstream file(path_file);
    std::string line;
    while (std::getline(file, line)) {
        paths.push_back(line);
    }
    Aux::SignalHandler handler;
    for (const auto& path : paths) {
        INFO("running on graph = ", path);
        run(path);
        handler.assureRunning();
    }

}

} /* namespace NetworKit */
