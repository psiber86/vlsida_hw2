// Eric Carver
// Thu Mar 13 14:13:38 EDT 2014

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <iostream>
#include <set>

#include "Cell.h"
#include "channel_routing.hpp"

#define TERMINALS_PER_CELL 4

channel_router::channel_router(std::vector<Cell> cells) : cells(cells) {
  for (auto &cell : cells) {
    for (int i=0; i < TERMINALS_PER_CELL; i++) {
      if (cell.nets.count(i)) {
        rows[cell.termXY[i][1]].push_back(node(cell.termXY[i][0], cell.termXY[i][1], cell.nets[i]));
      }
    }
  }
}

int channel_router::route(const int top_index, const int bottom_index) {
#ifdef DEBUG
  std::cout << "Routing rows at " << top_index << " and " << bottom_index << std::endl;
#endif

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

int find_rightmost(const std::vector<int, zero_allocator<int> >& terminals, int net) {
  for (int i = terminals.size(); i >= 0; i--) {
    if ( terminals[i] == net ) {
      return i;
    }
  }
  return 0;
}

void channel_router::insert_net(std::vector<std::set<std::pair<int,int> > >& tracks, const int net_left, const int net_right)
{
  // This is messy as fuck; is there a better way?
  std::set<std::pair<int,int> >* candidate_track;
  // Search all the tracks for one that can hold the net
  bool need_new_track = false;
  for (auto &track : tracks) {
    candidate_track = &track;
    bool found_track = true;
    for (auto &net : track) {
      if ( net.first < net_right || net.second > net_left ) {
        // The nets overlap
        found_track = false;
        break;
      }
    }
    if ( found_track ) {
      // We didn't find a track; we ran out of tracks
      need_new_track = true;
      break;
    }
  }
  if ( need_new_track ) {
    tracks.push_back(std::set<std::pair<int,int> >());
    candidate_track = &tracks.back();
  }
  candidate_track->insert(std::make_pair(net_left, net_right));
}

inline int greater(const int a, const int b) {
  if ( a > b ) return a;
  else return b;
}

int channel_router::route(const std::vector<int, zero_allocator<int> >& top, const std::vector<int, zero_allocator<int> >& bottom)
{
  std::vector<std::pair<int,int>> vcg;
  std::vector<std::set<std::pair<int,int> > > tracks;

  int width = greater(top.size(), bottom.size());

  // Place all the horizontal portions of the nets into tracks. Basically, implement the unconstrained
  // left-edge algorithm
  for (int i=0; i < width; i++) {
    if ( bottom[i] ) {
      int net_left = bottom[i];
      int rightmost_bottom = find_rightmost(bottom, net_left);
      int rightmost_top = find_rightmost(top, net_left);
      this->insert_net(tracks, net_left, greater(rightmost_bottom, rightmost_top));
    }
    if ( top[i] ) {
      int net_left = top[i];
      int rightmost_bottom = find_rightmost(bottom, net_left);
      int rightmost_top = find_rightmost(top, net_left);
      this->insert_net(tracks, net_left, greater(rightmost_bottom, rightmost_top));
    }
  }
#ifdef DEBUG
  std::cout << "Tracks used: " << tracks.size() << std::endl;
#endif
  return tracks.size();
}

int channel_router::route_all() {

}
