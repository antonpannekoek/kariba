#pragma once

#include <vector>

namespace kariba {

//! Structure used for GSL integration
struct PlParams {
    double s;
    double n;
};

//! Structure used for GSL integration
struct BknParams {
    double s1;
    double s2;
    double brk;
    double max;
    double m;
};

//! Structure used for GSL integration
struct ThParams {
    double t;
    double n;
    double m;
};

//! Structure used for GSL integration
struct KParams {
    double t;
    double k;
};

//! Structure used for GSL integration
struct InjectionMixedParams {
    double s;
    double t;
    double nth;
    double npl;
    double m;
    double min;
    double max;
    double cutoff;
};

//! Structure used for GSL integration
struct InjectionKappaParams {
    double t;
    double k;
    double n;
    double m;
};

//! Structure used for GSL integration
struct InjectionPlParams {
    double s;
    double n;
    double m;
    double max;
};

//! Structure used for GSL integration
struct InjectionBknParams {
    double s1;
    double s2;
    double brk;
    double max;
    double m;
    double n;
};

//! Template class for particle distributions
//! This class contains members and methods that are used for thermal,
//! non-thermal and mixed distributions
class Particles {
  protected:
    double mass_gr;     //!< particle mass in grams
    double mass_kev;    //!< same as above but in keV, using electrons as "reference"

    std::vector<double> p;        //!< array of particle momenta
    std::vector<double> ndens;    //!< array of number density per unit volume, per unit momentum
    std::vector<double> gamma;    //!< array of particle kinetic energies for each momentum
    std::vector<double> gdens;    //!< array of number density per unit volume, per unit gamma
    std::vector<double> gdens_diff;    //!< array with differential of number
                                       //!< density for radiation calculation

  public:
    Particles(size_t size);

    virtual void set_mass(double m);
    virtual void initialize_gdens();
    virtual void initialize_pdens();
    virtual void gdens_differentiate();

    virtual const std::vector<double>& get_p() const { return p; }

    virtual const std::vector<double>& get_pdens() const { return ndens; }

    virtual const std::vector<double>& get_gamma() const { return gamma; }

    virtual const std::vector<double>& get_gdens() const { return gdens; }

    virtual const std::vector<double>& get_gdens_diff() const { return gdens_diff; }

    virtual double count_particles();
    virtual double count_particles_energy();
    virtual double av_p();
    virtual double av_gamma();
    virtual double av_psq();
    virtual double av_gammasq();

    virtual void test_arrays();
};

}    // namespace kariba
