#pragma once

#include "Particles.hpp"

namespace kariba {

//! Class for non-thermal particles, inherited from the generic Particles class
//! in Particles.hpp note: ndens is number density per unit momentum
class Bknpower : public Particles {
  protected:
    double pspec1, pspec2, norm;
    double pmin, pbrk, pmax;

  public:
    Bknpower(size_t size);

    virtual void set_p(double min, double brk, double ucom, double bfield, double betaeff, double r,
                       double fsc);
    virtual void set_p(double min, double brk, double gmax);
    virtual void set_ndens();
    virtual void set_pspec1(double s1);
    virtual void set_pspec2(double s2);
    virtual void set_brk(double brk);
    virtual void set_norm(double n);

    virtual void cooling_steadystate(double ucom, double n0, double bfield, double r,
                                     double betaeff);
    virtual double max_p(double ucom, double bfield, double bjet, double r, double fsc);

    virtual void test();

    friend double norm_bkn_int(double x, void* p);
    friend double injection_bkn_int(double x, void* p);
};

}    // namespace kariba
