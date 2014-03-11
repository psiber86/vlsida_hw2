#include "PR.h"
#include "Placer.h"

bool CmpForce(Cell *a, Cell *b)
{
    return a->getForce() > b->getForce();
}
bool CmpNets(Cell *a, Cell *b)
{
    return a->getNetCount() > b->getNetCount();
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

    if ((forceOrderMap = (int*)malloc(sizeof(int)*(cellCount+1))) == NULL) {
        printf("MALLOC ERROR!\n");
        exit(1); 
    }

    if ((lockStatus = (int*)malloc(sizeof(int)*(cellCount+1))) == NULL) {
        printf("MALLOC ERROR!\n");
        exit(1); 
    }
    
    bypassLock = false;
}

void Placer::placeCellsInitial()
{
    const int midRow = int(ceil(cellGridRows/2.0)-1);
    const int midCol = int(ceil(cellGridCols/2.0)-1);
    int icell = 1;
    int offset = 2;

    std::sort(cells + 1, cells + (cellCount+1), CmpNets);

    //place first cell directly in the center of the grid
    cellGrid[midRow][midCol] = cells[icell]->getCellNum();
    cells[icell]->setCellCoordinates(midRow, midCol);
    if(debug) printf("placing cell %i at %i, %i\n", cells[icell]->getCellNum(), midRow, midCol);
    icell++;

    while (icell <= cellCount) {
        //start at bottom left corner
        int rowInd = midRow - offset;
        int colInd = midCol - offset;

        for ( ; rowInd < midRow + offset; rowInd += 2) {
            //adjust grid bounding 
            if (colInd < leftColBounding) {
               leftColBounding = colInd;
            }
            
            //return if all cells have been placed
            if (icell > cellCount) { return; }

            //place the cell
            cellGrid[rowInd][colInd] = cells[icell]->getCellNum();
            cells[icell]->setCellCoordinates(rowInd, colInd);
            if(debug) printf("placing cell %i at %i, %i\n", cells[icell]->getCellNum(), rowInd, colInd);
            icell++;
        }
        for ( ; colInd < midCol + offset; colInd += 2) {
            //adjust grid bounding 
            if (rowInd > topRowBounding) {
                topRowBounding = rowInd;
            }

            //return if all cells have been placed
            if (icell > cellCount) { return; }

            cellGrid[rowInd][colInd] = cells[icell]->getCellNum();
            cells[icell]->setCellCoordinates(rowInd, colInd);
            if(debug) printf("placing cell %i at %i, %i\n", cells[icell]->getCellNum(), rowInd, colInd);
            icell++;
        } 
        for (; rowInd > midRow - offset; rowInd -= 2) {
            //adjust grid bounding
            if (colInd > rightColBounding) {
                rightColBounding = colInd;
            }
            
            //return if all cells have been placed
            if (icell > cellCount) { return; }

            cellGrid[rowInd][colInd] = cells[icell]->getCellNum();
            cells[icell]->setCellCoordinates(rowInd, colInd);
            if(debug) printf("placing cell %i at %i, %i\n", cells[icell]->getCellNum(), rowInd, colInd);
            icell++;
        }
        for (; colInd > midCol - offset; colInd -= 2) { 
            //adjust grid bounding
            if (rowInd < botRowBounding) {
                botRowBounding = rowInd;
            }
            
            //return if all cells have been placed
            if (icell > cellCount) { return; }

            cellGrid[rowInd][colInd] = cells[icell]->getCellNum();
            cells[icell]->setCellCoordinates(rowInd, colInd);
            if(debug) printf("placing cell %i at %i, %i\n", cells[icell]->getCellNum(), rowInd, colInd);
            icell++;
        }
        offset += 2;
    }
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
    printf("\n");
}

void Placer::calculateConnectivity()
{
    int wireLengthEst = 0;

    for (int i1 = 1; i1 <= cellCount; i1++) {
        int curCell = cells[i1]->getCellNum();
        int force = 0;

        if (cells[i1]->getNetCount() == 0) { continue; }

        std::map<int, std::pair<int, int> > remCells = cells[i1]->getTermNets();
        for (int i2 = 1; i2 <= 4; i2++) {
            int remCell = remCells[i2].first;
            int remTerm = remCells[i2].second;

            if (remCell == 0 || remCell == curCell) { continue; }

            if(debug) printf("calculating connectivity between local cell %i-%i and remote cell %i-%i\n",
                             curCell, i2, remCell, remTerm);

            std::pair<int, int> locTermXY = cells[i1]->getTerminalCoordinates(i2);
            std::pair<int, int> remTermXY = cells[remCell]->getTerminalCoordinates(remTerm);

            force += abs(locTermXY.first - remTermXY.first) + abs(locTermXY.second - remTermXY.second);
        }
        wireLengthEst += force;
        cells[i1]->setForce(force);
    }

    std::sort(cells + 1, cells + (cellCount+1), CmpForce);

    for (int i1 = 1; i1 <= cellCount; i1++) {
        forceOrderMap[cells[i1]->getCellNum()] = i1;
    }

    printf("wire length estimate: %i\n", wireLengthEst);
}

