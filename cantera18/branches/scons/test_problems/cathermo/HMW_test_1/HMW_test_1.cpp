/* ======================================================================= */



/* ======================================================================= */


#include "Cantera.h"
#include "kernel/HMWSoln.h"

#include <cstdio>

using namespace std;
using namespace Cantera;

int CHECK_DEBUG_MODE = 0;

void printUsage() {
    cout << "usage: HMW_test_1 " <<  endl;
    cout <<"                -> Everything is hardwired" << endl;
}

void pAtable(HMWSoln *HMW) {
    int nsp = HMW->nSpecies();
    double acMol[100];
    double mf[100]; 
    double activities[100]; 
    double moll[100];
    
    if (CHECK_DEBUG_MODE == 1) {
      HMW->m_debugCalc = 1;
    }
    HMW->getMolalityActivityCoefficients(acMol);
    HMW->getMoleFractions(mf);
    HMW->getActivities(activities);
    HMW->m_debugCalc = 0;
    HMW->getMolalities(moll);
    string sName;
    printf("            Name      Activity  ActCoeffMolal "
	   "   MoleFract      Molality\n");
    for (int k = 0; k < nsp; k++) {
      sName = HMW->speciesName(k);
      printf("%16s %13g %13g %13g %13g\n", 
	     sName.c_str(), activities[k], acMol[k], mf[k], moll[k]);
    }

}

int main(int argc, char **argv)
{

   int retn = 0;

   try {
  
     HMWSoln *HMW = new HMWSoln(1);

#ifdef DEBUG_MODE
     CHECK_DEBUG_MODE = 1;
#endif
     if (CHECK_DEBUG_MODE == 1) {
       HMW->m_debugCalc = 1;
       if (HMW->debugPrinting()) {
         FILE *ff = fopen("CheckDebug.txt", "w");
         fprintf(ff,"%1d\n", 1);
         fclose(ff);
       }
       HMW->m_debugCalc = 0;
     }
     
     int nsp = HMW->nSpecies();
    
     /*
      *
      */
     double a1 = HMW->AionicRadius(1);
     printf("a1 = %g\n", a1);
     double a2 = HMW->AionicRadius(2);
     printf("a2 = %g\n", a2); 
     double mu0[100];
     double moll[100]; 
     string sName;
  
     HMW->getMolalities(moll);
     moll[1] = 6.0997;
     moll[2] = 2.1628E-9;
     moll[3] = 6.0997;
     moll[4] =1.3977E-6;
     /*
      * Equalize charge balance and dump into Cl-
      */
     double sum = -moll[1] + moll[2] + moll[3] - moll[4];
     moll[1] += sum;

     HMW->setMolalities(moll);
     HMW->setState_TP(298.15, 1.01325E5);
     pAtable(HMW);

     HMW->setState_TP(298.15, 1.01325E5);
     HMW->getStandardChemPotentials(mu0);
     // translate from J/kmol to kJ/gmol
     int k;
     for (k = 0; k < nsp; k++) {
        mu0[k] *= 1.0E-6; 
     }

     printf("           Species   Standard chemical potentials (kJ/gmol) \n");
     printf("------------------------------------------------------------\n");
     for (k = 0; k < nsp; k++) {
       sName = HMW->speciesName(k);
       printf("%16s %16.9g\n", sName.c_str(), mu0[k]);
     }
     printf("------------------------------------------------------------\n");
     printf(" Some DeltaSS values:               Delta(mu_0)\n");
     double deltaG;
     int i1, i2, j1;
     double RT = 8.314472E-3 * 298.15;

 
     
   
 
     i1 = HMW->speciesIndex("Na+");
     i2 = HMW->speciesIndex("Cl-");
     deltaG = -432.6304 - mu0[i1] - mu0[i2];
     printf(" NaCl(S): Na+ + Cl- -> NaCl(S): %14.7g kJ/gmol \n",
            deltaG);
     printf("                                : %14.7g (dimensionless) \n",
            deltaG/RT);
     printf("                                : %14.7g (dimensionless/ln10) \n",
            deltaG/(RT * log(10.0)));
     printf("            G0(NaCl(S)) = %14.7g (fixed)\n", -432.6304);
     printf("            G0(Na+)     = %14.7g\n", mu0[i1]);
     printf("            G0(Cl-)     = %14.7g\n", mu0[i2]);

     i1 = HMW->speciesIndex("H+");
     i2 = HMW->speciesIndex("H2O(L)");
     j1 = HMW->speciesIndex("OH-");
     if (i1 < 0 || i2 < 0 || j1 < 0) {
       printf("problems\n");
       exit(-1);
     }
     deltaG = mu0[j1] + mu0[i1] - mu0[i2];
     printf(" OH-: H2O(L) - H+ -> OH-: %14.7g kJ/gmol \n",
            deltaG);
     printf("                                : %14.7g (dimensionless) \n",
            deltaG/RT);
     printf("                                : %14.7g (dimensionless/ln10) \n",
            deltaG/(RT * log(10.0)));
     printf("            G0(OH-)    = %14.7g\n", mu0[j1]);
     printf("            G0(H+)     = %14.7g\n", mu0[i1]);
     printf("            G0(H2O(L)) = %14.7g\n", mu0[i2]);


     printf("------------------------------------------------------------\n");

     delete HMW;
     HMW = 0;
     Cantera::appdelete();

     return retn;

   } catch (CanteraError) {

     showErrors();
     return -1;
   }
} 
