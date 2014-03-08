#ifndef _HEADER_PLACER
#define _HEADER_PLACER

#include "Cell.h"

class Placer {
private:
    signed short **cellGrid;  //layout matrix with granularity of cells
    unsigned char **lambdaGrid; //layout matric with granularity of lambdas

    int cellGridRows;
    int cellGridCols;
    int lambdaGridRows;
    int lambdaGridCols;

    //area of current placement
    int topRowBounding;
    int botRowBounding;
    int leftColBounding;
    int rightColBounding;

    int cellCount;

    Cell **cells;

    bool debug;

public:
    Placer(int, Cell **, bool);

    void placeCellsInitial();

    void printCellGrid();

    void calculateConnectivity();
    
    void placeByForceDirected();

    void mapCellsToLambda();
};

#endif