void Placer::computeTargetLoc(int curCell, int *tarRow, int *tarCol)
{
    int weightXSum = 0, weightYSum = 0;
    int weightSum = 0;
    int weight = 0;

    std::map<int, std::pair<int, int> > remCells = cells[forceOrderMap[curCell]]->getTermNets();

    assert(curCell == cells[forceOrderMap[curCell]]->getCellNum());
    
    for (int iterm = 1; iterm <= 4; iterm++) {
        int remCell = remCells[iterm].first;
        int remTerm = remCells[iterm].second;

        if (remCell == 0 || (remCell == curCell)) { continue; }

        std::pair<int, int> locTermXY = cells[forceOrderMap[curCell]]->getTerminalCoordinates(iterm);
        std::pair<int, int> remTermXY = cells[forceOrderMap[remCell]]->getTerminalCoordinates(remTerm);

        if(debug) printf("[TARLOC-1] computing force between cell%i-%i @ %i, %i and cell %i-%i @ %i, %i\n",
                         curCell, iterm, locTermXY.first, locTermXY.second,
                         remCell, remTerm, remTermXY.first, remTermXY.second);

        weight = abs(locTermXY.first - remTermXY.first) + abs(locTermXY.second - remTermXY.second);
        if(debug) { 
            printf("xdelta between cell %i-%i and cell %i-%i is %i\n", 
                   curCell, iterm, remCell, remTerm, abs(locTermXY.first - remTermXY.first));
            printf("ydelta between cell %i-%i and cell %i-%i is %i\n", 
                   curCell, iterm, remCell, remTerm, abs(locTermXY.second - remTermXY.second));
            printf("total weight between cell %i-%i and cell %i-%i is %i\n", 
                   curCell, iterm, remCell, remTerm, weight);
        }

        weightSum += weight;
        weightXSum += weight * remTermXY.first;
        weightYSum += weight * remTermXY.second;
    }

    if (weightSum == 0) {
        *tarCol = int(ceil(cellGridCols/2.0));
        *tarRow = int(ceil(cellGridRows/2.0));
        findNearestVacantCell(curCell, tarRow, tarCol); 
        bypassLock = true;
    } else {
        *tarCol = round( (weightXSum/weightSum) * 1/6);
        *tarRow = round( (weightYSum/weightSum) * 1/6); 
        cells[forceOrderMap[curCell]]-> setForce(weightSum);
    }
    if(debug) printf("[TARLOC-2] cell %i has total weight=%i, Xweight=%i, Yweight=%i\n", 
                     curCell, weightSum, weightXSum, weightYSum);
    if(debug) printf("[TARLOC-3] cell %i target cell is %i, %i\n", curCell, *tarRow, *tarCol);
}

void Placer::findNearestVacantCell(int curCell, int *tarRow, int *tarCol)
{
    for (int offset = 1; offset <= cellGridRows; offset++) {
        //search for vacant cells in same row
        for (int offsetCol = *tarCol - offset; offsetCol <= *tarCol + offset; offsetCol++) {
            if (cellGrid[*tarRow][offsetCol] == 0) {
                *tarCol = offsetCol;
                return;
            }
        }

        //search for vacant cells in same col 
        for (int offsetRow = *tarRow - offset; offsetRow <= *tarRow + offset; offsetRow++) {
            if (cellGrid[offsetRow][*tarCol] == 0) {
                *tarRow = offsetRow;
                return;
            }
        }

        //search for vacant cells diagonal to target cell
        for (int offsetRow = *tarRow - offset; offsetRow <= *tarRow + offset; offsetRow++) {
            for (int offsetCol = *tarCol - offset; offsetCol <= *tarCol + offset; offsetCol++) {
                if (cellGrid[offsetRow][offsetCol] == 0) {
                    *tarRow = offsetRow;
                    *tarCol = offsetCol;
                    return;
                }
            }
        }
    }
}

