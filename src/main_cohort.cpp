#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include <mpi.h>
#include <CmdLineArgParser.h>
#include "SeapodymCohort.h"

using std::cout;
SeapodymCohort* seapodym_cohort(const char* parfile, const int cmp_regime, const bool reset_buffers);

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

	// Assign the cohorts to the workers
	int num_age_groups = cmdLine.get<int>("-na");
	// Worker Id to list of cohort Ids
	std::vector< std::vector<int> >cohorts_per_worker(numWorkers);
	for (auto ia = 0; ia < num_age_groups; ++ia) {
		// Initially, the cohort Id is the same as the age index
		cohorts_per_worker[ia % numWorkers].push_back(ia);
	}

	// The cohort Ids for this worker
	std::vector<int> cohort_ids = cohorts_per_worker[workerId];

	
	int cmp_regime = -1;
	bool reset_buffers = false;
	const char* parfile = cmdLine.get<std::string>("-s").c_str();

	std::vector<SeapodymCohort*> cohorts;
	for (auto& cohort_id : cohort_ids) {
		SeapodymCohort* scp = seapodym_cohort(parfile, cmp_regime, reset_buffers);
		cohorts.push_back(scp);
	}

	// Clean up the cohorts
	for (auto& scp : cohorts) {
		if (scp) {
			// currently this can causes a segmentation fault
			//delete scp; // Free the memory allocated for the cohort
		}
	}


	// Finalization of MPI
	err = MPI_Finalize();

	return 0;
}




