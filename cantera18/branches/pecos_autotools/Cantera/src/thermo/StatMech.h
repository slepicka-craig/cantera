/**
 *  @file Nasa9Poly1.h
 *  Header for a single-species standard state object derived
 *  from 
 */
/*
 * Copywrite (2006) Sandia Corporation. Under the terms of 
 * Contract DE-AC04-94AL85000 with Sandia Corporation, the
 * U.S. Government retains certain rights in this software.
 */

#ifndef CT_STATMECH_H
#define CT_STATMECH_H

/* 
 * $Revision: 279 $
 * $Date: 2009-12-05 13:08:43 -0600 (Sat, 05 Dec 2009) $
 */



#include "global.h"
#include "SpeciesThermoInterpType.h"

namespace Cantera {

 
  //! 
  /*!
   * @ingroup spthermo
   */
  class StatMech : public SpeciesThermoInterpType {

  public:

    //! Empty constructor
    StatMech();


    //! constructor used in templated instantiations
    /*!
     * @param n            Species index
     * @param tlow         Minimum temperature
     * @param thigh        Maximum temperature
     * @param pref         reference pressure (Pa).
     * @param coeffs       Vector of coefficients used to set the
     *                     parameters for the standard state.
     */
    StatMech(int n, doublereal tlow, doublereal thigh, doublereal pref,
	       const doublereal* coeffs);

    //! copy constructor
    /*!
     * @param b object to be copied
     */
    StatMech(const StatMech& b);

    //! assignment operator
    /*!
     * @param b object to be copied
     */
    StatMech& operator=(const StatMech& b);

    //! Destructor
    virtual ~StatMech();

    //! duplicator
    virtual SpeciesThermoInterpType *
    duplMyselfAsSpeciesThermoInterpType() const;

    //! Returns the minimum temperature that the thermo
    //! parameterization is valid
    virtual doublereal minTemp() const;

    //! Returns the maximum temperature that the thermo
    //! parameterization is valid
    virtual doublereal maxTemp() const;

    //! Returns the reference pressure (Pa)
    virtual doublereal refPressure() const;

    //! Returns an integer representing the type of parameterization
    virtual int reportType() const;

    //! Returns an integer representing the species index
    virtual int speciesIndex() const;

    //! Update the properties for this species, given a temperature polynomial
    /*!
     * This method is called with a pointer to an array containing the functions of
     * temperature needed by this  parameterization, and three pointers to arrays where the
     * computed property values should be written. This method updates only one value in
     * each array.
     *
     * Temperature Polynomial:
     *  tt[0] = t;
     *  tt[1] = t*t;
     *  tt[2] = t*t*t;
     *  tt[3] = t*t*t*t;
     *  tt[4] = 1.0/t;
     *  tt[5] = 1.0/(t*t);
     *  tt[6] = std::log(t);
     *
     * @param tt      vector of temperature polynomials
     * @param cp_R    Vector of Dimensionless heat capacities.
     *                (length m_kk).
     * @param h_RT    Vector of Dimensionless enthalpies.
     *                (length m_kk).
     * @param s_R     Vector of Dimensionless entropies.
     *                (length m_kk).
     */
    virtual void updateProperties(const doublereal* tt, 
				  doublereal* cp_R, doublereal* h_RT, doublereal* s_R) const;

 
    //! Compute the reference-state property of one species
    /*!
     * Given temperature T in K, this method updates the values of
     * the non-dimensional heat capacity at constant pressure,
     * enthalpy, and entropy, at the reference pressure, Pref
     * of one of the species. The species index is used
     * to reference into the cp_R, h_RT, and s_R arrays.
     *
     * Temperature Polynomial:
     *  tt[0] = t;
     *  tt[1] = t*t;
     *  tt[2] = t*t*t;
     *  tt[3] = t*t*t*t;
     *  tt[4] = 1.0/t;
     *  tt[5] = 1.0/(t*t);
     *  tt[6] = std::log(t);
     *
     * @param temp    Temperature (Kelvin)
     * @param cp_R    Vector of Dimensionless heat capacities.
     *                (length m_kk).
     * @param h_RT    Vector of Dimensionless enthalpies.
     *                (length m_kk).
     * @param s_R     Vector of Dimensionless entropies.
     *                (length m_kk).
     */
    virtual void updatePropertiesTemp(const doublereal temp, 
				      doublereal* cp_R, doublereal* h_RT, 
				      doublereal* s_R) const;

    //!This utility function reports back the type of 
    //! parameterization and all of the parameters for the 
    //! species, index.
    /*!
     * All parameters are output variables
     *
     * @param n         Species index
     * @param type      Integer type of the standard type
     * @param tlow      output - Minimum temperature
     * @param thigh     output - Maximum temperature
     * @param pref      output - reference pressure (Pa).
     * @param coeffs    Vector of coefficients used to set the
     *                  parameters for the standard state. There are
     *                  12 of them, designed to be compatible
     *                  with the multiple temperature formulation.
     *      coeffs[0] is equal to one.
     *      coeffs[1] is min temperature
     *      coeffs[2] is max temperature
     *      coeffs[3+i] from i =0,9 are the coefficients themselves
     */
    virtual void reportParameters(int &n, int &type,
				  doublereal &tlow, doublereal &thigh,
				  doublereal &pref,
				  doublereal* const coeffs) const;

    //! Modify parameters for the standard state
    /*!
     * @param coeffs   Vector of coefficients used to set the
     *                 parameters for the standard state.
     */
    virtual void modifyParameters(doublereal* coeffs);

  protected:
    //! lowest valid temperature
    doublereal m_lowT;    
    //! highest valid temperature
    doublereal m_highT;   
    //! standard-state pressure
    doublereal m_Pref;     
    //! species index
    int m_index;  
    //! array of polynomial coefficients       
    array_fp m_coeff;
  };

}
#endif

