#include <networkit/edgescores/EdgeScoreAsWeight.hpp>
#include <networkit/scd/EdgeScoreThenSCD.hpp>

namespace NetworKit {

EdgeScoreThenSCD::EdgeScoreThenSCD(const Graph &g, SelectiveCommunityDetector &scd,
                 EdgeScore<double> &edge_score)
    : SelectiveCommunityDetector(g), scd(scd), edge_score(edge_score) {
    edge_score.run();
    EdgeScoreAsWeight edge_score_as_weight(g, edge_score.scores(), false, 0, 1);
    reweighted_graph = edge_score_as_weight.calculate();
    scd.setGraph(&reweighted_graph);
    // TODO beware use-after-free. The SCD object will live beyond this EdgeScoreThenSCD object
    // but it can't be used anymore since the graph is gone.
    // For our use-case right now, it's fine, since that doesn't happen.
    // It's simply a flaw in the interface's capabilities
}

std::set<node> EdgeScoreThenSCD::expandOneCommunity(node s) {
    return scd.expandOneCommunity(s);
}

std::set<node> EdgeScoreThenSCD::expandOneCommunity(const std::set<node> &s) {
    return scd.expandOneCommunity(s);
}

} // namespace NetworKit
