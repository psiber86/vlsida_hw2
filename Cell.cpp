#include <cstdlib>

#include "PR.h"
#include "Cell.h"

Cell::Cell(int cellNum, int type, bool glob_debug) : orientation(NORM)
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

    for (int i=0; i < 4; i++) {
      nets[i] = 0;
    }

    zeroForce = -1;
}

int Cell::getCellNum() const 
{
    return cellId;
}

void Cell::connectTerminals(int locCell, int locTerm, int remCell, int remTerm)
{
    int numTerms = 4;
    std::pair<int, int> rcellTerm (remCell, remTerm);
    termNets[locTerm] = rcellTerm;

    if (cellWidth == 3) {
        numTerms = 2;
    }

    numNets++;
    if(debug) printf("connecting %i-%i to %i-%i\n", locCell, locTerm, remCell, remTerm);
    if(debug) printf("cell %i has %i links\n", locCell, numNets);
    if(debug) {
        printf("Cell %i:\n", locCell);
        printf("\tTerminal: ");
        for (int i = 0; i < numTerms; i++) {
            printf("\t%i", i+1);
        }
        printf("\n\tCell:     ");
        for (int i = 1; i <= numTerms; i++) {
            printf("\t%i", termNets[i].first);
        }
        printf("\n");
    }
}

void Cell::connectTerminals(int locCell, int locTerm, int remCell, int remTerm, int netNum)
{
  this->connectTerminals(locCell, locTerm, remCell, remTerm);
  if ( this->cellWidth == 3 ) {
    this->nets[0] = netNum;
    this->nets[1] = netNum;
    this->nets[2] = 0;
    this->nets[3] = 0;
  }
  else {
    this->nets[locTerm-1] = netNum;
  }
}

int Cell::getNetCount() const
{
    return numNets;
}

std::map<int, std::pair<int, int> > Cell::getTermNets() const
{
    return termNets; 
}

void Cell::setCellCoordinates(int y, int x)
{
    xcell = x;
    ycell = y;

    setLambdaCoordinates(y*6, x*6);
}

//x and y are the bottom left corner coordinates of the cell
void Cell::setLambdaCoordinates(int y, int x)
{
    //set coordinates of lower left corner
    xLbot = x;
    yLbot = y;

    //set coordinates of upper right cor
    xRtop = x + cellWidth + 1;
    yRtop = y + CELL_HEIGHT + 1; 

    if (cellWidth == 3) {
        //terminal 1
        termXY[0][0] = xLbot + T1T3_OFFSET;
        termXY[0][1] = yLbot;
        //terminal 2
        termXY[1][0] = xLbot + T1T3_OFFSET;
        termXY[1][1] = yLbot + 4;
    } else {
        switch (orientation) {
        case ROTATED:
            this->rotateCell();
            break;
        case FLIPHORZ:
            this->flipHorzCell();
            break;
        case FLIPVERT:
            this->flipVertCell();
            break;
        case NORM:
        default:
            //terminal 1
            termXY[0][0] = xLbot + T1T3_OFFSET;
            termXY[0][1] = yLbot + 4;
            //terminal 2
            termXY[1][0] = xLbot + T2T4_OFFSET;
            termXY[1][1] = yLbot + 4;
            //terminal 3
            termXY[2][0] = xLbot + T1T3_OFFSET;
            termXY[2][1] = yLbot;
            //terminal 4
            termXY[3][0] = xLbot + T2T4_OFFSET;
            termXY[3][1] = yLbot;
            break;
        }
    } 
}

void Cell::resetTermCoords()
{
    orientation = NORM;

    //terminal 1
    termXY[0][0] = xLbot + T1T3_OFFSET;
    termXY[0][1] = yLbot + 4;
    //terminal 2
    termXY[1][0] = xLbot + T2T4_OFFSET;
    termXY[1][1] = yLbot + 4;
    //terminal 3
    termXY[2][0] = xLbot + T1T3_OFFSET;
    termXY[2][1] = yLbot;
    //terminal 4
    termXY[3][0] = xLbot + T2T4_OFFSET;
    termXY[3][1] = yLbot;
}

void Cell::rotateCell() 
{
    orientation = ROTATED;

    //terminal 4
    termXY[3][0] = xLbot + T1T3_OFFSET;
    termXY[3][1] = yLbot + 4;
    //terminal 3
    termXY[2][0] = xLbot + T2T4_OFFSET;
    termXY[2][1] = yLbot + 4;
    //terminal 2
    termXY[1][0] = xLbot + T1T3_OFFSET;
    termXY[1][1] = yLbot;
    //terminal 1
    termXY[0][0] = xLbot + T2T4_OFFSET;
    termXY[0][1] = yLbot;
}

void Cell::flipHorzCell()
{
    orientation = FLIPHORZ;

    //terminal 3
    termXY[2][0] = xLbot + T1T3_OFFSET;
    termXY[2][1] = yLbot + 4;
    //terminal 4
    termXY[3][0] = xLbot + T2T4_OFFSET;
    termXY[3][1] = yLbot + 4;
    //terminal 1
    termXY[0][0] = xLbot + T1T3_OFFSET;
    termXY[0][1] = yLbot;
    //terminal 2
    termXY[1][0] = xLbot + T2T4_OFFSET;
    termXY[1][1] = yLbot;
}

void Cell::flipVertCell()
{
    orientation = FLIPVERT;

    //terminal 2
    termXY[1][0] = xLbot + T1T3_OFFSET;
    termXY[1][1] = yLbot + 4;
    //terminal 1
    termXY[0][0] = xLbot + T2T4_OFFSET;
    termXY[0][1] = yLbot + 4;
    //terminal 4
    termXY[3][0] = xLbot + T1T3_OFFSET;
    termXY[3][1] = yLbot;
    //terminal 3
    termXY[2][0] = xLbot + T2T4_OFFSET;
    termXY[2][1] = yLbot;
}

int Cell::getCellOrientation() const
{
    return orientation;
}

int Cell::getCellX() const
{
    return xcell;
}

int Cell::getCellY() const
{
    return ycell;
}

int Cell::getLambdaX() const
{
    return xLbot;
}

int Cell::getLambdaY() const
{
    return yLbot;
}

std::pair<int, int> Cell::getTerminalCoordinates(int term) const
{
    std::pair<int, int> loc (termXY[term-1][0], termXY[term-1][1]);
    return loc; 
}

int Cell::getTermLocInCell(int term)
{
    int loc = TOP;

    if (termXY[term-1][1] == yLbot) {
        loc = BOTTOM;
    } else {
        loc = TOP;
    }

    return loc;
}

void Cell::setForce(int force)
{
    zeroForce = force;
}

int Cell::getForce() const
{
    return zeroForce;
}

