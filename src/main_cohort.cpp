#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <mpi.h>
#include <CmdLineArgParser.h>
//#include "SeapodymCohort.h"

using std::cout;
int seapodym_cohort(const char* parfile, const int cmp_regime, const bool reset_buffers);

int main(int argc, char** argv) {

	// Initialization of MPI
	int err;
	err = MPI_Init(&argc, &argv);
	int numWorkers;
	MPI_Comm_size(MPI_COMM_WORLD, &numWorkers);
	int workerId;
	MPI_Comm_rank(MPI_COMM_WORLD, &workerId);

    CmdLineArgParser cmdLine;
	cmdLine.set("-s", std::string("initparfile.xml"), "Input file with parameters");

	// These could come from the input file (?)
    cmdLine.set("-na", 1, "Number of age groups");

	// Parse the command line arguments
    bool success = cmdLine.parse(argc, argv);
    bool help = cmdLine.get<bool>("-help") || cmdLine.get<bool>("-h");
    if (!success) {
        if (workerId == 0) {
			std::cerr << "Error parsing command line arguments." << std::endl;
			cmdLine.help();
		}
        MPI_Finalize();
        return 1;
    }
    if (help) {
        if (workerId == 0) cmdLine.help();
        MPI_Finalize();
        return 1;
    }


	
	
	
	int cmp_regime = -1;
	bool reset_buffers = false;

	// Assign the cohorts to the workers


	//std::vector<SeapodymCohort*> cohorts;
	std::vector<int> cohort_steps;
	std::vector<int> cohort_ids;
	std::vector<int> cohort_numsteps;

	const char* parfile = cmdLine.get<std::string>("-s").c_str();
	err = seapodym_cohort(parfile, cmp_regime, reset_buffers);
	if (err != 0) {
		cout << "Error in seapodym_cohort: " << err << "\n";
		// This will abort all processes in the MPI_COMM_WORLD communicator
		MPI_Abort(MPI_COMM_WORLD, err);
	}

	// Finalization of MPI
	err = MPI_Finalize();

	return 0;
}




