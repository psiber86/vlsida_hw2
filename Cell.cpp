#include "PR.h"
#include "Cell.h"

Cell::Cell(int cellNum, int type, bool glob_debug) 
{
    debug = glob_debug; 

    cellId = cellNum;

    if (type == FEEDTHRU) {
        cellWidth = 3; 
    } else {
        cellWidth = 6;
    }

    numNets = 0;

    memset(termXY, 0, sizeof(int)*2*4);

    zeroForce = -1;
}

int Cell::getCellNum() 
{
    return cellId;
}

void Cell::connectTerminals(int locCell, int locTerm, int remCell, int remTerm)
{
//    assert((terms[0][locTerm-1] == 0) && (terms[1][locTerm-1] == 0));
//    terms[0][locTerm-1] = remCell;
//    terms[1][locTerm-1] = remTerm;
    std::pair<int, int> rcellTerm (remCell, remTerm);
    termNets[locTerm] = rcellTerm;

    numNets++;
    if(debug) printf("connecting %i-%i to %i-%i\n", locCell, locTerm, remCell, remTerm);
    if(debug) printf("cell %i has %i links\n", locCell, numNets);
    if(debug) {
        printf("Cell %i:\n", locCell);
        printf("\tTerminal: ");
        for (int i = 0; i < 4; i++) {
            printf("\t%i", i+1);
        }
        printf("\n\tCell:     ");
        for (int i = 1; i <= 4; i++) {
            printf("\t%i", termNets[i].first);
        }
        printf("\n");
    }
}

int Cell::getLinkCount()
{
    return numNets;
}

std::map<int, std::pair<int, int> > Cell::getTermNets()
{
    return termNets; 
}

void Cell::setCellCoordinates(int y, int x)
{
    xcell = x;
    ycell = y;

    setLambdaCoordinates(x*6, y*6);
}

//x and y are the bottom left corner coordinates of the cell
void Cell::setLambdaCoordinates(int x, int y)
{
    //set coordinates of lower left corner
    xLbot = x;
    yLbot = y;

    //set coordinates of upper right cor
    xRtop = x + cellWidth;
    yRtop = y + CELL_HEIGHT; 

    //TODO: optimize??
    //terminal 1
    termXY[0][0] = xLbot + T1T3_OFFSET;
    termXY[0][1] = yRtop;
    //terminal 2
    termXY[1][0] = xLbot + T2T4_OFFSET;
    termXY[1][1] = yRtop - 2;
    //terminal 3
    termXY[2][0] = xLbot + T1T3_OFFSET;
    termXY[2][1] = yLbot;
    //terminal 4
    termXY[3][0] = xLbot + T2T4_OFFSET;
    termXY[3][1] = yLbot;
}

int Cell::getCellX() 
{
    return xcell;
}

int Cell::getCellY() 
{
    return ycell;
}

std::pair<int, int> Cell::getTerminalCoordinates(int term) 
{
    std::pair<int, int> loc (termXY[term-1][0], termXY[term-1][1]);
    return loc; 
}

void Cell::setForce(int force)
{
    zeroForce = force;
}

int Cell::getForce()
{
    return zeroForce;
}


