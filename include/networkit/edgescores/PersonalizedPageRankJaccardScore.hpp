#ifndef NETWORKIT_EDGESCORES_PREFIX_JACCARD_SCORE_HPP_
#define NETWORKIT_EDGESCORES_PREFIX_JACCARD_SCORE_HPP_

#include <networkit/edgescores/EdgeScore.hpp>

namespace NetworKit {

class PersonalizedPageRankJaccardScore final : public EdgeScore<double> {

public:
    PersonalizedPageRankJaccardScore(const Graph &G, double alpha, double eps);
    double score(edgeid eid) override;
    double score(node u, node v) override;
    void run() override;

private:
    double alpha, eps;
};

} // namespace NetworKit

#endif // NETWORKIT_EDGESCORES_PREFIX_JACCARD_SCORE_HPP_
