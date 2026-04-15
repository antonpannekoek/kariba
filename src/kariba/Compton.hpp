#pragma once

#include <gsl/gsl_spline2d.h>

#include "Radiation.hpp"

namespace kariba {

//! Class inverse Compton, inherited from Radiation.hpp
class Compton : public Radiation {
  protected:
    size_t Niter;          //!< number of IC iterations
    double tau, ypar;      //!< optical depth/comtpon Y of emitting region
    double rphot;          //!< photospheric radius when tau > 1, used to renormalize volume
    double escape_corr;    //!< escape term, used to renormalize our spectra to CompPS

    std::vector<double> seed_energ;    //!< array of seed frequencies in Hz
    std::vector<double> seed_urad;     //!< array of seed photon number density in log10(#/erg/cm^3)
    std::vector<double>
        iter_urad;    //!< array of iterated photon number density in log10(#/erg/cm^3)

    gsl_spline* seed_ph;           //!< interpolation of photon field array seed_urad
    gsl_interp_accel* acc_seed;    //!< accelerator for above spline

    gsl_spline* iter_ph;           //!< interpolation of photon field for multiple scatters
    gsl_interp_accel* acc_iter;    //!< accelerator of above spline

    gsl_spline2d* esc_p_sph;    //!< interpolation for escape calculation to mimic
    //!< radiative transfer
    gsl_spline2d* esc_p_cyl;    //!< interpolation for escape calculation to mimic
    //!< radiative transfer
    gsl_interp_accel* acc_tau;    //!< accelerator of above spline over tau
    gsl_interp_accel* acc_Te;     //!< accelerator of above spline over Te

  public:
    ~Compton();
    Compton(size_t size, size_t seed_size);

    virtual double comintegral(size_t it, double blim, double ulim, double nu, double numin,
                               double numax, gsl_spline* eldis, gsl_interp_accel* acc_eldis);
    virtual void compton_spectrum(double gmin, double gmax, gsl_spline* eldis,
                                  gsl_interp_accel* acc_eldis);

    virtual void cyclosyn_seed(const std::vector<double>& seed_arr,
                               const std::vector<double>& seed_lum);
    virtual void bb_seed_k(const std::vector<double>& seed_arr, double Urad, double Tbb);
    virtual void bb_seed_kev(const std::vector<double>& seed_energ, double Urad, double Tbb);
    virtual void shsdisk_seed(const std::vector<double>& seed_arr, double tin, double rin,
                              double rout, double h, double z);

    virtual void set_frequency(double numin, double numax);
    virtual void set_tau(double n, double gam);
    virtual void set_tau(double _tau);
    virtual void set_escape(double escape);
    virtual void set_niter(double nu0, double Te);
    virtual void set_niter(size_t n);
    virtual void seed_freq_array(const std::vector<double>& seed_energ);

    virtual double get_tau() const { return tau; };

    virtual double get_ypar() const { return ypar; };

    virtual void reset();
    virtual void urad_test();
    virtual void test();

    friend double comfnc(double ein, void* p);
    friend double comint(double gam, void* p);
    friend double disk_integral(double alfa, void* p);
};

}    // namespace kariba
