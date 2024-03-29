//------------------------------------------------
///
///  @file ReactionStoichMgr.cpp
///
///
//------------------------------------------------

// $Author$
// $Revision$
// $Date$

// turn off warnings under Windows
#ifdef WIN32
#pragma warning(disable:4786)
#pragma warning(disable:4503)
#endif


#include "ReactionStoichMgr.h"
#include "StoichManager.h"
#include "ctexceptions.h"
#include "ReactionData.h"

using namespace std;

namespace Cantera {

  // create stoichiometry managers for the reactants of all reactions,
  // for the products of the reversible reactions, and for the
  // products of the irreversible reactions.
  ReactionStoichMgr::
  ReactionStoichMgr() {
    m_reactants = new StoichManagerN;
    m_revproducts = new StoichManagerN;
    m_irrevproducts = new StoichManagerN;
#ifdef INCL_STOICH_WRITER
    m_rwriter = new StoichWriter;
#endif
    m_dummy.resize(10,1.0);
  }

  // delete the three stoichiometry managers
  ReactionStoichMgr::~ReactionStoichMgr() {
    delete m_reactants;
    delete m_revproducts;
    delete m_irrevproducts;
    //    delete m_global;
#ifdef INCL_STOICH_WRITER
    delete m_rwriter;
#endif
  }


  void ReactionStoichMgr::
  add(int rxn, const vector_int& reactants, const vector_int& products,
      bool reversible) {

    m_reactants->add(rxn, reactants);

    if (reversible) 
      m_revproducts->add(rxn, products);
    else
      m_irrevproducts->add(rxn, products);
  }


  void ReactionStoichMgr::
  add(int rxn, const ReactionData& r) {

    vector_int rk;
    doublereal frac;
    bool isfrac = false;
    int n, ns, m, nr = r.reactants.size();
    for (n = 0; n < nr; n++) {
      ns = int(r.rstoich[n]);
      frac = r.rstoich[n] - 1.0*int(r.rstoich[n]);
      if (frac != 0.0) isfrac = true;
      for (m = 0; m < ns; m++) {
	rk.push_back(r.reactants[n]);
      }
    }

    // if the reaction has fractional stoichiometric coefficients
    // or specified reaction orders, then add it in a general reaction
    if (isfrac || r.global || rk.size() > 3) {
      m_reactants->add(rxn, r.reactants, r.rorder, r.rstoich);
#ifdef INCL_STOICH_WRITER
      if (m_rwriter) m_rwriter->add(rxn, r.reactants, r.order, r.rstoich);
#endif
    }
    else {
      m_reactants->add( rxn, rk);
#ifdef INCL_STOICH_WRITER
      if (m_rwriter) m_rwriter->add(rxn, rk);
#endif
    }

    vector_int pk;
    isfrac = false;
    int np = r.products.size();
    for (n = 0; n < np; n++) {
      ns = int(r.pstoich[n]);
      frac = r.pstoich[n] - 1.0*int(r.pstoich[n]);
      if (frac != 0.0) isfrac = true;
      for (m = 0; m < ns; m++) {
	pk.push_back(r.products[n]);
      }
    }

    if (r.reversible) {
      if (isfrac && !r.isReversibleWithFrac) {
	throw CanteraError("ReactionStoichMgr::add",
			   "Fractional product stoichiometric coefficients only allowed "
			   "\nfor irreversible reactions and most reversible reactions");
      }
      if (pk.size() > 3 || r.isReversibleWithFrac) {
	m_revproducts->add(rxn, r.products, r.porder, r.pstoich);
      }
      else {
	m_revproducts->add(rxn, pk);
      }
    }
    else if (isfrac || pk.size() > 3) {            
      m_irrevproducts->add(rxn, r.products, r.porder,  r.pstoich);
    }
    else {   
      m_irrevproducts->add(rxn, pk);
    }
  }

  void ReactionStoichMgr::
  getCreationRates(int nsp, const doublereal* ropf, 
		   const doublereal* ropr, doublereal* c) {
    // zero out the output array
    fill(c, c + nsp, 0.0);
    
    // the forward direction creates product species
    m_revproducts->incrementSpecies(ropf, c);
    m_irrevproducts->incrementSpecies(ropf, c);

    // the reverse direction creates reactant species
    m_reactants->incrementSpecies(ropr, c);
  }

  void ReactionStoichMgr::
  getDestructionRates(int nsp, const doublereal* ropf, 
		      const doublereal* ropr, doublereal* d) {
    fill(d, d + nsp, 0.0);
    // the reverse direction destroys products in reversible reactions
    m_revproducts->incrementSpecies(ropr, d);
    // the forward direction destroys reactants
    m_reactants->incrementSpecies(ropf, d);
  }

