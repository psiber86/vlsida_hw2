// Kyle Craig

#ifndef MAZE_ROUTING_HPP_DEFINED
#define MAZE_ROUTING_HPP_DEFINED

#include <vector>
#include <memory>
#include <map>
#include <cstddef>
#include <set>
#include <utility>
#include <cstdlib>
#include <string>

#include "Cell.h"

struct gridcell {
  gridcell() : val(0), m1(0), m2(0), cell(0), term(0), checked(0), buf(0), m1buf(0), m2buf(0) {};
  int val;
  bool m1;
  bool m2;
  bool cell;
  bool term;
  bool buf;
  bool m1buf;
  bool m2buf;
  bool checked;
};

struct coord {
  coord(int x, int y) : x(x), y(y) {};
  int x;
  int y;
};

class maze_router {
public:
  maze_router(std::vector<Cell>, int, int, int, int**);
  int route();
  //void write_mag_file(const std::string);
private:
  struct wires {
    bool left_up;
    bool right_up;
    std::pair<int,int> horizontal;
    friend bool operator<(const wires& left, const wires& right) {
      return ( left.horizontal.first < right.horizontal.first );
    }
  };
  void create_grid(int, int);
  void print_grid(int, int);
  void reset_grid(int, int);
  int expand_grid(std::vector<Cell>&);
  bool lee_algorithm(int, int, int, int, int);
  bool check_neighbor(int, int, int);
  coord check_nvals(int, int, int, bool);
  std::vector<Cell> cells;
  std::map<int,std::vector<int>> rows;
  int num_nets;
  int crows;
  int ccols;
  gridcell** grid;
  int** nets;
};

#endif
