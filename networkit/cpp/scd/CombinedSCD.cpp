#include <networkit/scd/CombinedSCD.hpp>

namespace NetworKit {

CombinedSCD::CombinedSCD(const Graph &g, SelectiveCommunityDetector &first,
                         SelectiveCommunityDetector &second)
    : SelectiveCommunityDetector(g), first(first), second(second) {}

std::set<node> CombinedSCD::expandOneCommunity(node s) {
    return second.expandOneCommunity(first.expandOneCommunity(s));
}

std::set<node> CombinedSCD::expandOneCommunity(const std::set<node> &s) {
    return second.expandOneCommunity(first.expandOneCommunity(s));
}

void CombinedSCD::setGraph(const Graph& new_g) {
    first.setGraph(new_g);
    second.setGraph(new_g);
}

} // namespace NetworKit
