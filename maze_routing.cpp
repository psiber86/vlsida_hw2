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
      if (grid[j][i].cell){
	std::cout << "C";
      } else if (grid[j][i].m1){
	std::cout << "1";
      } else if (grid[j][i].m2){
	std::cout << "2";
      } else {
	std::cout << grid[j][i].val;
      }
    }
    std::cout << std::endl;
  }
}

void maze_router::expand_grid(std::vector<Cell> &cells){
  // generate rows of cells
  for (int i = 0; i < cells.size(); i++) {
    // only care about real cells
    if (cells[i].getCellWidth() == 6){
      int key = cells[i].getLambdaY();
      rows[key].push_back(cells[i].getCellNum());
    }
  }
  
  // expand rows
  int c = 1;
  std::map<int,std::vector<int> >::iterator iter = this->rows.begin();
  for (; iter != this->rows.end(); iter++){
    //std::cout << "Key: " << iter->first << std::endl;
    for(int i = 0; i < iter->second.size(); i++){
      //std::cout << cells[iter->second[i]-1].getLambdaY() << " " << cells[iter->second[i]-1].getCellNum() << " " << iter->second[i] << std::endl;
      cells[iter->second[i]-1].setLambdaCoordinates(cells[iter->second[i]-1].getLambdaY()+(3*c), cells[iter->second[i]-1].getLambdaX());
      //std::cout << cells[iter->second[i]-1].getLambdaY() << std::endl;
    }
    c++;
  }

  std::cout << "Grid Expanded." << std::endl;

}

maze_router::maze_router(std::vector<Cell> cells, int crows, int ccols, int num_nets, int** nets) : cells(cells), crows(crows), ccols(ccols), num_nets(num_nets), nets(nets) {
  // initialize grid
  this->create_grid(ccols, crows);
  std::cout << ccols << "x" << crows << std::endl;
  // expand vertical cell spacing
  this->expand_grid(cells);
  // mark cells in grid
  for (int c = 0; c < cells.size(); i++) {
    int x = cells[c].getLambdaX();
    int y = cells[c].getLambdaY();   
    std::cout << x << "," << y << std::endl;
    if (cells[c].getCellWidth() == 6){ //get rid of unneeded feedthrough cells
      for (int i = x; i < x+cells[c].getCellWidth(); i++){
	for (int j = y; j < y+6; j++){
	  grid[i][j].cell = 1;
	}
      }
    }    
  }
  //this->print_grid(ccols, crows);

}

int maze_router::route(){
 
  int num_routed = 0;

  int i = 0;
  int x1 = cells[nets[i][0]].termXY[nets[i][1]][0];
  int y1 = cells[nets[i][0]].termXY[nets[i][1]][1];
  int x2 = cells[nets[i][2]].termXY[nets[i][3]][0];
  int y2 = cells[nets[i][2]].termXY[nets[i][3]][1];
  
  bool passed = this->lee_algorithm(x1, y1+1, x2, y2+1, 1);

  if (passed)
    num_routed = 45;

  /*
  // try metal 1 for all nets
  for (int i = 0; i < num_nets; i++){
    int x1 = cells[nets[i][0]].termXY[nets[i][1]][0];
    int y1 = cells[nets[i][0]].termXY[nets[i][1]][1];
    int x2 = cells[nets[i][2]].termXY[nets[i][3]][0];
    int y2 = cells[nets[i][2]].termXY[nets[i][3]][1];
    std::cout << x1 << std::endl;
    if(this->lee_algorithm(x1, y1+1, x2, y2+1, 1)){
      nets[i][4] = 1;
      num_routed++;
    }
  }

  // try metal 2 for remaining nets
  for (int i = 0; i < num_nets; i++){
    if(nets[i][4] == 0){
      int x1 = cells[nets[i][0]].termXY[nets[i][1]][0];
      int y1 = cells[nets[i][0]].termXY[nets[i][1]][1];
      int x2 = cells[nets[i][2]].termXY[nets[i][3]][0];
      int y2 = cells[nets[i][2]].termXY[nets[i][3]][1];
      if(this->lee_algorithm(x1, y1+1, x2, y2+1, 2)){
	nets[i][4] = 1;
	num_routed++;
      }
    }
  }
  */
  return num_routed;
}

bool maze_router::lee_algorithm(int x1, int y1, int x2, int y2, int m){
  std::cout << x1 << "," << y1 << " -> " << x2 << "," << y2 << std::endl;
  
  std::vector<coord> cset; //current set
  std::vector<coord> nset; //neighbor set
  int wave = 1;

  grid[x1][y1].cell = 0;
  grid[x1][y1].val = 5;

  this->print_grid(ccols, crows);

  // init the sets
  cset.push_back(coord(x1, y1));
  if (this->check_neighbor(x1+1, y1, m)) {nset.push_back(coord(x1+1, y1));}
  if (this->check_neighbor(x1-1, y1, m)) {nset.push_back(coord(x1-1, y1));}
  if (this->check_neighbor(x1, y1+1, m)) {nset.push_back(coord(x1, y1+1));}
  if (this->check_neighbor(x1, y1-1, m)) {nset.push_back(coord(x1, y1-1));}

  std::cout << nset.size() << std::endl;
  for ( int i = 0; i < nset.size(); i++){
    std::cout << nset[i].x << "," << nset[i].y << std::endl; 
  }

  return 1;
}

bool maze_router::check_neighbor(int x, int y, int m){
  if (grid[x][y].cell) {return 0;}
  if ((m == 1) && (grid[x][y].m1)) {return 0;}
  if ((m == 2) && (grid[x][y].m2)) {return 0;}

  return 1;
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
