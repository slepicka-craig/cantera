/*
 *  Copyright 2002 California Institute of Technology
 */

#include "Cantera.h"
#include "equilibrium.h"
#include "kernel/vcs_MultiPhaseEquil.h"
#include "kernel/vcs_internal.h"

#include "kernel/ThermoFactory.h"
#include "kernel/IdealGasPhase.h"
#include "kernel/HMWSoln.h"
#include "kernel/StoichSubstanceSSTP.h"

using namespace Cantera;
using namespace std;

void printUsage() {
    cout << "usage: nacl_equil [-h] [-help_cmdfile] [-d #] [HMW_NaCl.xml] "
         <<  endl;
    cout << "    -h           help" << endl;
    cout << "    -d           #   : level of debug printing" << endl;
    cout << " [HMW_NaCl.xml]  - Optionally change the name of the input file " << endl;
    cout << endl;
    cout << endl;
}

int main(int argc, char **argv) {
#ifdef _MSC_VER
    _set_output_format(_TWO_DIGIT_EXPONENT);
#endif
  //  int solver = 2;
  int numSucc = 0;
  int numFail = 0;
  int printLvl = 1;
  string inputFile = "HMW_NaCl.xml";
  bool printInputFormat = false; // print cmdfile.txt format 
  bool printedUsage = false;
  VCSnonideal::vcs_timing_print_lvl = 0;

  /*
   * Process the command line arguments
   */
  if (argc > 1) {
    string tok;
    for (int j = 1; j < argc; j++) {
      tok = string(argv[j]);
      if (tok[0] == '-') {
	int nopt = static_cast<int>(tok.size());
	for (int n = 1; n < nopt; n++) {
	  if (!strcmp(tok.c_str() + 1, "help_cmdfile")) {
	    printInputFormat = true;
	  } else if (tok[n] == 'h') {
	    printUsage();
	    printedUsage = true;
	    exit(1);
	  } else if (tok[n] == 'd') {
	    printLvl = 2;
	    int lvl = 2;
	    if (j < (argc - 1)) {
	      string tokla = string(argv[j+1]);
	      if (strlen(tokla.c_str()) > 0) {
		lvl = atoi(tokla.c_str());
		n = nopt - 1;
		j += 1;
		if (lvl >= 0) {
		   printLvl = lvl;
		}
	      }
	    }
	  } else {
	    printUsage();
	    printedUsage = true;
	    exit(1);
	  }
	}
      } else if (inputFile == "HMW_NaCl.xml") {
	inputFile = tok;
      } else {
	printUsage();
	printedUsage = true;
	exit(1);
      }
    }
  }



  try {
    int retnSub;
    int estimateEquil = 0;
    double T = 298.15;
    double pres = OneAtm;

    // Initialize the individual phases

    HMWSoln hmw(inputFile, "");
    int kk = hmw.nSpecies();
    vector_fp Xmol(kk, 0.0);
    int iH2OL = hmw.speciesIndex("H2O(L)");
    Xmol[iH2OL] = 1.0;
    hmw.setState_TPX(T, pres, DATA_PTR(Xmol));

    ThermoPhase *gas = newPhase("gas.xml", "");

    kk = gas->nSpecies();
    Xmol.resize(kk, 0.0);
    for (int i = 0; i < kk; i++) {
      Xmol[i] = 0.0;
    }
    int iN2 = gas->speciesIndex("N2");
    Xmol[iN2] = 1.0;
    gas->setState_TPX(T, pres, DATA_PTR(Xmol));

 
    StoichSubstanceSSTP ss("NaCl_Solid.xml", "");
    ss.setState_TP(T, pres);


    // Construct the multiphase object
    MultiPhase *mp = new MultiPhase();

    mp->addPhase(&hmw, 2.0);
    mp->addPhase(gas, 4.0);
    mp->addPhase(&ss, 5.0);

 
    try {
      retnSub = vcs_equilibrate(*mp, "TP", estimateEquil, printLvl);
      
      cout << *mp;
      if (retnSub != 1) {
	cerr << "ERROR: MultiEquil equilibration step failed at " 
	     << " T    = " << T 
	     << " Pres = " << pres 
	     << endl;
	cout << "ERROR: MultiEquil equilibration step failed at " 
	     << " T    = " << T 
	     << " Pres = " << pres
	     << endl;
	exit(-1);
      }
      numSucc++;
    } catch (CanteraError) {
      cout << *mp;
      showErrors(cerr);
      cerr << "ERROR: MultiEquil equilibration step failed at " 
	   << " T    = " << T 
	   << " Pres = " << pres 
	   << endl;
      cout << "ERROR: MultiEqiul equilibration step failed at " 
	   << " T    = " << T 
	   << " Pres = " << pres
	   << endl;
      exit(-1);
    }

    cout << "NUMBER OF SUCCESSES =  " << numSucc << endl;
    cout << "NUMBER OF FAILURES  =  " << numFail << endl;

    return numFail;
  }
  catch (CanteraError) {
    showErrors(cerr);
    cerr << "ERROR: program terminating due to unforeseen circumstances." << endl;
    return -1;
  }
}
