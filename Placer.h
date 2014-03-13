#ifndef _HEADER_PLACER
#define _HEADER_PLACER

#include "Cell.h"

class Placer {
private:
    char *magfile;

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
    int feedCellCount;

    Cell **cells;

    std::vector<Cell*> feedCells;

    int *forceOrderMap;

    int *lockStatus;

    bool bypassLock;

    bool debug;

public:
    Placer(char*, int, Cell **, bool);

    void placeCellsInitial();

    void printCellGrid();

    void calculateConnectivity();
    
    void computeTargetLoc(int, int*, int*);

    void findNearestVacantCell(int, int *, int *);

    void placeByForceDirected();

    void verifyPlacement();

    void placeFeedThruCells();

    void shiftCellsLeft(int, int);

    void shiftCellsRight(int, int);

    void writeMagFile();

    void compactAndMapLambda();

    int findFeedThruAtLoc(int, int, int);

    int getFeedCellInd(int);

    void sortCellArrays();

    std::vector<Cell> get_cells() const;
};

#endif