void Placer::placeByForceDirected()
{
    int iterCount = 0;
    int abortCount = 0;
    while (iterCount < 5) {
        printf("***** ITER %i *****\n", iterCount);

        //get next seed cell with next highest force 
        for (int icell = 1; icell <= cellCount; icell++) {
            bool endRipple = false;
            int curCell = cells[icell]->getCellNum();
            int tmpCell = -1;

            if (lockStatus[curCell]) { 
                if(debug) printf("cell %i is locked, moving to next cell in list\n", curCell);    
                continue; 
            }

            //mark current cell as vacant
            if(debug) printf("[WHILE] placing cell%i\n", curCell);
            cellGrid[cells[forceOrderMap[curCell]]->getCellY()][cells[forceOrderMap[curCell]]->getCellX()] = 0;

            while (!endRipple) {
                int tarRow = 0, tarCol = 0;

                computeTargetLoc(curCell, &tarRow, &tarCol);

                //if target cell is vacant
                if (cellGrid[tarRow][tarCol] == 0) {
                    if(debug) printf("[VACANT] moving cell %i to %i, %i\n", curCell, tarRow, tarCol);

                    //move seed to target point
                    cellGrid[tarRow][tarCol] = curCell;
                    cells[forceOrderMap[curCell]]->setCellCoordinates(tarRow, tarCol);

                    //lock cell
                    if (!bypassLock) {
                        if(debug) printf("[VACANT] locking cell%i\n", curCell);
                        lockStatus[curCell] = 1;
                    }
                    bypassLock = false;

                    endRipple = true;
                    abortCount = 0;
                } 
                //if target is same as present location 
                else if ((tarRow == cells[forceOrderMap[curCell]]->getCellY() &&
                          tarCol == cells[forceOrderMap[curCell]]->getCellX()) &&
                         cellGrid[tarRow][tarCol] == 0)
                {
                    if(debug) printf("[SAME] keeping cell %i at %i, %i\n", curCell, tarRow, tarCol);

                    endRipple = true;
                    abortCount = 0;
                }
                //cell is occupied 
                else if (cellGrid[tarRow][tarCol]) {
                    tmpCell = cellGrid[tarRow][tarCol]; 
                   
                    if (lockStatus[tmpCell]) {      //cell is locked
                        //move selected cell to nearest vacant cell
                        findNearestVacantCell(curCell, &tarRow, &tarCol);
                        cellGrid[tarRow][tarCol] = curCell;
                        cells[forceOrderMap[curCell]]->setCellCoordinates(tarRow, tarCol);
                        if(debug) printf("[OCCUPIED-LOCKED] moving cell %i to nearby %i, %i\n", curCell, tarRow, tarCol);

                        endRipple = true;
                        abortCount++;
                        if (abortCount > 3) {
                            //unlock all cells
                            if(debug) printf("[ABORT] unlocking all cells\n");
                            memset(lockStatus, 0, sizeof(int)*(cellCount+1));
                            iterCount++; 
                        }
                    } else {        //cell is unlocked
                        //move seed cell to target point and lock target point
                        cellGrid[tarRow][tarCol] = curCell;
                        cells[forceOrderMap[curCell]]->setCellCoordinates(tarRow, tarCol);
                        if(debug) printf("[OCCUPIED-UNLOCKED] replacing cell %i with cell %i at %i, %i\n", 
                                         tmpCell, curCell, tarRow, tarCol);

                        //lock cell
                        if(debug) printf("[OCCUPIED-UNLOCKED] locking cell %i\n", curCell);
                        lockStatus[curCell] = 1;

                        curCell = tmpCell;

                        endRipple = false;
                        abortCount = 0;

                        if(debug) printf("[WHILE] placing cell%i\n", curCell);
                    }
                } 
                else {
                    printf("ERROR: INVALID CELL STATE!\n");
                    exit(1);
                }

                if (endRipple) {
                    if(debug) printf("last placed cell was cell %i\n", curCell);
                    verifyPlacement();
                }
            }
        }
//        iterCount++;

        printCellGrid();
        std::sort(cells + 1, cells + (cellCount+1), CmpForce);
        for (int i1 = 1; i1 <= cellCount; i1++) {
            forceOrderMap[cells[i1]->getCellNum()] = i1;
        }
    }
}

void Placer::verifyPlacement()
{
    int sumGrid = 0;
    int sumCells = 0;

    for (int icell = 1; icell <= cellCount; icell++) {
        sumCells += icell;
    }

    for (int irow = 0; irow < cellGridRows; irow++) {
        for (int icol = 0; icol < cellGridCols; icol++) {
            sumGrid += cellGrid[irow][icol];         
        }
    }

    if(debug) printf("sumGrid = %i, sumCell = %i\n", sumGrid, sumCells);
    assert(sumGrid == sumCells);
}

