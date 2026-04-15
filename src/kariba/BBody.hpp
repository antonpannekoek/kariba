#pragma once

#include "Radiation.hpp"

namespace kariba {

//! Class black body photons, inherited from Radiation.hh
class BBody : public Radiation {
  protected:
    double Tbb;
    double Lbb;
    double normbb;

  public:
    BBody(size_t size = 40);

    virtual void set_temp_kev(double T);
    virtual void set_temp_k(double T);
    virtual void set_temp_hz(double nu);
    virtual void set_lum(double L);
    virtual void bb_spectrum();

    virtual double temp_kev() const;
    virtual double temp_k() const;
    virtual double temp_hz() const;
    virtual double lum() const;
    virtual double norm() const;
    virtual double Urad(double d) const;

    virtual void test();
};

}    // namespace kariba
