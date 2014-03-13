#include <time.h>
#include <math.h>
#include "PR.h"
#include "Cell.h"
#include "Placer.h"

int main(int argc, char* argv[])
{
    time_t t0 = time(NULL);
    bool debug = false;
    char *filename = NULL;
    int linenum = 0;
    int netcount, cellcount = 0;
    Cell **cells = NULL;
    Placer *placer = NULL;
    
    if (argc < 2) {
        std::cout << "Usage: placeAndRoute <filename.mag> [-d] < <BM#>" << std::endl;
        exit(0);
    } else if (argc == 3 && std::string(argv[2]) == "-d") {
        debug = true;
    }
    filename = argv[1];

    //parse file and populate cell objects
    int tmp1;
    while (std::cin >> tmp1) {
        int t_cell1, t_term1, t_cell2, t_term2;
        if (linenum == 0) {
            cellcount = tmp1;
            
            cells = new Cell*[cellcount+1];
            for (int i = 1; i < cellcount+1; i++) {
                cells[i] = new Cell(i, STDCELL, debug);
            }
        } else if (linenum == 1) {
            netcount = tmp1;
        } else {
            std::cin >> t_cell1;
            std::cin >> t_term1;
            std::cin >> t_cell2;
            std::cin >> t_term2;
            
            cells[t_cell1]->connectTerminals(t_cell1, t_term1, t_cell2, t_term2);
            cells[t_cell2]->connectTerminals(t_cell2, t_term2, t_cell1, t_term1);
        } 
        linenum++;
    }

    placer = new Placer(filename, cellcount, cells, debug);
    
    placer->placeCellsInitial();
    placer->calculateConnectivity();
    if(debug) placer->printCellGrid();
    placer->placeByForceDirected();
    placer->calculateConnectivity();
    if(debug) placer->printCellGrid();
    placer->placeFeedThruCells();
    placer->compactAndMapLambda();
    if(debug) placer->printCellGrid();
    placer->writeMagFile();
    placer->sortCellArrays();

    //routing stuff

    time_t tf = time(NULL);
    printf("Run time: %i hour(s), %i min(s), %i (secs)\n",
            int((tf - t0)/3600), int(((tf - t0)%3600)/60), int((tf - t0)%60));

    return 0;
}
