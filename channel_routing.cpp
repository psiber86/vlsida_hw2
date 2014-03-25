// Eric Carver
// Thu Mar 13 14:13:38 EDT 2014

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <sstream>
#include <cassert>
#include <cmath>

#include "Cell.h"
#include "channel_routing.hpp"
#include "PR.h"

#define TERMINALS_PER_CELL 4

channel_router::channel_router(std::vector<Cell> cells, int max_net_num)
  : cells(cells), num_nets(0), stranded_nets(0), unroutable_nets(0),
    max_net_num(max_net_num), bumps(0) {
  for (auto &cell : cells) {
    if ( cell.getCellWidth() == 6 ) {
      switch (cell.getCellOrientation()) {
      case NORM:
	cell.resetTermCoords();
	break;
      case FLIPVERT:
	cell.flipVertCell();
	break;
      case FLIPHORZ:
	cell.flipHorzCell();
	break;
      case ROTATED:
	cell.rotateCell();
	break;
      default:
	throw "Invalid cell orientation";
	break;
      }
    }
    const int* nets = cell.getNets();
    for (int i=0; i < TERMINALS_PER_CELL; i++) {
      if ( nets[i] ) {
        rows[cell.termXY[i][1]].push_back(node(cell.termXY[i][0], cell.termXY[i][1], nets[i]));
        // row_mapping[cell.termXY[i][1]] = cell.getLambdaY();
      }
    }
  }
}

int channel_router::route(const int top_index, const int bottom_index) {
#ifdef DEBUG
  std::cout << "Routing rows at " << top_index << " and " << bottom_index << std::endl;
#endif

  int bottom_row;

  std::vector<int> top;
  if ( !top_index ) {
    // Need a fake top row
    top.push_back(0);
  }
  else {
    std::sort(rows[top_index].begin(), rows[top_index].end());
    for (auto &node : rows[top_index]) {
      // According to Josh, his coordinates are never negative
      for (int i = top.size()-1; i < node.x; i++) {
        top.push_back(0);
      }
      top[node.x] = node.net;
    }
  }
  std::vector<int> bottom;
  if ( !bottom_index ) {
    // Need a fake bottom row
    bottom.push_back(0);
    bottom_row = top_index-2;
  }
  else {
    std::sort(rows[bottom_index].begin(), rows[bottom_index].end());
    for (auto &node : rows[bottom_index]) {
      for (int i = bottom.size()-1; i < node.x; i++) {
        bottom.push_back(0);
      }
      bottom[node.x] = node.net;
    }
    bottom_row = bottom_index;
  }

  // Make the vectors the same length
  for (unsigned int i = top.size(); i < bottom.size(); i++) {
    top.push_back(0);
  }
  for (unsigned int i = bottom.size(); i < top.size(); i++) {
    bottom.push_back(0);
  }

  return this->route(top, bottom, bottom_row);
}

int find_rightmost(const std::vector<int>& terminals, int net) {
  for (int i = terminals.size()-1; i >= 0; i--) {
    if ( terminals[i] == net ) {
      return i;
    }
  }
  return 0;
}

void channel_router::insert_net(std::vector<std::set<wires > >& tracks, const int net_left, const int net_right,
                                const bool left_up, const bool right_up)
{
  // This is messy; is there a better way?
  // Search all the tracks for one that can hold the net
  wires this_net;
  this_net.left_up = left_up;
  this_net.right_up = right_up;
  // Handle a special case: a purely vertical net. This does not require us to find a track
  if ( net_left == net_right ) {
    this_net.horizontal = std::make_pair(net_left, net_right);
    tracks.back().insert(this_net);
  }
  else {
    if ( abs(net_right - net_left) > 1 ) {
      this->bumps += 2;
    }
    this_net.horizontal = std::make_pair(net_left, net_right);
    bool need_new_track = true;
    for (auto &track : tracks) {
      bool found_track = true;
      for (auto &net : track) {
        if ( !(net_right < net.horizontal.first-1 || net_left > net.horizontal.second+1) ) {
          // The nets overlap
          found_track = false;
          break;
        }
      }
      // if ( !found_track ) {
      //   // We didn't find a track; we ran out of tracks
      //   need_new_track = true;
      //   break;
      // }
      // else {
      //   track.insert(this_net);
      // }
      if ( found_track ) {
        track.insert(this_net);
        need_new_track = false;
        break;
      }
    }
    if ( need_new_track ) {
      tracks.push_back(std::set<wires>());
      tracks.back().insert(this_net);
    }
  }
}

int greater(const int a, const int b) {
  if ( a > b ) return a;
  else return b;
}

