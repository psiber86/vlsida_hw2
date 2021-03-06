

#include <ctime>
#include <cmath>
#include <string>
#include <cstddef>
#include <cstdlib>

#include "PR.h"
#include "Cell.h"
#include "Placer.h"
#ifdef CHANNEL_ROUTING
#include "channel_routing.hpp"
#else
#include "maze_routing.hpp"
#endif

int main(int argc, char* argv[])
{
    time_t t0 = time(NULL);
    bool debug = false;
    std::string filename;
    int linenum = 0;
    int netcount = 0, cellcount = 0;
    Cell **cells = NULL;
    Placer *placer = NULL;
    int **nets = NULL;
    
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename.mag> [-d] < <BM#>" << std::endl;
        exit(1);
    } else if (argc == 3 && std::string(argv[2]) == "-d") {
        debug = true;
    }
    filename = argv[1];

    //parse file and populate cell objects
    int net_number;
    while (std::cin >> net_number) {
        int t_cell1, t_term1, t_cell2, t_term2;
        if (linenum == 0) {
            cellcount = net_number;

            cells = new Cell*[cellcount+1];
            for (int i = 1; i < cellcount+1; i++) {
                cells[i] = new Cell(i, STDCELL, debug);
            }
        } else if (linenum == 1) {
            netcount = net_number;
	    nets = new int*[netcount];
	    for(int i = 0; i < netcount; i++){
	      nets[i] = new int[5];
	    }
        } else {
            std::cin >> t_cell1;
            std::cin >> t_term1;
            std::cin >> t_cell2;
            std::cin >> t_term2;
	    
	    nets[net_number-1][0] = t_cell1;
	    nets[net_number-1][1] = t_term1;
	    nets[net_number-1][2] = t_cell2;
	    nets[net_number-1][3] = t_term2;
	    nets[net_number-1][4] = 0;
	    
            cells[t_cell1]->connectTerminals(t_cell1, t_term1, t_cell2, t_term2, net_number);
            cells[t_cell2]->connectTerminals(t_cell2, t_term2, t_cell1, t_term1, net_number);
        } 
        linenum++;
    }

    std::cout << "Placing..." << std::endl;

    placer = new Placer("placer.mag", cellcount, cells, debug);

    placer->placeCellsInitial();
    placer->calculateConnectivity();
    if(debug) placer->printCellGrid();
    placer->placeByForceDirected();
    placer->calculateConnectivity();
#ifdef DEBUG
    placer->printCellGrid();
#endif
    placer->placeFeedThruCells();
    placer->compactAndMapLambda();
    if(debug) placer->printCellGrid();
    placer->writeMagFile();
    //    placer->sortCellArrays();

    //routing stuff
#ifdef CHANNEL_ROUTING
    std::cout << "Channel routing..." << std::endl;
    channel_router channelRouter(placer->get_cells(), netcount);
    delete placer;
    placer = NULL;
    std::cout << channelRouter.route_all();
    std::cout << " of " << channelRouter.get_num_nets() << " terminals routed" << std::endl;
    channelRouter.write_mag_file(filename);
    channelRouter.print_net_stats();
#else
    std::cout << "Maze Routing." << std::endl;
    maze_router mazeRouter(placer->get_cells(), placer->topRowBounding*6+placer->topRowBounding, placer->rightColBounding*6+placer->rightColBounding*3, netcount, nets, filename);
    delete placer;
    placer = NULL;

#endif

    time_t tf = time(NULL);
    printf("Run time: %i hour(s), %i min(s), %i (secs)\n",
            int((tf - t0)/3600), int(((tf - t0)%3600)/60), int((tf - t0)%60));

    return 0;
}
