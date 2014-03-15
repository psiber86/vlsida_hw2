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
#include "PR.h"

#define TERMINALS_PER_CELL 4

channel_router::channel_router(std::vector<Cell> cells, int num_nets) : cells(cells), num_nets(num_nets) {
  for (auto &cell : cells) {
    for (int i=0; i < TERMINALS_PER_CELL; i++) {
      const int* nets = cell.getNets();
      if ( nets[i] ) {
        rows[cell.termXY[i][1]].push_back(node(cell.termXY[i][0], cell.termXY[i][1], nets[i]));
      }
    }
  }
}

int channel_router::route(const int top_index, const int bottom_index) {
#ifdef DEBUG
  std::cout << "Routing rows at " << top_index << " and " << bottom_index << std::endl;
#endif

  std::vector<int> top;
  std::sort(rows[top_index].begin(), rows[top_index].end());
  for (auto &node : rows[top_index]) {
    // According to Josh, his coordinates are never negative
    for (int i = top.size()-1; i < node.x; i++) {
      top.push_back(0);
    }
    top[node.x] = node.net;
  }
  std::vector<int> bottom;
  std::sort(rows[bottom_index].begin(), rows[bottom_index].end());
  for (auto &node : rows[bottom_index]) {
    for (int i = bottom.size()-1; i < node.x; i++) {
      bottom.push_back(0);
    }
    bottom[node.x] = node.net;
  }

  // Make the vectors the same length
  for (int i = top.size(); i < bottom.size(); i++) {
    top.push_back(0);
  }
  for (int i = bottom.size(); i < top.size(); i++) {
    bottom.push_back(0);
  }

  return this->route(top, bottom);
}

int find_rightmost(const std::vector<int>& terminals, int net) {
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
  // Search all the tracks for one that can hold the net
  bool need_new_track = false;
  for (auto &track : tracks) {
    bool found_track = true;
    for (auto &net : track) {
      if ( net.first < net_right || net.second > net_left ) {
        // The nets overlap
        found_track = false;
        break;
      }
    }
    if ( !found_track ) {
      // We didn't find a track; we ran out of tracks
      need_new_track = true;
      break;
    }
    else {
      track.insert(std::make_pair(net_left, net_right));
    }
  }
  if ( need_new_track ) {
    tracks.push_back(std::set<std::pair<int,int> >());
    tracks.back().insert(std::make_pair(net_left, net_right));
  }
}

int greater(const int a, const int b) {
  if ( a > b ) return a;
  else return b;
}

int channel_router::route(const std::vector<int>& top, const std::vector<int>& bottom)
{
  std::vector<std::pair<int,int>> vcg;
  std::vector<std::set<std::pair<int,int> > > tracks;
  std::vector<int> routed_nets;
  tracks.resize(1); // Need at least one track to start

  int width = greater(top.size(), bottom.size());

  int terminals_routed = 0;

  // Place all the horizontal portions of the nets into tracks. Basically, implement the unconstrained
  // left-edge algorithm
#ifdef DEBUG
  std::cout << "Routing nets: ";
#endif
  for (int i=0; i < width; i++) {
    if ( bottom[i] && (std::find(routed_nets.begin(), routed_nets.end(), bottom[i]) == routed_nets.end()) ) {
      std::cout << bottom[i] << ' ';
      int net_left = bottom[i];
      int rightmost_bottom = find_rightmost(bottom, net_left);
      int rightmost_top = find_rightmost(top, net_left);
      routed_nets.push_back(bottom[i]);
      this->insert_net(tracks, net_left, greater(rightmost_bottom, rightmost_top));
      ++terminals_routed;
    }
    if ( top[i] && (std::find(routed_nets.begin(), routed_nets.end(), top[i]) == routed_nets.end()) ) {
      std::cout << top[i] << ' ';
      int net_left = top[i];
      int rightmost_bottom = find_rightmost(bottom, net_left);
      int rightmost_top = find_rightmost(top, net_left);
      routed_nets.push_back(top[i]);
      this->insert_net(tracks, net_left, greater(rightmost_bottom, rightmost_top));
      ++terminals_routed;
    }
  }
#ifdef DEBUG
  std::cout << std::endl << "Tracks used: " << tracks.size() << std::endl;
#endif
  return terminals_routed;
}

int channel_router::route_all() {
  int num_routed = 0;
  // Iterate starting with the second row
  std::map<int,std::vector<node> >::iterator iter = this->rows.begin();
  for (++iter; iter != this->rows.end(); ++iter) {
    int bottom_row = iter->first;
    if ( ++iter == this->rows.end() || next(iter) == this->rows.end() ) {
      // Skip the last row too
      break;
    }
    num_routed += this->route(iter->first, bottom_row);
  }
  return num_routed;
}

int channel_router::get_num_nets() const
{
  int nets = this->num_nets;
  for (auto &cell : this->cells) {
    if ( cell.getCellWidth() == 3 ) {
      nets++;
    }
  }
  return nets;
}
