#ifndef TILE_NODE_HPP
#define TILE_NODE_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <iomanip>
#include <limits>
#include <numeric>
#include <optional>
#include <ostream>

namespace Tiles {

template <int WIDTH, int HEIGHT>
bool isValidBoard(std::array<uint8_t, WIDTH * HEIGHT> const &board) {
  // check if valid input : TODO: check parity
  for (char i = 0; i < WIDTH * HEIGHT; ++i) {
    if (std::find(board.begin(), board.end(), i) == board.end()) {
      return false;
    }
  }
  return true;
}

// board moves; blank moved in the specified direction
enum MOVE : uint8_t {
  DOWN,
  LEFT,
  RIGHT,
  UP,
  N_MOVES,
  NONE // best ordering for 15 puzzle
};

template <int WIDTH, int HEIGHT> struct TileNode {

  // goal board configuration
  static TileNode<WIDTH, HEIGHT> goal_node;

  // tile values, indexed in row-major order
  // e.g. 24 puzzle:
  /* 0  1  2  3  4
     5  6  7  8  9
     10 11 12 13 14
     15 16 17 18 19
     20 21 22 23 24 */
  std::array<uint8_t, WIDTH * HEIGHT> board;

  // index of blank
  uint8_t blank_idx = std::numeric_limits<uint8_t>::max();

  // caching to prevent regeneration of parent node
  MOVE prev_move = NONE;

  // cost and heuristic value of node
  uint8_t g_val = 0;
  uint8_t h_val = std::numeric_limits<uint8_t>::max();

  TileNode() = default; // sentinel value;

  // construct node from array of tiles
  TileNode(std::array<uint8_t, WIDTH * HEIGHT> board)
      : board(std::move(board)) {
    if (!isValidBoard<WIDTH, HEIGHT>(board)) {
      throw std::invalid_argument("invalid initial board configuration");
    }
    blank_idx = getBlankIdx();
  }

  static void setGoalBoard(std::array<uint8_t, WIDTH * HEIGHT> goal_board) {
    if (!isValidBoard<WIDTH, HEIGHT>(goal_board)) {
      throw std::invalid_argument("invalid goal board configuration");
    }
    goal_node.board = goal_board;
  }

  // get index of current blank tile
  uint8_t getBlankIdx() noexcept {
    // not cached, do linear scan
    if (blank_idx == std::numeric_limits<uint8_t>::max()) {
      auto blank_iter = std::find(board.begin(), board.end(), 0);
      blank_idx = std::distance(board.begin(), blank_iter);
    }
    return blank_idx;
  }

  // swap blank tile with new blank tile to get new tile node
  TileNode<WIDTH, HEIGHT> swapBlank(uint8_t new_blank_idx) const noexcept {
    auto new_node = *this; // copy
    std::swap(new_node.board[blank_idx], new_node.board[new_blank_idx]);
    new_node.blank_idx = new_blank_idx; // cache blank idx
    return new_node;
  }

  // get new node from moving blank in direction move
  // cache previous move and increments g_val
  std::optional<TileNode<WIDTH, HEIGHT>> moveBlank(MOVE move) const noexcept {
    std::optional<TileNode<WIDTH, HEIGHT>> new_node;
    switch (move) {
    case UP:
      if (blank_idx >= WIDTH) {
        new_node.emplace(swapBlank(blank_idx - WIDTH));
      }
      break;
    case DOWN:
      if (blank_idx < (WIDTH * (HEIGHT - 1))) {
        new_node.emplace(swapBlank(blank_idx + WIDTH));
      }
      break;
    case LEFT:
      if ((blank_idx % WIDTH) != 0) {
        new_node.emplace(swapBlank(blank_idx - 1));
      }
      break;
    case RIGHT:
      if ((blank_idx % WIDTH) != (WIDTH - 1)) {
        new_node.emplace(swapBlank(blank_idx + 1));
      }
      break;
    default:
      break;
    }
    if (new_node.has_value()) {
      new_node->prev_move = move; // cache previous move
      ++new_node->g_val;          // increment g value
    }
    return new_node;
  }

  // simple iterator
  using const_iterator =
      typename std::array<uint8_t, WIDTH * HEIGHT>::const_iterator;

  const_iterator begin() const noexcept { return board.begin(); }
  const_iterator end() const noexcept { return board.end(); }
};

// returns goal board, where value at each index = index
template <int WIDTH, int HEIGHT>
std::array<uint8_t, WIDTH * HEIGHT> getGoalBoard() noexcept {
  std::array<uint8_t, WIDTH * HEIGHT> tiles;
  std::iota(tiles.begin(), tiles.end(), 0);
  return tiles;
}

// static initialization of goal node
template <int WIDTH, int HEIGHT>
TileNode<WIDTH, HEIGHT> TileNode<WIDTH, HEIGHT>::goal_node =
    TileNode<WIDTH, HEIGHT>(getGoalBoard<WIDTH, HEIGHT>());

// free functions, loose coupling, take advantage of argument dependent
// lookup

template <int WIDTH, int HEIGHT>
bool operator==(TileNode<WIDTH, HEIGHT> const &lhs,
                TileNode<WIDTH, HEIGHT> const &rhs) noexcept {
  return lhs.board == rhs.board;
}

// get cost of path to node
template <int WIDTH, int HEIGHT>
int getG(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  return static_cast<int>(node.g_val);
}

// get heuristic value of node
template <int WIDTH, int HEIGHT>
int getH(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  return static_cast<int>(node.h_val);
}

// get g + h value
template <int WIDTH, int HEIGHT>
int getF(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  return getG(node) + getH(node);
}

// check if node is goal node
template <int WIDTH, int HEIGHT>
bool isGoal(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  return node == node.goal_node;
}

// get nodes that can be generated from current node
template <int WIDTH, int HEIGHT>
std::array<std::optional<TileNode<WIDTH, HEIGHT>>, N_MOVES>
getChildNodes(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  std::array<std::optional<TileNode<WIDTH, HEIGHT>>, N_MOVES> child_nodes;
  if (node.prev_move != UP) {
    child_nodes[DOWN] = node.moveBlank(DOWN);
  }
  if (node.prev_move != RIGHT) {
    child_nodes[LEFT] = node.moveBlank(LEFT);
  }
  if (node.prev_move != LEFT) {
    child_nodes[RIGHT] = node.moveBlank(RIGHT);
  }
  if (node.prev_move != DOWN) {
    child_nodes[UP] = node.moveBlank(UP);
  }
  return child_nodes;
}

template <int WIDTH, int HEIGHT>
std::optional<TileNode<WIDTH, HEIGHT>>
getParent(TileNode<WIDTH, HEIGHT> const &node) noexcept {
  if (node.prev_move == UP) {
    return node.moveBlank(DOWN);
  }
  if (node.prev_move == DOWN) {
    return node.moveBlank(UP);
  }
  if (node.prev_move == LEFT) {
    return node.moveBlank(RIGHT);
  }
  if (node.prev_move == RIGHT) {
    return node.moveBlank(LEFT);
  }
  return {};
}

// pretty print board
template <int WIDTH, int HEIGHT>
std::ostream &operator<<(std::ostream &os,
                         TileNode<WIDTH, HEIGHT> const &node) {
  for (int row = 0; row < HEIGHT; ++row) {
    for (int col = 0; col < WIDTH; ++col) {
      os << std::setw(2) << static_cast<int>(node.board[row * WIDTH + col])
         << " ";
    }
    os << "\n";
  }
  return os;
}
} // namespace Tiles

// overload default hash
namespace std {
template <int WIDTH, int HEIGHT> struct hash<Tiles::TileNode<WIDTH, HEIGHT>> {

  size_t operator()(const Tiles::TileNode<WIDTH, HEIGHT> &node) const noexcept {
    size_t result = 0;
    for (auto i = 0; i < WIDTH * HEIGHT; ++i) {
      result = result * 31 + hash<char>()(node.board[i]);
    }
    return result;
  }
};
} // namespace std

#endif
