networkit_add_module(edgescores
    ChibaNishizekiQuadrangleEdgeScore.cpp
    ChibaNishizekiTriangleEdgeScore.cpp
    EdgeScore.cpp
    EdgeScoreAsWeight.cpp
    EdgeScoreBlender.cpp
    EdgeScoreLinearizer.cpp
    EdgeScoreNormalizer.cpp
    GeometricMeanScore.cpp
    PrefixJaccardScore.cpp
    TriangleEdgeScore.cpp
	PersonalizedPageRankJaccardScore.cpp
    )

networkit_module_link_modules(edgescores
    auxiliary graph)

add_subdirectory(test)
