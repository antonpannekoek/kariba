#pragma once

#include "Radiation.hpp"

namespace kariba {

//! Class Shakura-Sunyeav disk, inherited from Radiation.hpp
class ShSDisk : public Radiation {
  protected:
    double Tin;        //!< Disk temperature at R=Rin in Kev
    double Mbh, Rg;    //!< Black hole mass in solar masses
    double Ldisk;      //!< Disk luminosity in Eddington units
    double Hratio;     //!< Disk aspect ratio; TODO test with constant hbb or with
                       //!< hbb varying with disk size. Also test with just one
                       //!< Comptonization zone, and with one up to hbb(Rin) and
                       //!< another up to the end of the nozzle

  public:
    ShSDisk(size_t size = 50);

    virtual double hdisk() const { return Hratio; };

    virtual double tin() const { return Tin; };

    virtual double rin() const { return r; };

    virtual double lum() const { return Ldisk; };

    virtual void disk_spectrum();
    virtual void cover_disk(double f);

    virtual double total_luminosity();

    virtual void set_mbh(double M);
    virtual void set_rin(double R);
    virtual void set_rout(double R);
    virtual void set_luminosity(double L);
    virtual void set_tin_k(double T);
    virtual void set_tin_kev(double T);

    virtual void test();

    friend double disk_int(double nu, void* p);
};

}    // namespace kariba
