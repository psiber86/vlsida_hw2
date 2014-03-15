#ifndef _HEADER_CELL
#define _HEADER_CELL

#define CELL_HEIGHT     6
#define T1T3_OFFSET     1
#define T2T4_OFFSET     4

class Cell {
private:
    int cellId;         //number of current cell
    
//    int terms[2][4];    //ind=terminal, val[ind0]=cell, val[ind1]=cellterminal, 0 otherwise 
    //key = terminal, pair0 = remote cell, pair1 = remote cell terminal
    std::map<int, std::pair<int, int> > termNets;
    
    int numNets;        //num of nets cell is involved in

    int xcell, ycell;   //coordinates of cell in cellgrid

    int xLbot, yLbot;   //lower left corner of cell in lambda grid
    int xRtop, yRtop;   //upper right corner of cell

    int cellWidth;      //different for feedthrough cells 

    int zeroForce;      //cost of cell in current location

    int orientation;

    bool debug;

    int nets[4];

public:
    Cell(int, int, bool);
    Cell(const Cell& other) : numNets(other.numNets), xcell(other.xcell), ycell(other.ycell), xLbot(other.xLbot),
      yLbot(other.yLbot), xRtop(other.xRtop), yRtop(other.yRtop), cellWidth(other.cellWidth),
      zeroForce(other.zeroForce), orientation(other.orientation), debug(other.debug)
    {
      for (int i = 0; i < 4; i++) {
        this->termXY[i][0] = other.termXY[i][0];
        this->termXY[i][1] = other.termXY[i][1];
        this->nets[i] = other.nets[i];
      }
    }

    int getCellNum();

    void connectTerminals(int, int, int, int);
    void connectTerminals(int, int, int, int, int);

    int getNetCount();

    std::map<int, std::pair<int, int> >  getTermNets();

    void setCellCoordinates(int, int);

    int getCellX();

    int getCellY();

    int getLambdaX();

    int getLambdaY();

    std::pair<int, int> getTerminalCoordinates(int);

    void setForce(int);

    int getForce();

    void setLambdaCoordinates(int, int );

    void resetTermCoords();

    void rotateCell();

    void flipHorzCell();

    void flipVertCell();

    int getCellOrientation();

    int termXY[4][2];   //coordinates of terminals 

    const int* getNets() const { return nets; };

    int getCellWidth() const { return cellWidth; };
};

#endif
