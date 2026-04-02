#pragma once

#include "Particles.hpp"

namespace kariba {

//! Class for thermal particles. ndens is number density per unit momentum
class Thermal : public Particles {
  protected:
    double Temp, thnorm, theta;

  public:
    Thermal(size_t size);

    virtual void set_p();
    virtual void set_ndens();
    virtual void set_temp_kev(double T);
    virtual void set_norm(double n);

    virtual double K2(double x);

    virtual void test();
};

}    // namespace kariba