  void ReactionStoichMgr::
  getNetProductionRates(int nsp, const doublereal* ropnet, doublereal* w) {
    fill(w, w + nsp, 0.0);
    // products are created for positive net rate of progress
    m_revproducts->incrementSpecies(ropnet, w);
    m_irrevproducts->incrementSpecies(ropnet, w);
    // reactants are destroyed for positive net rate of progress
    m_reactants->decrementSpecies(ropnet, w);
  }

  void ReactionStoichMgr::
  getReactionDelta(int nr, const doublereal* g, doublereal* dg) {
    fill(dg, dg + nr, 0.0);
    // products add 
    m_revproducts->incrementReactions(g, dg);
    m_irrevproducts->incrementReactions(g, dg);
    // reactants subtract
    m_reactants->decrementReactions(g, dg);
  }

  void ReactionStoichMgr::
  getRevReactionDelta(int nr, const doublereal* g, doublereal* dg) {
    fill(dg, dg + nr, 0.0);
    m_revproducts->incrementReactions(g, dg);
    m_reactants->decrementReactions(g, dg);
  }

  void ReactionStoichMgr::
  multiplyReactants(const doublereal* c, doublereal* r) {
    m_reactants->multiply(c, r);
  }

  void ReactionStoichMgr::
  multiplyRevProducts(const doublereal* c, doublereal* r) {
    m_revproducts->multiply(c, r);
  }

    
  void ReactionStoichMgr::
  write(string filename) {
    ofstream f(filename.c_str());
    f << "namespace mech {" << endl;
    writeCreationRates(f);
    writeDestructionRates(f);
    writeNetProductionRates(f);
    writeMultiplyReactants(f);
    writeMultiplyRevProducts(f);
    f << "} // namespace mech" << endl;
    f.close();
  }

  void ReactionStoichMgr::
  writeCreationRates(ostream& f) {
    f << "    void getCreationRates(const doublereal* rf, const doublereal* rb," << endl;
    f << "          doublereal* c) {" << endl;
    map<int, string> out;
    m_revproducts->writeIncrementSpecies("rf",out);
    m_irrevproducts->writeIncrementSpecies("rf",out);
    m_reactants->writeIncrementSpecies("rb",out);
    map<int, string>::iterator b;
    for (b = out.begin(); b != out.end(); ++b) {
      string rhs = wrapString(b->second);
      rhs[1] = '=';
      f << "     c[" << b->first << "] " << rhs << ";" << endl;
    }
    f << "    }" << endl << endl << endl;
  }  

  void ReactionStoichMgr::
  writeDestructionRates(ostream& f) {
    f << "    void getDestructionRates(const doublereal* rf, const doublereal* rb," << endl;
    f << "          doublereal* d) {" << endl;
    map<int, string> out;
    m_revproducts->writeIncrementSpecies("rb",out);
    m_reactants->writeIncrementSpecies("rf",out);
    map<int, string>::iterator b;
    for (b = out.begin(); b != out.end(); ++b) {
      string rhs = wrapString(b->second);
      rhs[1] = '=';
      f << "     d[" << b->first << "] " << rhs << ";" << endl;
    }
    f << "    }" << endl << endl << endl;
  }

  void ReactionStoichMgr::
  writeNetProductionRates(ostream& f) {
    f << "    void getNetProductionRates(const doublereal* r, doublereal* w) {" << endl;
    map<int, string> out;
    m_revproducts->writeIncrementSpecies("r",out);
    m_irrevproducts->writeIncrementSpecies("r",out);
    m_reactants->writeDecrementSpecies("r",out);
    map<int, string>::iterator b;
    for (b = out.begin(); b != out.end(); ++b) {
      string rhs = wrapString(b->second);
      rhs[1] = '=';
      f << "     w[" << b->first << "] " << rhs << ";" << endl;
    }
    f << "    }" << endl << endl << endl;
  }

  void ReactionStoichMgr::
  writeMultiplyReactants(ostream& f) {
    f << "    void multiplyReactants(const doublereal* c, doublereal* r) {" << endl;
    map<int, string> out;
    m_reactants->writeMultiply("c",out);
    map<int, string>::iterator b;
    for (b = out.begin(); b != out.end(); ++b) {
      string rhs = b->second;
      f << "      r[" << b->first << "] *= " << rhs << ";" << endl;
    }
    f << "    }" << endl << endl << endl;
  }  

  void ReactionStoichMgr::
  writeMultiplyRevProducts(ostream& f) {
    f << "    void multiplyRevProducts(const doublereal* c, doublereal* r) {" << endl;
    map<int, string> out;
    m_revproducts->writeMultiply("c",out);
    map<int, string>::iterator b;
    for (b = out.begin(); b != out.end(); ++b) {
      string rhs = b->second;
      f << "      r[" << b->first << "] *= " << rhs << ";" << endl;
    }
    f << "    }" << endl << endl << endl;
  }  
}
