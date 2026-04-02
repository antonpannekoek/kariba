#pragma once

#include <string>

#include <gsl/gsl_interp.h>
#include <gsl/gsl_spline.h>

#include "Radiation.hpp"

namespace kariba {

struct HetaParams {
    // eq 70 from KA08 for photons and writen as 0< x=Eg/Ep <1
    double eta;
    double eta_zero;
    double E;
    double gp_min;
    double gp_max;
    gsl_spline* spline_Jp;
    gsl_interp_accel* acc_Jp;
    std::string product;
    gsl_interp_accel* acc_ng;
    gsl_spline* spline_ng;
    double nu_min;
    double nu_max;
};

class Neutrinos_pg : public Radiation {
  public:
    Neutrinos_pg(size_t size, double Emin, double Emax);

    virtual void set_neutrinos(double gp_min, double gp_max, gsl_interp_accel* acc_Jp,
                               gsl_spline* spline_Jp, const std::vector<double>& en_perseg,
                               const std::vector<double>& lum_perseg, size_t nphot,
                               const std::string& outputConfiguration, const std::string& flavor,
                               int infosw, std::string_view source);
};

double Heta(double x, void* p);
double colliding_protons(gsl_spline* spline_Jp, gsl_interp_accel* acc_Jp, double gp_min,
                         double gp_max, double Ep);
double photons_jet(double eta, double Ep, gsl_spline* spline_ng, gsl_interp_accel* acc_ng,
                   double nu_min, double nu_max);
void tables_photomeson(double& s, double& delta, double& Beta, std::string_view product,
                       double xeta);
double PhiFunc(double eta, double eta0, double x, std::string_view product);

}    // namespace kariba
