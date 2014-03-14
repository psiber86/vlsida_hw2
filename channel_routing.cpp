// Eric Carver
// Thu Mar 13 14:13:38 EDT 2014

#include <vector>
#include <map>
#include <algorithm>

#include "Cell.h"
#include "channel_routing.hpp"

#define TERMINALS_PER_CELL 4

channel_router::channel_router(std::vector<Cell> cells) : cells(cells) {
  for (auto &cell : cells) {
    for (int i=0; i < TERMINALS_PER_CELL; i++) {
      if (cell.nets.count(i)) {
        rows[cell.getCellY()].push_back(node(cell.termXY[i][0], cell.termXY[i][1], cell.nets[i]));
      }
    }
  }
}

int channel_router::route(const int top_index, const int bottom_index) {
  std::vector<int, zero_allocator<int> > top;
  std::sort(rows[top_index].begin(), rows[top_index].end());
  for (auto &node : rows[top_index]) {
    top[node.x] = node.net;
  }
  std::vector<int, zero_allocator<int> > bottom;
  std::sort(rows[bottom_index].begin(), rows[bottom_index].end());
  for (auto &node : rows[bottom_index]) {
    bottom[node.x] = node.net;
  }

  return this->route(top, bottom);
}

int channel_router::route(const std::vector<int, zero_allocator<int> >& top, const std::vector<int, zero_allocator<int> >& bottom)
{
}
