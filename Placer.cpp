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
bool CmpCellNum(Cell *a, Cell *b)
{
    return a->getCellNum() < b->getCellNum();
}

Placer::Placer(const char *filename, int cellCountIn, Cell **cellList, bool debugIn)
{
    debug = debugIn;

    magfile = filename;

    cells = cellList;
    cellCount = cellCountIn;
    feedCellCount = 0;

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
    const int midRow = 2*round(int(ceil(cellGridRows/2.0)-1)/2.0);
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

    if(debug) printf("wire length estimate: %i\n", wireLengthEst/2);
}

void Placer::computeTargetLoc(int curCell, int *tarRow, int *tarCol)
{
    int finalOrien = 0;
    int weightXSum[ORIEN_COUNT] = {0, 0, 0, 0}, weightYSum[ORIEN_COUNT] = {0, 0, 0, 0};
    int weightSum[ORIEN_COUNT] = {0, 0, 0, 0};

    std::map<int, std::pair<int, int> > remCells = cells[forceOrderMap[curCell]]->getTermNets();

    assert(curCell == cells[forceOrderMap[curCell]]->getCellNum());
    
    for (int iOrien = 0; iOrien < ORIEN_COUNT; iOrien++) {
        switch (iOrien) {
        case NORM:
            break;
        case ROTATED:
            cells[forceOrderMap[curCell]]->rotateCell();
            break;
        case FLIPHORZ:
            cells[forceOrderMap[curCell]]->flipHorzCell();
            break;
        case FLIPVERT:
            cells[forceOrderMap[curCell]]->flipVertCell();
            break;
        default:
            printf("INVALID ORIENTATION\n");
            exit(1);
        }

        for (int iterm = 1; iterm <= 4; iterm++) {
            int weight = 0;
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
                printf("[ORIEN-%i] xdelta between cell %i-%i and cell %i-%i is %i\n", 
                       iOrien, curCell, iterm, remCell, remTerm, abs(locTermXY.first - remTermXY.first));
                printf("[ORIEN-%i] ydelta between cell %i-%i and cell %i-%i is %i\n", 
                       iOrien, curCell, iterm, remCell, remTerm, abs(locTermXY.second - remTermXY.second));
                printf("[ORIEN-%i] total weight between cell %i-%i and cell %i-%i is %i\n", 
                       iOrien, curCell, iterm, remCell, remTerm, weight);
            }

            weightSum[iOrien] += weight;
            weightXSum[iOrien] += weight * remTermXY.first;
            weightYSum[iOrien] += weight * remTermXY.second;

            if (weightSum[iOrien] < weightSum[finalOrien]) {
                finalOrien = iOrien;
            }
        }
    }

    //switch back to preferred orientation
    switch (finalOrien) {
    case NORM:
        if(debug) printf("cell %i has minimal force in standard orientation\n", curCell);
        cells[forceOrderMap[curCell]]->resetTermCoords();
        break;
    case ROTATED:
        if(debug) printf("cell %i has minimal force in rotated orientation\n", curCell);
        cells[forceOrderMap[curCell]]->rotateCell();
        break;
    case FLIPHORZ:
        if(debug) printf("cell %i has minimal force in horizontal flipped orientation\n", curCell);
        cells[forceOrderMap[curCell]]->flipHorzCell();
        break;
    case FLIPVERT:
        if(debug) printf("cell %i has minimal force in vertical flipped orientation\n", curCell);
        cells[forceOrderMap[curCell]]->flipVertCell();
        break;
    }

    if (weightSum[finalOrien] == 0) {
        *tarCol = int(ceil(cellGridCols/2.0));
        *tarRow = int(ceil(cellGridRows/2.0));
        findNearestVacantCell(curCell, tarRow, tarCol); 
        bypassLock = true;
    } else {
        *tarCol = round( (weightXSum[finalOrien]/weightSum[finalOrien]) * 1/6);
        *tarRow = round( (weightYSum[finalOrien]/weightSum[finalOrien]) * 1/6); 
        cells[forceOrderMap[curCell]]-> setForce(weightSum[finalOrien]);
    }
    if(debug) printf("[TARLOC-2] cell %i has total weight=%i, Xweight=%i, Yweight=%i\n", 
                     curCell, weightSum[finalOrien], weightXSum[finalOrien], weightYSum[finalOrien]);
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
        for (int offsetRow = *tarRow - 2*offset; offsetRow <= *tarRow + 2*offset; offsetRow += 2) {
            if (cellGrid[offsetRow][*tarCol] == 0) {
                *tarRow = offsetRow;
                return;
            }
        }

        //search for vacant cells diagonal to target cell
        for (int offsetRow = *tarRow - 2*offset; offsetRow <= *tarRow + 2*offset; offsetRow += 2) {
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
    while (iterCount < 15) {
        if(debug) printf("***** ITER %i *****\n", iterCount);

        //reset grid boundings
        topRowBounding = 0;
        botRowBounding = cellGridRows;
        leftColBounding = cellGridCols;
        rightColBounding = 0;

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
                //round tarRow to nearest multiple of 2
                tarRow = 2 * int(round(tarRow/2.0));

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

                //update grid boundings
                if (tarRow > topRowBounding) { 
                    topRowBounding = tarRow;
                } else if (tarRow < botRowBounding) {
                    botRowBounding = tarRow;
                }
                if (tarCol > rightColBounding) {
                    rightColBounding = tarCol;
                } else if (tarCol < leftColBounding) {
                    leftColBounding = tarCol;
                }

            }
        }
        verifyPlacement();
        if(debug) printCellGrid();
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