int channel_router::route(const std::vector<int>& top, const std::vector<int>& bottom, const int row_num)
{
  //  std::vector<std::pair<int,int>> vcg;
  std::vector<std::set<wires> > tracks;
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
    assert( bottom[i] <= this->max_net_num && top[i] <= this->max_net_num );
    if ( bottom[i] && (std::find(routed_nets.begin(), routed_nets.end(), bottom[i]) == routed_nets.end()) ) {
#ifdef DEBUG
      std::cout << bottom[i];
#endif
      ++num_nets;
      int net_left = i;
      int rightmost_bottom = find_rightmost(bottom, bottom[i]);
      int rightmost_top = find_rightmost(top, bottom[i]);
      routed_nets.push_back(bottom[i]);
      int net_right = greater(rightmost_bottom, rightmost_top);
      if ( net_right > net_left ) {
        this->insert_net(tracks, net_left, net_right, false, ( net_right == rightmost_top ));
#ifdef DEBUG
        std::cout << "(R)";
#endif
        ++terminals_routed;
      }
      else if ( net_right == net_left && rightmost_top != 0 ) {
        // A purely vertical net
        this->insert_net(tracks, net_left, net_right, true, false);
#ifdef DEBUG
        std::cout << "(R)";
#endif
        ++terminals_routed;
      }
      else {
#ifdef DEBUG
        std::cout << "(S)";
#endif
        ++stranded_nets;
      }
#ifdef DEBUG
      std::cout << ' ';
#endif
    }
    if ( top[i] && (std::find(routed_nets.begin(), routed_nets.end(), top[i]) == routed_nets.end()) ) {
#ifdef DEBUG
      std::cout << top[i];
#endif
      ++num_nets;
      int net_left = i;
      int rightmost_bottom = find_rightmost(bottom, top[i]);
      int rightmost_top = find_rightmost(top, top[i]);
      routed_nets.push_back(top[i]);
      int net_right = greater(rightmost_bottom, rightmost_top);
      if ( net_right > net_left ) {
        this->insert_net(tracks, net_left, net_right, true, ( net_right == rightmost_top ));
#ifdef DEBUG
        std::cout << "(R)";
#endif
        ++terminals_routed;
      }
      else if ( net_right == net_left && rightmost_bottom != 0) {
        // A purely vertical net
        this->insert_net(tracks, net_left, net_right, true, false);
#ifdef DEBUG
        std::cout << "(R)";
#endif
        ++terminals_routed;
      }
      else {
#ifdef DEBUG
        std::cout << "(S)";
#endif
        ++stranded_nets;
      }
#ifdef DEBUG
      std::cout << ' ';
#endif
    }
  }
#ifdef DEBUG
  std::cout << std::endl << "Tracks used: " << tracks.size() << std::endl;
#endif
  this->routed_tracks[row_num] = tracks;
  return terminals_routed;
}

int channel_router::route_all() {
  const int row_spacing = 2;
  int num_routed = 0;
  // Iterate starting with the second row
  std::map<int,std::vector<node> >::iterator iter = this->rows.begin();
  for (; iter != this->rows.end(); ++iter) {
    int bottom_row = iter->first;
    if ( std::next(iter)->first - row_spacing != iter->first ) {
      // Make a fake row below the first row
      if ( std::next(iter) == this->rows.end() ) {
        num_routed += this->route(0, bottom_row);
      }
      else {
        num_routed += this->route(bottom_row, 0);
      }
      continue;
    }
    if ( ++iter == this->rows.end() ) {
      num_routed += this->route(0, bottom_row);
      break;
    }
    if ( std::next(iter) == this->rows.end() ) {
      num_routed += this->route(0, bottom_row);
      break;
    }
    num_routed += this->route(iter->first, bottom_row);
  }
  return num_routed;
}

int channel_router::get_num_nets() const
{
  // int nets = this->num_nets;
  // for (auto &cell : this->cells) {
  //   if ( cell.getCellWidth() == 3 ) {
  //     nets++;
  //   }
  // }
  // return nets;
  return this->num_nets;
}

std::pair<std::map<int,int>,std::map<int,int> > channel_router::calc_row_offsets() const
{
  // Need to know how far to space each row of cells
  const int cell_offset = 1;
  const int track_offset = 0;
  int offset = 1;
  std::map<int,int> row_offsets;
  for (auto &tracks : this->routed_tracks) {
    offset += tracks.second.size()*2 + cell_offset + track_offset;
    row_offsets[tracks.first] = offset;
  }
  row_offsets[std::prev(row_offsets.end())->first+6] = offset;
  //row_offsets[row_offsets.begin()->first-6] = row_offsets.begin()->second;
  // Translate from terminal location to cell location
  std::map<int,int> ret;
  for (auto &cell : this->cells) {
    for (auto &entry : row_offsets) {
      for (int i = 0; i < TERMINALS_PER_CELL; i++) {
        if ( entry.first == cell.termXY[i][1] ) {
          ret[cell.getLambdaY()] = entry.second;
        }
      }
    }
  }
  ret[ret.begin()->first-6] = 6;
  ret[std::prev(ret.end())->first+6] = offset;
  return make_pair(ret,row_offsets);
}

