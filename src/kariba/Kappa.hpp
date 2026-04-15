#pragma once

#include "Particles.hpp"

namespace kariba {

//! Class for kappa distribution of particles, inherited from the generic
//! Particles class in Particles.hh
class Kappa : public Particles {
  protected:
    double theta;
    double kappa, knorm;
    double pmin, pmax;

  public:
    Kappa(size_t size);

    virtual void set_p(double ucom, double bfield, double betaeff, double r, double fsc);
    virtual void set_p(double max);
    virtual void set_ndens();
    virtual void set_kappa(double k);
    virtual void set_temp_kev(double T);
    virtual void set_norm(double n);

    virtual void cooling_steadystate(double ucom, double n0, double bfield, double r,
                                     double gshift);
    virtual double max_p(double ucom, double bfield, double betaeff, double r, double fsc);

    virtual void test();

    friend double norm_kappa_int(double x, void* p);
    friend double injection_kappa_int(double x, void* p);
};

}    // namespace kariba
