/*************************************************************************************************************
Gamma-rays from neutral pion decay, products of inelastic pp and pγ collisions
*************************************************************************************************************/
#pragma once

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include "Radiation.hpp"

namespace kariba {

struct HetagParams {
    // eq 70 from KA08 for photons and writen as 0< x=Eg/Ep <1
    double eta;
    double eta_zero;
    double Eg;
    double gp_min;
    double gp_max;
    gsl_spline* spline_Jp;
    gsl_interp_accel* acc_Jp;
    gsl_interp_accel* acc_ng;
    gsl_spline* spline_ng;
    double nu_min;
    double nu_max;
};

class Grays : public Radiation {
  public:
    Grays(size_t size, double numin, double numax);

    //! Method to set the gamma-rays from pp inelastic interactions. p: pspec_p,
    //! ntot_prot: total proton number density of the jet segment,
    //! ntargets: the number density of external proton density (companion etc),
    //! plfrac: plfrac_p
    virtual void set_grays_pp(double p, double gammap_min, double gammap_max, double ntot_prot,
                              double ntargets, double plfrac, gsl_interp_accel* acc_Jp,
                              gsl_spline* spline_Jp);

    virtual void set_grays_pg(double gp_min, double gp_max, gsl_interp_accel* acc_Jp,
                              gsl_spline* spline_Jp, std::vector<double>& nu_per_seg,
                              std::vector<double>& ng_per_seg, size_t ne);
};

//! Adds up in the lum_perseg the target photon luminosity (in erg/sec/Hz)
void sum_photons(size_t nphot, std::vector<double>& en_perseg, std::vector<double>& lum_perseg,
                 size_t ntarg, const std::vector<double>& targ_en,
                 const std::vector<double>& targ_lum);
void sum_photons(size_t nphot, const std::vector<double>& en_perseg,
                 std::vector<double>& lum_perseg, size_t ntarg, const std::vector<double>& targ_en,
                 const std::vector<double>& targ_lum);

//! funtions for γ rays from pγ
double Hetag(double x, void* p);

//! The following are common for γ rays/electrons/neutrinos from pp:
double set_ntilde(double p);
double target_protons(double ntot_prot, double ntargets, double plfrac);
double sigma_pp(double Ep);
double proton_dist(double gpmin, double Ep, double Epcode_max, gsl_spline* spline_Jp,
                   gsl_interp_accel* acc_Jp);
double gspec_pp(double Ep, double y);

//! The following are common for γ rays/electrons/neutrinos from pγ:
double colliding_protons(gsl_spline* spline_Jp, gsl_interp_accel* acc_Jp, double gp_min,
                         double gp_max, double Ep);
double photons_jet(double eta, double Ep, gsl_spline* spline_ng, gsl_interp_accel* acc_ng,
                   double nu_min, double nu_max);
void tables_photomeson_gamma(double& s, double& delta, double& Beta, double xeta);
double PhiFunc_gamma(double eta, double eta0, double x);

}    // namespace kariba
