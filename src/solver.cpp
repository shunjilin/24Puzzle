#include "tile_node.hpp"
#include "manhattan_distance_heuristic.hpp"
#include "search.hpp"
#include "astar.hpp"
#include "idastar.hpp"
#include "open_array.hpp"
#include "closed_open_address_pool.hpp"
#include "tabulation.hpp"
#include "cxxopts.hpp"
#include "steady_clock_timer.hpp"
#include <array>
#include <string>
#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace Tiles;

int const WIDTH = 4;
int const HEIGHT = 4;
int const N_TILES = WIDTH*HEIGHT;

using Node = TileNode<WIDTH, HEIGHT>;
using Heuristic = ManhattanDistanceHeuristic<WIDTH, HEIGHT>;
using HashFunction = TabulationHash<Node, WIDTH*HEIGHT>;
size_t const ClosedEntries = 512927357;
int const MaxMoves = 100;

using DefaultAStar = AStar<Node, Heuristic, HashFunction>;
using AStarPool = AStar<Node, Heuristic, HashFunction,
                        ClosedOpenAddressPool<Node, HashFunction, ClosedEntries>,
                        OpenArray<Node, MaxMoves> >;

int main(int argc, char *argv[]) {

    cxxopts::Options options("Sliding Tiles Puzzle Search",
                             "Search algorithms for solving the sliding tiles puzzle.");
    // command line options
    options.add_options("Search")
        ("d,domain", "domain",
         cxxopts::value<std::string>()->default_value("tiles"))
        ("i,initial_state", "initial state configuration "
         "e.g. \"0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15\"",
         cxxopts::value<std::string>()->default_value("null")) // prevent segfault
        ("s,search_algorithm", "search algorithm [astar, astar_pool, idastar]",
         cxxopts::value<std::string>()->default_value("astar"))
        ("h,help", "print help");

    // parse command line
    auto result = options.parse(argc, argv);

    if (result.count("h")) {
        std::cout << options.help({"", "Search"}) << std::endl;
        return EXIT_SUCCESS;
    }

    auto timer = SteadyClockTimer();
    timer.start();

    // initial state
    std::array<uint8_t, N_TILES> initial_tiles;

    auto initial_tiles_string = result["initial_state"].as<std::string>();
    std::istringstream iss(initial_tiles_string);

    try {
        // read tiles
        // for generic node perhaps modify constructor to take in string
        for (int i = 0; i < N_TILES; ++i) {
            std::string tile_string;
            iss >> tile_string;
            initial_tiles[i] = std::stoi(tile_string);
        }

        // initial state
        auto initial_node = Node(initial_tiles);

        // search algorithm
        auto search_string = result["search_algorithm"].as<std::string>();
        std::unique_ptr<Search<Node> > search_algo;

        if (search_string == "astar") {
            search_algo = std::make_unique<DefaultAStar>();
        } else if (search_string == "astar_pool") {
            search_algo = std::make_unique<AStarPool>();
        } else if (search_string == "idastar") {
            search_algo = std::make_unique<IDAStar<Node, Heuristic>>();
        } else {
            std::cerr << "Invalid search algorithm option: "
                      << "\"" << search_string << "\"\n";
            return EXIT_FAILURE;
        }

        std::cout <<  timer.getElapsedTime<milliseconds>()
                  << " ms to initialize\n";

        auto path = search_algo->search(initial_node);

        std::cout << timer.getElapsedTime<milliseconds>()
                  << " ms to solve (including initialization)\n"
                  << *search_algo << "\n"
                  << "n moves: " << path.size() - 1 << "\n"
                  << "sequence:\n";
        for (auto node : path) {
            std::cout << node << "\n";
        }
    }
    // handle errors
    catch (const std::invalid_argument& ia) {
        std::cerr << "Invalid argument for initial state: "
                  << ia.what() << " : "
                  <<  "\"" << initial_tiles_string << "\"\n";
        return EXIT_FAILURE;
    }
    catch (const std::bad_alloc& ba) {
        std::cerr << "Memory allocation failed : " << ba.what() << "\n";
    }
    catch (const std::exception &e) {
        std::cerr << "Search failed: " << e.what() << "\n";
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
