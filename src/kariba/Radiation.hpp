#pragma once

#include <string>
#include <vector>

#include <gsl/gsl_spline.h>

namespace kariba {

//! Structure used for GSL integration
struct CyclosynEmisParams {
    double nu;
    double b;
    gsl_spline* syn;
    gsl_interp_accel* acc_syn;
    gsl_spline* eldis;
    gsl_interp_accel* acc_eldis;
};

//! Structure used for GSL integration
struct CyclosynAbsParams {
    double nu;
    double b;
    gsl_spline* syn;
    gsl_interp_accel* acc_syn;
    gsl_spline* derivs;
    gsl_interp_accel* acc_derivs;
};

//! Structure used for GSL integration
struct ComintParams {
    double eph;
    double ephmin;
    double ephmax;
    gsl_spline* eldis;
    gsl_interp_accel* acc_eldis;
    gsl_spline* phodis;
    gsl_interp_accel* acc_phodis;
};

//! Structure used for GSL integration
struct ComfncParams {
    double game;
    double e1;
    gsl_spline* phodis;
    gsl_interp_accel* acc_phodis;
};

//! Structure used for GSL integration
struct DiskObsParams {
    double tin;
    double rin;
    double nu;
};

//! Structure used for GSL integration
struct DiskIcParams {
    double gamma;
    double beta;
    double tin;
    double rin;
    double rout;
    double h;
    double z;
    double nu;
};

//! Base class for photon/neutrino distributions
class Radiation {
  protected:
    std::vector<double> en_phot;         //!< array of photon energies
    std::vector<double> num_phot;        //!< array of number of photons in units of erg/s/Hz
    std::vector<double> en_phot_obs;     //!< same as above but in observer frame
    std::vector<double> num_phot_obs;    //!< same as above but in observer frame

    double r, z;             //!< Dimensions of emitting region
    double vol;              //!< Volume of emitting region
    double beta;             //!< speed of the emitting region
    double dopfac, angle;    //!< Viewing angle/Doppler factor of emitting region
    double dopnum;           //!< Doppler boosting exponent, depends on geometry
    bool counterjet;         //!< boolean switch if user wants to include counterjet emission
    std::string geometry;    //!< string to track geometry of emitting region

  public:
    Radiation(size_t size);

    virtual const std::vector<double>& get_energy() const { return en_phot; }

    virtual const std::vector<double>& get_nphot() const { return num_phot; }

    virtual const std::vector<double>& get_energy_obs() const { return en_phot_obs; }

    virtual const std::vector<double>& get_nphot_obs() const { return num_phot_obs; }

    virtual size_t get_size() const { return en_phot.size(); }

    virtual double get_volume() const { return vol; }

    virtual double integrated_luminosity(double numin, double numax);

    virtual void set_beaming(double theta, double speed, double doppler);
    virtual void set_inclination(double theta);
    virtual void set_geometry(const std::string& geom, double l1, double l2);
    virtual void set_geometry(const std::string& geom, double l1);

    virtual void set_counterjet(bool flag);
    virtual void test_arrays();
};
}    // namespace kariba
