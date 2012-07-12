/**
 *  @file statmech
 *       test problem for statistical mechanics in cantera
 */

//  Example 
//
// Test case for the statistical mechanics in cantera
//

#include <iostream>
#include <string>
#include <vector>
#include <string>
#include <iomanip>

using namespace std;

/*****************************************************************/
/*****************************************************************/

#include "Cantera.h"
#include "transport.h"
#include "IdealGasMix.h"
#include "equil.h"

#include "TransportFactory.h"

using namespace Cantera;
using namespace Cantera_CXX;

int main(int argc, char** argv) 
{

  try
    {
      int k;
      IdealGasMix g("test_stat.xml");
      int nsp = g.nSpecies();
      double pres = 1.0E5;

      vector_fp Xset(nsp, 0.0);
      Xset[0] =  0.5 ;
      Xset[1] =  0.5;
  
      g.setState_TPX(1500.0, pres, DATA_PTR(Xset));
      equilibrate(g, "TP", -1);

      vector_fp cp_R(nsp, 0.0);
      g.getCp_R(DATA_PTR(cp_R));

      for(int i=0;i<nsp;i++)
	{
	  std::cout << cp_R[i] << std::endl;
	}  

      // error check-- exact for atoms
      if(cp_R[0] != 2.5)
	{
	  std::cout << "Error for monotomic Species!\n";
	  return 1;
	}
    }
    catch (CanteraError) 
      {
	showErrors(cout);
	return 1;
      }

  // Mark it zero!
  return 0;

}
