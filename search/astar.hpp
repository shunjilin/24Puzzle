#ifndef ASTAR_HPP
#define ASTAR_HPP

#include <memory>
#include <vector>

template <typename Node, typename Open, typename Closed>
struct AStar {

    // perform astar search: lazy with reopenings
    // return path to solution
    std::vector<Node>
    search(Node initial_node) {

        Open open;
        Closed closed;
        
        open.push(std::move(initial_node));

        while (!open.isEmpty()) {
            auto node = open.pop();
            if (!closed.isDuplicate(node)) {
                // check goal node
                
                auto child_nodes = node.getChildNodes();
                for (auto child_node : child_nodes) {
                    if (child_node.has_value()) {
                        open.push(child_node);
                    }
                }
            }
            
        }
	return std::vector<Node>();
    }
};

#endif
