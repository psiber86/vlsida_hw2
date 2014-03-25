// Eric Carver
// Thu Mar 13 16:53:18 EDT 2014

#ifndef CHANNEL_ROUTING_HPP_DEFINED
#define CHANNEL_ROUTING_HPP_DEFINED

#include <vector>
#include <memory>
#include <map>
#include <cstddef>
#include <set>
#include <utility>
#include <cstdlib>
#include <string>

#include "Cell.h"

struct node {
  node(int x, int y, int net) : x(x), y(y), net(net) {};
  int x;
  int y;
  int net;
  friend bool operator<(const node& left, const node& right) {
    return ( left.x < right.x );
  }
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
  int route(std::vector<int>&, std::vector<int>&, const int, std::vector<std::set<int> >&);
  int route(const int, const int);
  int route_all();
  int get_num_nets() const;
  void write_mag_file(const std::string);
  void print_net_stats() const;
private:
  struct wires {
    bool left_up;
    bool right_up;
    std::pair<int,int> horizontal;
    friend bool operator<(const wires& left, const wires& right) {
      return ( left.horizontal.first < right.horizontal.first );
    }
  };
  void insert_net(std::vector<std::set<wires> >&, const int, const int, const bool, const bool, int&);
  // enum region_fill { EMPTY = 0, HORIZONTAL_WIRE, VERTICAL_WIRE, HORIZONTAL_AND_VERTICAL_WIRE, BUFFER_SPACE };
  // std::vector<std::vector<int> > grid;
  std::map<int,std::vector<node> > rows;
  std::vector<Cell> cells;
  std::map<int,std::vector<std::set<wires > > > routed_tracks;
  std::pair<std::map<int,int>,std::map<int,int> > calc_row_offsets() const;
  std::vector<std::set<int> > construct_vcg(const std::vector<int>&, const std::vector<int>&) const;
  inline void delete_from_vcg(const int, std::vector<std::set<int> >&) const;

  int num_nets;
  int stranded_nets;
  int unroutable_nets;
  int max_net_num;
  int bumps;
  int cyclical_nets;
  int wire_length;
};

#endif
