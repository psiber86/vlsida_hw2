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
      } else if (grid[j][i].m1 && grid[j][i].m2){
	std::cout << "3";
      } else if (grid[j][i].m1){
	std::cout << "1";
      } else if (grid[j][i].m2){
	std::cout << "2";
      } else if (grid[j][i].term){
	//std::cout << "T";
	std::cout << grid[j][i].val;
      } else if (grid[j][i].buf){
	std::cout << "B";
      } else {
	std::cout << grid[j][i].val;
      }
    }
    std::cout << std::endl;
  }
}

void maze_router::reset_grid(int cols, int rows){
  for (int i = 0; i < cols; i++){
    for (int j = 0; j < rows; j++){
      grid[i][j].val = 0;
      grid[i][j].checked = 0;
    }
  }
}

int maze_router::expand_grid(std::vector<Cell> &cells){
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

  //std::cout << "Grid Expanded." << std::endl;
  
  return rows.size()*3;
  
}

maze_router::maze_router(std::vector<Cell> cells, int crows, int ccols, int num_nets, int** nets, std::string filename) : cells(cells), crows(crows), ccols(ccols), num_nets(num_nets), nets(nets), filename(filename) {
  
  //std::cout << ccols << "x" << crows << std::endl;
  // expand vertical cell spacing
  crows += this->expand_grid(cells);
  // initialize grid
  //std::cout << ccols << "x" << crows << std::endl;
  this->create_grid(ccols, crows);
  // mark cells in grid
  for (int c = 0; c < cells.size(); c++) {
    int x = cells[c].getLambdaX();
    int y = cells[c].getLambdaY();   
    if (cells[c].getCellWidth() == 6){ //get rid of unneeded feedthrough cells
      for (int i = x; i < x+cells[c].getCellWidth(); i++){
	for (int j = y; j < y+6; j++){
	  grid[i][j].cell = 1;
	  if (i+1 < ccols) {grid[i+1][j].buf = 1;}
	  if (i-1 >= 0) {grid[i-1][j].buf = 1;}
	  if (j+1 < crows) {grid[i][j+1].buf = 1;}
	  if (j-1 >= 0) {grid[i][j-1].buf = 1;}
	  if ((i+1 < ccols) && (j+1 < crows)) {grid[i+1][j+1].buf = 1;}
	  if ((i+1 < ccols) && (j-1 >= 0)) {grid[i+1][j-1].buf = 1;}
	  if ((i-1 >= 0) && (j+1 < crows)) {grid[i-1][j+1].buf = 1;}
	  if ((i-1 >= 0) && (j-1 >= 0)) {grid[i-1][j-1].buf = 1;}
	}
      }
      for (int i = 0; i < TERMINALS_PER_CELL; i++){
	grid[cells[c].termXY[i][0]][cells[c].termXY[i][1]].cell = 0;
	grid[cells[c].termXY[i][0]][cells[c].termXY[i][1]].buf = 0;
	grid[cells[c].termXY[i][0]+1][cells[c].termXY[i][1]].buf = 0;
	grid[cells[c].termXY[i][0]-1][cells[c].termXY[i][1]].buf = 0;
	grid[cells[c].termXY[i][0]][cells[c].termXY[i][1]+1].buf = 0;
	grid[cells[c].termXY[i][0]][cells[c].termXY[i][1]-1].buf = 0;
	grid[cells[c].termXY[i][0]][cells[c].termXY[i][1]].term = 1;
      }
    }    
  }

  //std::cout << "Grid Initialized" << std::endl;
  
  int num_routed = 0;
  // try metal 1 for all nets
  for (int i = 0; i < num_nets; i++){
    int x1 = cells[nets[i][0]-1].termXY[nets[i][1]-1][0];
    int y1 = cells[nets[i][0]-1].termXY[nets[i][1]-1][1];
    int x2 = cells[nets[i][2]-1].termXY[nets[i][3]-1][0];
    int y2 = cells[nets[i][2]-1].termXY[nets[i][3]-1][1];
    if(this->lee_algorithm(x1, y1, x2, y2, 1)){
      nets[i][4] = 1;
      num_routed++;
      //std::cout << "ROUTED" << std::endl;
    }
  }

  // try metal 2 for remaining nets
  for (int i = 0; i < num_nets; i++){
    if(nets[i][4] == 0){
      int x1 = cells[nets[i][0]-1].termXY[nets[i][1]-1][0];
      int y1 = cells[nets[i][0]-1].termXY[nets[i][1]-1][1];
      int x2 = cells[nets[i][2]-1].termXY[nets[i][3]-1][0];
      int y2 = cells[nets[i][2]-1].termXY[nets[i][3]-1][1];
      if(this->lee_algorithm(x1, y1, x2, y2, 2)){
	nets[i][4] = 1;
	num_routed++;
	//std::cout << "ROUTED" << std::endl;
      }
    }
  }

  //this->print_grid(ccols, crows);
  std::cout << "Routed " << num_routed << " out of " << num_nets << std::endl;
  //calculate wire length
  int wirelength = 0;
  for (int i = 0; i < ccols; i++){
    for (int j = 0; j < crows; j++){
      if (grid[i][j].m1 || grid[i][j].m2){
	wirelength++;
      }
    }
  }
  std::cout << "Wire Length: " << wirelength << std::endl;
  //std::cout << "Writing Magic File." << std::endl;
  this->write_mag_file(filename, cells);
}