// Portions of this method are adapted from Josh's code
void channel_router::write_mag_file(std::string magfile)
{
  // Find out how much we need to space out each row
  auto row_offsets = this->calc_row_offsets();

  std::ofstream fp;
  fp.open(magfile);

  fp << "magic\n";
  fp << "tech scmos\n";
  fp << "timestamp " << time(NULL) << "\n";
  fp << "<< m1p >>\n";

  for (auto &cell : this->cells) {
    int shift = 0;
    int extra_offset = 0;
    // if ( !row_offsets.first[cell.getLambdaY()] ) {
    //   shift = -6;
    //   extra_offset = 3;
    // }
    cell.setLambdaCoordinates(cell.getLambdaY() + row_offsets.first[cell.getLambdaY() + shift - 6] + 4 + extra_offset,
                              cell.getLambdaX());
    if ( cell.getCellWidth() == 6 ) {
      fp << "use CELL  " << cell.getCellNum() << std::endl;
    }
    else if ( cell.getCellWidth() == 3 ) {
      fp << "use FEEDTHRU  F" << -1*cell.getCellNum() << std::endl;
    }
    else {
      throw "Invalid cell type";
    }
    switch ( cell.getCellOrientation() ) {
    case NORM:
      fp << "transform 1 0 " << cell.getLambdaX() << " 0 1 " << cell.getLambdaY() << std::endl;
      break;
    case ROTATED:
      fp << "transform -1 0 " << cell.getLambdaX() + 6 << " 0 -1 " << cell.getLambdaY() + 6 << std::endl;
      break;
    case FLIPHORZ:
      fp << "transform 1 0 " << cell.getLambdaX() << " 0 -1 " << cell.getLambdaY() + 6 << std::endl;
      break;
    case FLIPVERT:
      fp << "transform -1 0 " << cell.getLambdaX() + 6 << " 0 1 " << cell.getLambdaY() << std::endl;
      break;
    default:
      throw "Invalid cell orientation";
      break;
    }
    fp << "box 0 0 " << cell.getCellWidth() << " 6" << std::endl;
  }
  // Write wires
  std::stringstream metal1;
  std::stringstream metal2;
  std::stringstream via;
  metal1 << "<< metal1 >>" << std::endl;
  metal2 << "<< metal2 >>" << std::endl;
  via << "<< via >>" << std::endl;
  for (auto channel = routed_tracks.begin(); channel != routed_tracks.end(); ++channel) {
    int tracknum = 0;
    int row_y = channel->first + row_offsets.second[channel->first-6] + 1 + 4;
    for (auto &track : channel->second) {
      tracknum += 2;
      for (auto &net : track) {
        //format is rect xbot ybot xtop ytop
        // Skip metal1 for pure vertical net or net with no vias
        if ( abs(net.horizontal.second - net.horizontal.first) > 1 ) {
          metal1 << "rect " << net.horizontal.first << ' ' << row_y + tracknum << ' ' << net.horizontal.second + 1
                 << ' ' << row_y + tracknum + 1 << std::endl;
          via << "rect " << net.horizontal.first << ' ' << row_y + tracknum << ' '
              << net.horizontal.first + 1 << ' ' << row_y + tracknum + 1 << std::endl;
          via << "rect " << net.horizontal.second << ' ' << row_y + tracknum << ' '
              << net.horizontal.second + 1 << ' ' << row_y + tracknum + 1 << std::endl;
        }
        if ( net.left_up ) {
          int ytop;
          if ( std::next(channel) == routed_tracks.end() ) {
            ytop = channel->first + row_offsets.second[channel->first] + 6;
          }
          else {
            ytop = std::next(channel)->first + row_offsets.second[std::next(channel)->first-6];
          }
          metal2 << "rect " << net.horizontal.first << ' ' << row_y + tracknum << ' ' << net.horizontal.first + 1
                 << ' ' << ytop + 1 << std::endl;
        }
        else {
          metal2 << "rect " << net.horizontal.first << ' ' << row_y << ' ' << net.horizontal.first + 1
                 << ' ' << row_y + tracknum + 1 << std::endl;
        }
        if ( net.right_up ) {
          int ytop;
          if ( std::next(channel) == routed_tracks.end() ) {
            ytop = channel->first + row_offsets.second[channel->first] + 6;
          }
          else {
            ytop = std::next(channel)->first + row_offsets.second[std::next(channel)->first-6];
          }
          metal2 << "rect " << net.horizontal.second << ' ' << row_y + tracknum << ' ' << net.horizontal.second + 1
                 << ' ' << ytop + 1 << std::endl;
        }
        else {
          metal2 << "rect " << net.horizontal.second << ' ' << row_y << ' ' << net.horizontal.second + 1
                 << ' ' << row_y + tracknum + 1 << std::endl;
        }
      }
    }
  }
  fp << metal1.str() << metal2.str() << via.str();
  fp << "<< end >>" << std::endl;

  fp.close();
}

void channel_router::print_net_stats() const
{
  std::cout << "Stranded terminals:   " << stranded_nets << std::endl;
  std::cout << "Unroutable terminals: " << unroutable_nets << std::endl;
  std::cout << "Bumps:                " << bumps << std::endl;
}
