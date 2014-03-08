#include "PR.h"
#include "Placer.h"

bool CmpForce(Cell *a, Cell *b)
{
    return a->getForce() > b->getForce();
}

Placer::Placer(int cellCountIn, Cell **cellList, bool debugIn)
{
    debug = debugIn;

    cells = cellList;
    cellCount = cellCountIn;

    cellGridRows = cellCount < 24 ? 24 : cellCount;
    cellGridCols = cellGridRows;
    cellGrid = new signed short*[cellGridRows];
    for (int i1 = 0; i1 < cellGridRows; i1++) {
        cellGrid[i1] = new signed short[cellGridCols]; 
        for (int i2 = 0; i2 < cellGridCols; i2++) {
            cellGrid[i1][i2] = 0;
        }
    }
    if(debug) printf("allocated cellGrid of size %i X %i\n", cellGridRows, cellGridCols);

    //initialize boundings to opposite sides of grid for comparison update
    topRowBounding = 0;
    botRowBounding = cellGridRows;
    leftColBounding = cellGridCols;
    rightColBounding = 0;
}

//TODO: needs to be heavily optimized, this is a terrible approach
void Placer::placeCellsInitial()
{
    const int midInd = int(ceil(float(cellGridRows)/2.0)-1);
    int numLinks;
    //toggle between top and bottom rows for cells w/ < 4 links
    unsigned flipRows[5];

    //initialize flip* and column shift offsets
    for (int i = 0; i < 5; i++) {
        flipRows[i] = 0;
    }

    for (int i = 1; i <= cellCount; i++) {
        int coli = 0;

        numLinks = cells[i]->getLinkCount();
        if(debug) printf("cell %i has %i links\n", i, numLinks);
        
        //offset from center row by 2 rows for each category
        //row selection is based on how many links cell has, 0 - 4
        int linkRow;
        if (flipRows[numLinks]) { 
            linkRow = midInd + 2*(4-numLinks);
            //update row boundings
            if (linkRow > topRowBounding) {
                topRowBounding = linkRow;
            }
        } else {
            linkRow =  midInd - 2*(4-numLinks);
            //update row boundings
            if (linkRow < botRowBounding) {
                botRowBounding = linkRow;
            }
        }

        for (int colOffset = 0; -midInd <= colOffset && colOffset < midInd; colOffset++) {
            if (colOffset % 2 == 0) {
                coli = midInd - colOffset;
                if (!cellGrid[linkRow][coli]) {
                    cellGrid[linkRow][coli] = i;
                    cells[i]->setCellCoordinates(linkRow, coli);

                    //update column boundings
                    if (midInd - colOffset < leftColBounding) {
                        leftColBounding = midInd - colOffset; 
                    }

                    break;
                }
            } else {
                coli = midInd + colOffset;
                if (!cellGrid[linkRow][coli]) {
                    cellGrid[linkRow][coli] = i;
                    cells[i]->setCellCoordinates(linkRow, coli);

                    //update column boundings
                    if (midInd + colOffset > rightColBounding) {
                        rightColBounding = midInd + colOffset; 
                    }

                    break;
                }
            }
        }

        //toggle switches for next cell placement locations
        flipRows[numLinks] = flipRows[numLinks]^1;

        if(debug) printf("placed cell %i at %i, %i\n", i, linkRow, coli);
    }

    //add buffer space in grid
    topRowBounding += 1;
    botRowBounding -= 1;
}

void Placer::printCellGrid()
{
    printf("cell|       ");
    for (int i = leftColBounding; i <= rightColBounding; i++) {
        printf("%3i ", i);
    }
    printf("\n     lambda|");
    for (int i = leftColBounding; i <= rightColBounding; i++) {
        printf("%3i ", i*6);
    }
    printf("\n");
    
    for (int i1 = topRowBounding; i1 >= botRowBounding; i1--) {
        printf("%4i|%6i|", i1, i1*6);
        
        for (int i2 = leftColBounding; i2 <= rightColBounding; i2++) {
            printf("%3i ", cellGrid[i1][i2]);
        }
        printf("\n");
    }
}

void Placer::calculateConnectivity()
{
    for (int i1 = 1; i1 <= cellCount; i1++) {
        int force = 0;

        if (cells[i1]->getLinkCount() == 0) { continue; }

        std::map<int, std::pair<int, int> > remCells = cells[i1]->getTermNets();
        for (int i2 = 1; i2 <= 4; i2++) {
            int remCell = remCells[i2].first;
            int remTerm = remCells[i2].second;

            if (remCell == 0) { continue; }

            if(debug) printf("calculating connectivity between local cell %i-%i and remote cell %i-%i\n",
                             i1, i2, remCell, remTerm);

            std::pair<int, int> locTermXY = cells[i1]->getTerminalCoordinates(i2);
            std::pair<int, int> remTermXY = cells[remCell]->getTerminalCoordinates(remTerm);

            force += abs(locTermXY.first - remTermXY.first) + abs(locTermXY.second - remTermXY.second);
        }
        cells[i1]->setForce(force);
    }

    std::sort(cells + 1, cells + (cellCount+1), CmpForce);

    if(debug) { 
        for (int i1 = 1; i1 <= cellCount; i1++) {
            printf("cell %i has force %i\n", cells[i1]->getCellNum(), cells[i1]->getForce());
        }
    }
}

void Placer::placeByForceDirected()
{
    int iterCount = 0;
    int abortCount = 0;
    bool endRipple = false;
    while (iterCount < 5) {
        //get next seed cell with next highest force 
        for (int icell = 1; icell <= cellCount; icell++) {
            while (!endRipple) {
                int tarRow = 0, tarCol = 0;
                //TODO: compute target location of cell

                //if target cell is vacant
                if (!cellGrid[tarRow][tarCol]) {
                    //TODO: move seed to target point

                    endRipple = true;
                    abortCount = 0;
                } 
                //if target is same as present location 
                else if (tarRow == cells[icell]->getCellX() &&
                         tarCol == cells[icell]->getCellY())
                {
                    endRipple = true;
                    abortCount = 0;
                }
                //cell is occupied AND LOCKED
                //TODO: get cell at target location
                else if (false) {
                    //TODO: move selected cell to nearest vacant cell

                    endRipple = true;
                    abortCount++;
                    if (abortCount > 5) {
                        //TODO: unlock all cells
                        iterCount++; 
                    }
                }
                //cell is occupied AND UNLOCKED
                else if (false) {
                    //TODO: select cell at target point for next move

                    //TODO: move seed cell to target point and lock target point

                    endRipple = false;
                    abortCount = 0;
                } 
                else {
                    printf("ERROR: INVALID CELL STATE!\n");
                    exit(1);
                }
            }
        }
    }
}