bool maze_router::lee_algorithm(int x1, int y1, int x2, int y2, int m){
  //std::cout << x1 << "," << y1 << " -> " << x2 << "," << y2 << std::endl;
  
  std::vector<coord> cset; //current set
  std::vector<coord> nset; //neighbor set
  int wave = 1;
  bool targetFound = 0;

  //this->print_grid(ccols, crows);

  // init the current set
  cset.push_back(coord(x1, y1));
  grid[x1][y1].val = wave;
  
  
  while(!targetFound){
  //for(int pass = 0; pass < 70; pass++){
    //set wave
    wave++;
    if (wave == 10){wave = 1;}

    // find neighbors
    //std::cout << "Checking Neighbors." << std::endl;
    //std::cout << "Cset = " << cset.size() <<  "; Nset = " << nset.size() << std::endl;
    for ( int i = 0; i < cset.size(); i++){
      if (this->check_neighbor(cset[i].x+1, cset[i].y, m)) {nset.push_back(coord(cset[i].x+1, cset[i].y));}
      if (this->check_neighbor(cset[i].x-1, cset[i].y, m)) {nset.push_back(coord(cset[i].x-1, cset[i].y));}
      if (this->check_neighbor(cset[i].x, cset[i].y+1, m)) {nset.push_back(coord(cset[i].x, cset[i].y+1));}
      if (this->check_neighbor(cset[i].x, cset[i].y-1, m)) {nset.push_back(coord(cset[i].x, cset[i].y-1));}
    }
    if (nset.size() == 0) {
      // no neighbors, no route
      this->reset_grid(ccols, crows);
      return 0;
    }
    
    // wave propagation
    //std::cout << "Propagating Wave." << std::endl;
    //std::cout << "Cset = " << cset.size() <<  "; Nset = " << nset.size() << std::endl;
    for ( int i = 0; i < nset.size(); i++){
      grid[nset[i].x][nset[i].y].val = wave;
      // check if target was found
      if((nset[i].x == x2) && (nset[i].y == y2)) {targetFound = 1;}
    }
    
    
    // reset sets
    //std::cout << "Re-setting." << std::endl;
    //cset.erase(cset.begin(), cset.end());
    cset.clear();
    for( int i = 0; i < nset.size(); i++){
      cset.push_back(nset[i]);
    }
    //nset.erase(nset.begin(), nset.end());
    nset.clear();
  }

  //this->print_grid(ccols, crows);

  // trace route back to source
  bool routeFound = 0;
  coord cur = coord(x2, y2);
  coord wstart = cur;
  bool vert = 1;
  while (!routeFound){
    coord next = this->check_nvals(cur.x, cur.y, grid[cur.x][cur.y].val, vert);
    //check if wire ends (change in direction)
    if (((cur.x == next.x) && (vert == 0)) || ((cur.y == next.y) && (vert == 1))){
      if (m == 1){
	metal1.push_back(wire(wstart.x, wstart.y, cur.x, cur.y));
      } else {
	metal2.push_back(wire(wstart.x, wstart.y, cur.x, cur.y));
      }
      wstart = cur;
    }

    //track direction
    if (cur.x == next.x) {vert = 1;}
    if (cur.y == next.y) {vert = 0;}

    if (m == 1) {
      grid[cur.x][cur.y].m1 = 1;
      if (vert){
	grid[cur.x+1][cur.y].m1buf = 1;
	grid[cur.x-1][cur.y].m1buf = 1;
      } else {
	grid[cur.x][cur.y+1].m1buf = 1;
	grid[cur.x][cur.y-1].m1buf = 1;
      }
    }
    if (m == 2) {
      grid[cur.x][cur.y].m2 = 1;
      if (vert){
	grid[cur.x+1][cur.y].m2buf = 1;
	grid[cur.x-1][cur.y].m2buf = 1;
      } else {
	grid[cur.x][cur.y+1].m2buf = 1;
	grid[cur.x][cur.y-1].m2buf = 1;
      }
    }
    
    cur = next;
    
    if ((cur.x == x1) && (cur.y == y1)) {
      //add final wire
      if (m == 1){
	metal1.push_back(wire(wstart.x, wstart.y, cur.x, cur.y));
      } else {
	metal2.push_back(wire(wstart.x, wstart.y, cur.x, cur.y));
      }
      routeFound = 1;
    }
  }
    
  //this->print_grid(ccols, crows);
  this->reset_grid(ccols, crows);
  //this->print_grid(ccols, crows);
  return 1;
}

