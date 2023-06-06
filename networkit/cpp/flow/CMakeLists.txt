networkit_add_module(flow
    EdmondsKarp.cpp
	PushRelabel.cpp
    )

networkit_module_link_modules(flow
        graph)

add_subdirectory(test)
