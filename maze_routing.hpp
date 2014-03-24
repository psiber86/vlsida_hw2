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
  node(int x, int y, bool m1, bool m2, bool cell ) : x(x), y(y), m1(m1), m2(m2), cell(cell) {};
  int x;
  int y;
  bool m1;
  bool m2;
  bool cell;
};

template<typename T> class zero_allocator : public std::allocator<T> {
public:
  T* allocate(size_t size, const void* hint = 0) {
    T* mem = std::allocator<T>::allocate(size, hint);
    memset(mem, 0, sizeof(T));
    return mem;
  }
};

class channel_router {
public:
  channel_router(std::vector<Cell>, int);
  int route(const std::vector<int>&, const std::vector<int>&, const int);
  int route(const int, const int);
  int route_all();
  int get_num_nets() const;
  void write_mag_file(const std::string);
private:
  struct wires {
    bool left_up;
    bool right_up;
    std::pair<int,int> horizontal;
    friend bool operator<(const wires& left, const wires& right) {
      return ( left.horizontal.first < right.horizontal.first );
    }
  };
  void insert_net(std::vector<std::set<wires> >&, const int, const int, const bool, const bool);
  std::map<int,std::vector<node> > rows;
  std::vector<Cell> cells;
  std::map<int,std::vector<std::set<wires > > > routed_tracks;
  std::pair<std::map<int,int>,std::map<int,int> > calc_row_offsets() const;
  int num_nets;
};

#endif
