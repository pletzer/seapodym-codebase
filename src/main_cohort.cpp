#include <iostream>
#include <cstring>
#include <cstdlib>
#include "SeapodymCohort.h"
#include <CmdLineArgParser.h>

using std::cout;
SeapodymCohort* seapodym_cohort(const char* parfile, const int cmp_regime, const bool reset_buffers, int cohort_id, gradient_structure& gs);
void buffers_init(long int &mv, long int &mc, long int &mg, const bool grad_calc);
void buffers_set(long int &mv, long int &mc, long int &mg);

int main(int argc, char** argv) {

	// Initialization of MPI
	int err;
	int workerId = 0;
	int num_workers = 1;	
	
	int cmp_regime = 0;
	bool reset_buffers = false;

	CmdLineArgParser cmdLine;
	cmdLine.set("-s", std::string("initparfile.xml"), "Input parameter file");
	// Parse the command line arguments
    bool success = cmdLine.parse(argc, argv);
    bool help = cmdLine.get<bool>("-help") || cmdLine.get<bool>("-h");
    if (!success) {
        std::cerr << "Error parsing command line arguments." << std::endl;
        cmdLine.help();
        exit(1);
    }
    if (help) {
        cmdLine.help();
        exit(1);
    }

	//-----Memory stack sizes for dvariables and derivatives storage------
	gradient_structure::set_YES_SAVE_VARIABLES_VALUES();
	long int gradstack_buffer, cmpdif_buffer, gs_var_buffer;
	bool grad_calc = false;
	if (cmp_regime==-1 || cmp_regime==2 || cmp_regime==4) grad_calc = true;
	buffers_init(gs_var_buffer, gradstack_buffer, cmpdif_buffer, grad_calc);
	if (reset_buffers)
		buffers_set(gs_var_buffer, gradstack_buffer, cmpdif_buffer);

	gradient_structure::set_GRADSTACK_BUFFER_SIZE(gradstack_buffer);
	gradient_structure::set_CMPDIF_BUFFER_SIZE(cmpdif_buffer);
	gradient_structure gs(gs_var_buffer);
	int out_hessian = 0;
	gradient_structure::set_USE_FOR_HESSIAN(out_hessian);

	// cohorts handled by this worker
	const char* parfile = cmdLine.get<std::string>("-s").c_str();

	// Initialize the cohorts for each age group assigned to this worker
	int cohort_id = 0;
	//SeapodymCohort* scp = seapodym_cohort(parfile, cmp_regime, reset_buffers, cohort_id, gs);

	//read parfile
	SeapodymCohort* scp = new SeapodymCohort(parfile, cohort_id);
	SeapodymCohort& sc = *scp;

	//initialize variables of optimization
	const int nvar = sc.nvarcalc();
	independent_variables x(1, nvar);
	adstring_array x_names(1,nvar);

	sc.xinit(x, x_names);
	cout << "Total number of variables: " << nvar << '\n'<<'\n';

	//initialization of simulation
	sc.prerun_model();

	//the function is invoked in the coupled simulation only
	string tempparfile = "tempparfile.xml";
	string newparfile  = "newparfile.xml";

	//after minimization is finished one simulation will 
	//be run with estimated parameters; outputs will be saved
	gradient_structure::set_NO_DERIVATIVES();

	sc.run_cohort((dvar_vector)x, true);

	sc.write(newparfile.c_str());

	remove(tempparfile.c_str());

	//writes new parameters on the screen
	sc.param->outp_param(x_names,nvar);


	scp->OnRunFirstStep();
	delete scp;

	return 0;
}



