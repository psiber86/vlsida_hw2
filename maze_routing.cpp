// Kyle Craig

#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <iostream>
#include <set>
#include <fstream>
#include <string>
#include <sstream>

#include "Cell.h"
#include "maze_routing.hpp"
#include "PR.h"

#define TERMINALS_PER_CELL 4

void maze_router::create_grid(int cols, int rows){
  grid = new gridcell*[cols];
  for (int i = 0; i < cols; i++){
    grid[i] = new gridcell[rows];
  }
}

void maze_router::print_grid(int cols, int rows){
  for (int i = rows-1; i >= 0; i--){
    for (int j = 0; j < cols; j++){
      //std::cout << j << "," << i << " ";
      std::cout << grid[j][i].cell ;
    }
    std::cout << std::endl;
  }
}

maze_router::maze_router(std::vector<Cell> cells, int crows, int ccols, int num_nets) : cells(cells), crows(crows), ccols(ccols), num_nets(num_nets) {
  this->create_grid(ccols, crows);
  std::cout << ccols << "x" << crows << std::endl;
  for (auto &cell : cells) {
    // color in grid
    int x = cell.getLambdaX();
    int y = cell.getLambdaY();
    
    if (cell.getCellWidth() == 6){ //get rid of unneeded feedthrough cells
      for (int i = x; i < x+cell.getCellWidth(); i++){
	for (int j = y; j < y+6; j++){
	  grid[i][j].cell = 1;
	}
      }
    }
    
    for (int i=0; i < TERMINALS_PER_CELL; i++) {
      const int* nets = cell.getNets();
      if( nets[i]){
	
      }
    }
  }
  this->print_grid(ccols, crows);
}

/*
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
    if ( !row_offsets.first[cell.getLambdaY()] ) {
      shift = -6;
      extra_offset = 3;
    }
    cell.setLambdaCoordinates(cell.getLambdaY() + row_offsets.first[cell.getLambdaY() + shift - 6] + extra_offset,
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
  metal1 << "<< metal1 >>" << std::endl;
  metal2 << "<< metal2 >>" << std::endl;
  for (auto channel = routed_tracks.begin(); channel != routed_tracks.end(); ++channel) {
    int tracknum = 0;
    int row_y = channel->first + row_offsets.second[channel->first-6] + 1;
    for (auto &track : channel->second) {
      tracknum += 2;
      for (auto &net : track) {
        //format is rect xbot ybot xtop ytop
        metal1 << "rect " << net.horizontal.first << ' ' << row_y + tracknum << ' ' << net.horizontal.second
               << ' ' << row_y + tracknum + 1 << std::endl;
        if ( net.left_up ) {
          metal2 << "rect " << net.horizontal.first << ' ' << row_y + tracknum << ' ' << net.horizontal.first + 1
                 << ' ' << std::next(channel)->first + row_offsets.second[std::next(channel)->first-6]-3 << std::endl;
        }
        else {
          metal2 << "rect " << net.horizontal.first << ' ' << row_y << ' ' << net.horizontal.first + 1
                 << ' ' << row_y + tracknum + 1 << std::endl;
        }
        if ( net.right_up ) {
          metal2 << "rect " << net.horizontal.second << ' ' << row_y + tracknum << ' ' << net.horizontal.second + 1
                 << ' ' << std::next(channel)->first + row_offsets.second[std::next(channel)->first-6]-3 << std::endl;
        }
        else {
          metal2 << "rect " << net.horizontal.second << ' ' << row_y << ' ' << net.horizontal.second + 1
                 << ' ' << row_y + tracknum + 1 << std::endl;
        }
      }
    }
  }
  fp << metal1.str() << metal2.str();
  fp << "<< end >>" << std::endl;

  fp.close();
}
*/
