#include <networkit/edgescores/PersonalizedPageRankJaccardScore.hpp>
#include <networkit/auxiliary/SignalHandling.hpp>
#include <networkit/scd/ApproximatePageRank.hpp>

namespace NetworKit {

PersonalizedPageRankJaccardScore::PersonalizedPageRankJaccardScore(const Graph &G, double alpha, double eps)
    : EdgeScore<double>(G), alpha(alpha), eps(eps) {}

void PersonalizedPageRankJaccardScore::run() {
    if (!G->hasEdgeIds())
        throw std::runtime_error("Error, edges need to be indexed first");
    Aux::SignalHandler handler;
    handler.assureRunning();

    scoreData.assign(G->upperEdgeIdBound(), 0.0);

    std::vector< std::vector<std::pair<node, double>> > pprs(G->upperNodeIdBound());
    G->balancedParallelForNodes([&](node u) {
      ApproximatePageRank ppr_algo(*G, alpha, eps);
      pprs[u] = ppr_algo.run(u);
      std::sort(pprs[u].begin(), pprs[u].end());    // for intersection with weighted Jaccard
    });

    G->parallelForEdges([&](node u, node v, edgeid e_id) {
        const auto& p = pprs[u];
        const auto& q = pprs[v];
        double numerator = 0.0, denominator = 0.0;
        size_t l = 0, r = 0;
        while (l < p.size() && r < q.size()) {
            if (p[l].first == q[r].first) {
                numerator += std::min(p[l].second, q[r].second);
                denominator += std::max(p[l].second, q[r].second);
                l++; r++;
            } else if (p[l].first < q[r].first) {
                numerator += p[l].second;
                denominator += p[l].second;
                l++;
            } else {
                numerator += q[r].second;
                denominator += q[r].second;
                r++;
            }
        }
        while (l < p.size()) {
            numerator += p[l].second;
            denominator += p[l].second;
            l++;
        }
        while (r < q.size()) {
            numerator += q[r].second;
            denominator += q[r].second;
            r++;
        }
        scoreData[e_id] = numerator / denominator;
    });

    hasRun = true;
}

double PersonalizedPageRankJaccardScore::score(node, node) {
    throw std::runtime_error("Not implemented: Use scores() instead.");
}

double PersonalizedPageRankJaccardScore::score(edgeid) {
    throw std::runtime_error("Not implemented: Use scores() instead.");
}

} // namespace NetworKit