bool maze_router::check_neighbor(int x, int y, int m){
  //std::cout << "Checking Neighbor: " << x << "," << y << std::endl;
  if ((x >= ccols) || (y >= crows) || (x < 0) || (y < 0)) {return 0;}
  if (grid[x][y].val > 0) {return 0;}
  if (grid[x][y].cell) {return 0;}
  if ((m == 1) && (grid[x][y].m1)) {return 0;}
  if ((m == 1) && (grid[x][y].m1buf)) {return 0;}
  if ((m == 2) && (grid[x][y].m2)) {return 0;}
  if ((m == 2) && (grid[x][y].m2buf)) {return 0;}
  if (grid[x][y].checked == 1) {return 0;}
  if (grid[x][y].buf == 1) {return 0;}

  grid[x][y].checked = 1;
  return 1;
}

coord maze_router::check_nvals(int x, int y, int val, bool vert){

  int next = val-1;
  if (val == 1) {next = 9;}
  
  if(vert){
    if(grid[x][y+1].val == next){return coord(x, y+1);}
    if(grid[x][y-1].val == next){return coord(x, y-1);}
  }
  if(grid[x+1][y].val == next){return coord(x+1, y);}
  if(grid[x-1][y].val == next){return coord(x-1, y);}
  if(grid[x][y+1].val == next){return coord(x, y+1);}
  if(grid[x][y-1].val == next){return coord(x, y-1);}

}

// Portions of this method are adapted from Josh's and Eric's code
void maze_router::write_mag_file(std::string magfile, std::vector<Cell> &cells)
{
  std::ofstream fp;
  fp.open(magfile);

  fp << "magic\n";
  fp << "tech scmos\n";
  fp << "timestamp " << time(NULL) << "\n";
  fp << "<< m1p >>\n";

  // write all cells (except feedthrough which aren't used
  for (auto &cell : cells) {
    if ( cell.getCellWidth() == 6 ) {
      fp << "use CELL  " << cell.getCellNum() << std::endl;

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
  }
  
  // Write wires
  std::stringstream met1;
  std::stringstream met2;
  met1 << "<< metal1 >>" << std::endl;
  met2 << "<< metal2 >>" << std::endl;
  //format is rect xbot ybot xtop ytop
  for (int i = 0; i < metal1.size(); i++){
    int xbot = metal1[i].xbot;
    int ybot = metal1[i].ybot;
    int xtop = metal1[i].xtop;
    int ytop = metal1[i].ytop;
    if (xbot == xtop) {
      if (ytop < ybot){
	int temp = ybot;
	ybot = ytop;
	ytop = temp+1;
      }
      xtop++;
    }
    if (ybot == ytop) {
      if (xtop < xbot){
	int temp = xbot;
	xbot = xtop;
	xtop = temp+1;
      }
      ytop++;
    }
    met1 << "rect " << xbot << " " << ybot << " " << xtop << " " << ytop << std::endl;
  }
  for (int i = 0; i < metal2.size(); i++){
    int xbot = metal2[i].xbot;
    int ybot = metal2[i].ybot;
    int xtop = metal2[i].xtop;
    int ytop = metal2[i].ytop;
    if (xbot == xtop) {
      if (ytop < ybot){
	int temp = ybot;
	ybot = ytop;
	ytop = temp;
      }
      xtop++;
    }
    if (ybot == ytop) {
      if (xtop < xbot){
	int temp = xbot;
	xbot = xtop;
	xtop = temp;
      }
      ytop++;
    }
    met2 << "rect " << xbot << " " << ybot << " " << xtop << " " << ytop << std::endl;
  }
    
  fp << met1.str() << met2.str();
  fp << "<< end >>" << std::endl;
  
  fp.close();
}