void Placer::placeFeedThruCells()
{
    //track what feed thru cells have already been placed to avoid duplicates
    std::vector<std::pair<float, float> > placedFeedCells;
    bool placed = false;
    
    for (int icell = 1; icell <= cellCount; icell++) {
        if (cells[forceOrderMap[icell]]->getNetCount() == 0) { continue; }

        int curCell = cells[forceOrderMap[icell]]->getCellNum();
        
        std::map<int, std::pair<int, int> > remCells = cells[forceOrderMap[icell]]->getTermNets();
        for (int iterm = 1; iterm <= 4; iterm++) {
            int remCell = remCells[iterm].first;
            int remTerm = remCells[iterm].second;
            int cellRow;
            int delta;
            int tarRow, tarCol;
            int leftOf, rightOf;

            if (remCell == 0 || remCell < 0)
            { 
                continue; 
            }

            //check if net feed thru cells have already been placed
            for (int i1 = 0; i1 < int(placedFeedCells.size()); i1++) {
                float cur = float(curCell) + iterm/10.0;
                float rem = float(remCell) + remTerm/10.0;
                if (placedFeedCells[i1].first == rem && placedFeedCells[i1].second == cur) {
                    placed = true;
                }
            }

            if (placed) {
                placed = false;
                continue;
            }

            cellRow = cells[forceOrderMap[icell]]->getCellY();
            tarCol = cells[forceOrderMap[icell]]->getCellX();
            delta =  cellRow - cells[forceOrderMap[remCell]]->getCellY();
            if (abs(delta) > 2 || delta == 0) {
                int iFeed = 1;
                int numFeedThrus = abs(delta/2)-1;

                //place feed thru cell in each row until remote cell is reached
                tarRow = cellRow;
                //check terminal location with respect to cell
                //if one terminal is on the outside of pair, add 1 feed thru cell
                //if both terminals are on outside of pair, add 2 feed thru cells
                //determine which way to iterate vertically
                if (delta > 0) { //move down rows
                    //if locTerm is at top and remTerm is bottom of cell
                    if (!cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                        cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm))
                    {
                        numFeedThrus += 2;
                    }
                    //if locTerm is at top and remTerm is at top or
                    //if locTerm is on top and remTerm is on top
                    else if (  (!cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                                !cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm))
                            || (cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                                cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm)) )
                    {
                        numFeedThrus += 1;
                    }
                    else {
                        tarRow -= 2;
                    }
                } else if (delta < 0) {         //move up rows
                    //if locTerm is at bottom of cell and remTerm is top of cell
                    if (cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                        !cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm))
                    {
                        numFeedThrus += 2;
                    }
                    //if locTerm is at bottom and remTerm is at bottom or
                    //if locTerm is on top and remTerm is on top
                    else if (  (cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                                cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm))
                            || (!cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                                !cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm)) )
                    {
                        numFeedThrus += 1;
                    }
                    else {
                        tarRow += 2;
                    }
                } else {
                    if ( (cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                          !cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm)) ||
                         (!cells[forceOrderMap[icell]]->getTermLocInCell(iterm) &&
                          cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm)) )
                    {
                        numFeedThrus = 1;
                    } else {
                        continue;
                    }
                }

                printf("cell %i (bottom=%i) is separated from cell %i (bottom=%i) by %i rows\n",
                        curCell, cells[forceOrderMap[icell]]->getTermLocInCell(iterm),
                        remCell, cells[forceOrderMap[remCell]]->getTermLocInCell(remTerm), numFeedThrus);

                do {
                    //determine which way to shift currently placed cells
                    for (int icol = leftColBounding; icol <= rightColBounding; icol++) {
                        if (cellGrid[tarRow][icol] > 0 && icol < tarCol) {
                            leftOf++;
                        } else if (cellGrid[tarRow][icol] > 0 && icol > tarCol) {
                            rightOf++;
                        }
                    }
                    if (leftOf <= rightOf) {
                        shiftCellsLeft(tarRow, tarCol);
                    } else {
                        shiftCellsRight(tarRow, tarCol);
                    }

                    //create place feed thru cell
                    Cell *feedThru = new Cell(++feedCellCount, FEEDTHRU, debug);
                    if(debug) printf("creating new feed thru cell %i\n", feedCellCount);
                
                    //CONNECT TERMINALS
                    //connect to current cell
                    if (iFeed == 1) {
                        if (debug) printf("connecting feed thru to local cell\n");
                        if (delta > 0) { 
                            cells[forceOrderMap[icell]]->connectTerminals(curCell, iterm, (-1)*feedCellCount, 1); 
                            feedThru->connectTerminals((-1)*feedCellCount, 1, curCell, iterm,
                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        } else {
                            cells[forceOrderMap[icell]]->connectTerminals(curCell, iterm, (-1)*feedCellCount, 2); 
                            feedThru->connectTerminals((-1)*feedCellCount, 2, curCell, iterm,
                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        }
                    }

                    //connect to intermediate feed thru cells
                    if (numFeedThrus > 1 && iFeed > 1) {
                        if(debug) printf("connecting feed thru to intermediate feed thru\n");
                        int remFeedCell = feedCells[getFeedCellInd((-1)*(feedCellCount-1))]->getCellNum();
                        if (delta > 0) { 
                            feedThru->connectTerminals((-1)*feedCellCount, 2, (-1)*(feedCellCount-1), 1,
                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                            feedCells[getFeedCellInd((-1)*(feedCellCount-1))]->
                              connectTerminals((-1)*remFeedCell,1, (-1)*feedCellCount, 2,
                                               cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        } else {
                            feedThru->connectTerminals((-1)*feedCellCount, 1, (-1)*(feedCellCount-1), 2,
                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                            feedCells[getFeedCellInd((-1)*(feedCellCount-1))]->
                              connectTerminals((-1)*remFeedCell, 2, (-1)*feedCellCount, 1,
                                               cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        }
                    }

                    //connect to remote cell
                    if (iFeed == numFeedThrus) {    
                        if(debug) printf("connecting feed thru to remote cell\n");
                        if (delta > 0) { 
                            feedThru->connectTerminals((-1)*feedCellCount, 2, remCell, remTerm);
                            cells[forceOrderMap[remCell]]->connectTerminals(remCell, remTerm, (-1)*feedCellCount, 2,
                                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        } else {
                            feedThru->connectTerminals((-1)*feedCellCount, 1, remCell, remTerm);
                            cells[forceOrderMap[remCell]]->connectTerminals(remCell, remTerm, (-1)*feedCellCount, 1,
                                                                       cells[forceOrderMap[icell]]->getNets()[iterm-1]);
                        }
                    } 

                    feedThru->setCellCoordinates(tarRow, tarCol); 
                    feedCells.push_back(feedThru);

                    if(debug) printf("placing feed thru cell -%i in row %i,%i\n", feedCellCount, tarRow, tarCol);

                    cellGrid[tarRow][tarCol] = feedCellCount * -1;
                    if (delta > 0) { //move down rows
                        tarRow -= 2;
                    } else {         //move up rows 
                        tarRow += 2;
                    }

                    iFeed++;
                } while (iFeed <= numFeedThrus);  

                placedFeedCells.push_back( std::make_pair(float(curCell)+iterm/10.0, float(remCell)+remTerm/10.0) );
            }
        }
    }
}

void Placer::shiftCellsLeft(int tarRow, int tarCol)
{
    for (int icol = leftColBounding - 1; icol < tarCol; icol++) {
        //TODO: don't shift feed thru cells
//        if (cellGrid[tarRow][icol+1] < 0) { 
//            cellGrid[tarRow][icol] = cellGrid[tarRow][icol + 2]; 
//            icol++;
//        } else {
//            cellGrid[tarRow][icol] = cellGrid[tarRow][icol + 1]; 
//        }

            //update hidden feed thru cell locations
        int nextCell = cellGrid[tarRow][icol + 1];
        int tmpCell = 0; 
        if (nextCell < 0) {
            tmpCell = findFeedThruAtLoc((-1)*nextCell, tarRow, icol); 
        }
        if (tmpCell != 0) {
            if(debug) printf("found feed thru cell %i at loc %i,%i\n", tmpCell, tarRow, icol);
            feedCells[getFeedCellInd(tmpCell)]->setCellCoordinates(tarRow, icol);
        }
        cellGrid[tarRow][icol] = nextCell; 
    }
    
    if (cellGrid[tarRow][leftColBounding - 1] > 0) {
        leftColBounding -= 1;
    }
}
void Placer::shiftCellsRight(int tarRow, int tarCol)
{
    for (int icol = rightColBounding + 1; icol > tarCol; icol--) {
        //TODO: don't shift feed thru cells if possible
//        if (cellGrid[tarRow][icol-1] < 0) {
//            cellGrid[tarRow][icol] = cellGrid[tarRow][icol - 2]; 
//            icol--;
//        } else {
//            cellGrid[tarRow][icol] = cellGrid[tarRow][icol - 1]; 
//        }
//    }
        int prevCell = cellGrid[tarRow][icol - 1];
        int tmpCell = 0; 
        if (prevCell < 0) {
            tmpCell = findFeedThruAtLoc((-1)*prevCell, tarRow, icol); 
        }
        if (tmpCell != 0) {
            if(debug) printf("found feed thru cell %i at loc %i,%i\n", tmpCell, tarRow, icol);
            feedCells[getFeedCellInd(tmpCell)]->setCellCoordinates(tarRow, icol);
        }
        cellGrid[tarRow][icol] = prevCell; 
    }

    if (cellGrid[tarRow][rightColBounding + 1] > 0) {
        rightColBounding += 1;
    }
}

void Placer::compactAndMapLambda() 
{
    bool firstCell = true;
    int curColPos  = 0;

    for (int iRow = topRowBounding; iRow >= botRowBounding; iRow--) {
        for (int iCol = leftColBounding; iCol <= rightColBounding; iCol++) {
            int curCell = cellGrid[iRow][iCol];
            int tarCol = 0;

            if (firstCell && curCell != 0) {
                firstCell = false;
                curColPos = iCol*6; 
            }

            if (curCell > 0) {
                tarCol = cells[forceOrderMap[curCell]]->getCellX()*6;
                if (tarCol - curColPos >= 6) {
                    curColPos = tarCol; 
                }

                if(debug) printf("placing cell %i at lambda %i\n", curCell, curColPos);
                cells[forceOrderMap[curCell]]->setLambdaCoordinates(iRow*3, curColPos);
                curColPos += 6;
            } else if (curCell < 0) {
                int tmpCell;
                int ind = getFeedCellInd(curCell);
                if(debug) printf("ind of feed cell %i is %i\n", curCell, ind);
                tarCol = feedCells[ind]->getCellX()*6;
                if (tarCol - curColPos >= 6) {
                    curColPos = tarCol; 
                }
                if(debug) printf("placing feed cell %i at lambda %i\n", curCell, curColPos);
                feedCells[getFeedCellInd(curCell)]->setLambdaCoordinates(iRow*3, curColPos);
                curColPos += 3;

                //find and place other feedthru cell that is assigned to this location
                tmpCell = findFeedThruAtLoc((-1)*curCell, iRow, tarCol/6); 
                if (tmpCell != 0) {
                    if(debug) printf("found feed thru cell %i at loc %i,%i\n", tmpCell, iRow, tarCol/6);
                    feedCells[getFeedCellInd(tmpCell)]->setLambdaCoordinates(iRow*3, curColPos);
                    curColPos += 3; 
                }
            }

            if (cellGrid[iRow][iCol+1] > 0 && curCell > 0) {
                //leave a space between cells
                curColPos += 1;
            } 
        }
        firstCell = true;
        curColPos = 0;
    }
}

int Placer::getFeedCellInd(int curCell)
{
    for (int i1 = 0; i1 < feedCellCount; i1++) {
        if (feedCells[i1]->getCellNum() == (curCell*(-1))) {
            return i1;
        }
    }

    return -1;
}

int Placer::findFeedThruAtLoc(int curCell, int row, int col)
{
    for (int i1 = 0; i1 < feedCellCount; i1++) {
        if (feedCells[i1]->getCellNum() == curCell) {
            continue;
        } else if (feedCells[i1]->getCellX() == col &&
            feedCells[i1]->getCellY() == row)
        {
            return (-1)*feedCells[i1]->getCellNum();
        }
    }
    return 0;
}

void Placer::sortCellArrays() 
{
    std::sort(cells + 1, cells + (cellCount+1), CmpCellNum);
    for (int i1 = 1; i1 <= cellCount; i1++) {
        if(debug) printf("%i: CellNum = %i\n", i1, cells[i1]->getCellNum());
    }

    std::sort(feedCells.begin(), feedCells.end(), CmpCellNum);
    for (int i1 = 0; i1 <= feedCellCount; i1++) {
        if(debug) printf("%i: CellNum = %i\n", i1, feedCells[i1]->getCellNum());
    }
} 

void Placer::writeMagFile() const
{
    std::ofstream fp;
    fp.open(magfile);

    fp << "magic\n";
    fp << "tech scmos\n";
    fp << "timestamp " << time(NULL) << "\n";
    fp << "<< m1p >>\n";
    //place std cells
    for (int icell = 1; icell <= cellCount; icell++) {
        fp << "use CELL  " << cells[forceOrderMap[icell]]->getCellNum() << "\n";
        switch (cells[forceOrderMap[icell]]->getCellOrientation()) {
        case NORM:
            fp << "transform 1 0 " << cells[forceOrderMap[icell]]->getLambdaX()
               << " 0 1 " << cells[forceOrderMap[icell]]->getLambdaY() << "\n";
            break;
        case ROTATED:
            fp << "transform -1 0 " << cells[forceOrderMap[icell]]->getLambdaX() + 6
               << " 0 -1 " << cells[forceOrderMap[icell]]->getLambdaY() + 6 << "\n";
            break;
        case FLIPHORZ:
            fp << "transform 1 0 " << cells[forceOrderMap[icell]]->getLambdaX()
               << " 0 -1 " << cells[forceOrderMap[icell]]->getLambdaY() + 6 << "\n";
            break;
        case FLIPVERT:
            fp << "transform -1 0 " << cells[forceOrderMap[icell]]->getLambdaX() + 6
               << " 0 1 " << cells[forceOrderMap[icell]]->getLambdaY() << "\n";
            break;
        default:
            printf("INVALID ORIENTATION\n");
            exit(1);
        }
        fp << "box 0 0 6 6\n";
    }
    //place feed thru cells
    for (int ifeed = 0; ifeed < feedCellCount; ifeed++) {
        fp << "use FEEDTHRU  F" << feedCells[ifeed]->getCellNum()*(-1) << "\n";
        fp << "transform 1 0 " << feedCells[ifeed]->getLambdaX()
           << " 0 1 " << feedCells[ifeed]->getLambdaY() << "\n";
        fp << "box 0 0 3 6\n";
    }
    fp << "<< end >>\n";

    fp.close();
/*
    //write cell template file
    std::ofstream cellfp; 
    cellfp.open("Cell.mag");
    cellfp << "magic\n";
    cellfp << "tech scmos\n";
    cellfp << "timestamp " << time(NULL) << "\n";
    cellfp << "<< metal1 >>\n";
    cellfp << "rect 1 5 2 6\n";
    cellfp << "rect 4 5 5 6\n";
    cellfp << "rect 1 0 2 1\n";
    cellfp << "rect 4 0 5 1\n";
    cellfp << "<< metal2 >>\n";
    cellfp << "rect 1 5 2 6\n";
    cellfp << "rect 4 5 5 6\n";
    cellfp << "rect 1 0 2 1\n";
    cellfp << "rect 4 0 5 1\n";
    cellfp << "<< comment >>\n";
    cellfp << "rect 0 0 6 6\n";
    cellfp << "<< labels >>\n";
    cellfp << "rlabel metal1 1 5 2 6 0 1\n";
    cellfp << "rlabel metal1 4 5 5 6 0 2\n";
    cellfp << "rlabel metal1 1 0 2 1 0 3\n";
    cellfp << "rlabel metal1 4 0 5 1 0 4\n";
    cellfp << "<< end >>\n";
    cellfp.close();
*/
} 

std::vector<Cell> Placer::get_cells() const
{
  std::vector<Cell> all_cells;
  for (int i = 1; i < this->cellCount+1; i++) {
    all_cells.push_back(*cells[i]);
  }
  for (auto &cell : this->feedCells) {
    all_cells.push_back(*cell);
  }

  return all_cells;
}
