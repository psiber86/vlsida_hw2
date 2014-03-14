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
  channel_router(std::vector<Cell>);
  int route(const std::vector<int>&, const std::vector<int>&);
  int route(const int, const int);
  int route_all();
private:
  void insert_net(std::vector<std::set<std::pair<int,int> > >&, const int, const int);
  enum region_fill { EMPTY = 0, HORIZONTAL_WIRE, VERTICAL_WIRE, HORIZONTAL_AND_VERTICAL_WIRE, BUFFER_SPACE };
  std::vector<std::vector<int> > grid;
  std::map<int,std::vector<node> > rows;
  std::vector<Cell> cells;
};

#endif
