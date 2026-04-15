#pragma once

#include "Particles.hpp"

namespace kariba {

//! Class for mixed particles, inherited from the generic Particles class in
//! Particles.hh the minimum momentum of the PL is always be assumed to be the
//! averge momentum of the thermal note: ndens is number density per unit
//! momentum
//!
//! The reasons the integrands are friends is kind of magic. Basically, we need
//! pointers to the private members Temp/thnorm/theta/whatever to set the
//! parameters of the integrand functions in the GSL libraries. In C++ this is
//! not possible for non-static private member functions (because otherwise we
//! could access private members from the outside by using a pointer). To quote
//! the compiler error, C++ forbids taking the address of an unqualified or
//! parenthesized non-static member function to form a pointer to member
//! function. And we need a pointer member function void *p to set up the
//! integrands for GSL libraries. By using a friend function we can instead set
//! up a set of parameters whose value is that of a pointer which points at the
//! values stored in the private members; by doing this, we can not change the
//! private members' values (correctly so) but we can still access their
//! numerical value and use it elsewhere.
class Mixed : public Particles {
  protected:
    double thnorm, theta, Temp;
    double pspec, plnorm;
    double pmin_th, pmax_th, pmin_pl, pmax_pl;
    double plfrac;

  public:
    Mixed(size_t size);

    virtual void set_p(double ucom, double bfield, double betaeff, double r, double fsc);
    virtual void set_p(double gmax);
    virtual void set_ndens();
    virtual void set_temp_kev(double T);
    virtual void set_norm(double n);
    virtual void set_plfrac(double f);
    virtual void set_plfrac(double Le, double r, double eldens);
    virtual void set_pspec(double s1);

    virtual void cooling_steadystate(double ucom, double n0, double bfield, double r,
                                     double betaeff);
    virtual double max_p(double ucom, double bfield, double betaeff, double r, double fsc);

    virtual double count_th_particles();
    virtual double av_th_p();
    virtual double av_th_gamma();

    virtual double count_pl_particles();
    virtual double av_pl_p();
    virtual double av_pl_gamma();

    virtual double K2(double x);

    virtual void test();

    friend double th_num_dens_int(double x, void* p);
    friend double av_th_p_int(double x, void* p);

    friend double pl_num_dens_int(double x, void* p);
    friend double av_pl_p_int(double x, void* p);
};

}    // namespace kariba
