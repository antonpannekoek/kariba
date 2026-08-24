#include <cmath>
#include <iostream>

#include <gsl/gsl_integration.h>

#include "kariba/Cyclosyn.hpp"
#include "kariba/Radiation.hpp"
#include "kariba/constants.hpp"

namespace kariba {

//! Synchrotron tables for F(nu/nuc) for calculation of single particle spectrum
static double arg[47] = {0.0001, 0.0002, 0.0005, 0.001, 0.002, 0.005, 0.01, 0.03, 0.05, 0.07,
                         0.1,    0.2,    0.3,    0.4,   0.5,   0.6,   0.7,  0.8,  0.9,  1.,
                         1.5,    2.,     2.5,    3.,    3.5,   4.,    4.5,  5.,   5.5,  6.,
                         6.5,    7.,     7.5,    8.,    8.5,   9.,    9.5,  10.,  12.,  14.,
                         16.,    18.,    20.,    25.,   30.,   40.,   50.};

static double var[47] = {
    -1.002e+0, -9.031e-1, -7.696e-1, -6.716e-1, -5.686e-1, -4.461e-1, -3.516e-1, -2.125e-1,
    -1.537e-1, -1.192e-1, -8.725e-2, -4.383e-2, -3.716e-2, -4.528e-2, -5.948e-2, -7.988e-2,
    -1.035e-1, -1.296e-1, -1.586e-1, -1.838e-1, -3.507e-1, -5.214e-1, -6.990e-1, -8.861e-1,
    -1.073e+0, -1.267e+0, -1.470e+0, -1.670e+0, -1.870e+0, -2.073e+0, -2.279e+0, -2.483e+0,
    -2.686e+0, -2.893e+0, -3.097e+0, -3.303e+0, -3.510e+0, -3.717e+0, -4.550e+0, -5.388e+0,
    -6.230e+0, -7.075e+0, -7.921e+0, -1.005e+1, -1.218e+1, -1.646e+1, -2.076e+1};

Cyclosyn::~Cyclosyn() { gsl_spline_free(syn_f), gsl_interp_accel_free(syn_acc); }

//! This constructor initializes the arrays and interpolations. In this case,
//! calculations are done in frequency space, not in photon energies.
Cyclosyn::Cyclosyn(size_t size) : Radiation(size) {
    en_phot_obs.resize(en_phot_obs.size() * 2, 0.0);
    num_phot_obs.resize(num_phot_obs.size() * 2, 0.0);
    cyclosyn_absorption_rate.resize(size, 0.0);

    counterjet = false;

    syn_acc = gsl_interp_accel_alloc();
    syn_f = gsl_spline_alloc(gsl_interp_cspline, 47);

    gsl_spline_init(syn_f, arg, var, 47);
}

double cyclosyn_kernel(double gamma, double nu, double b, gsl_spline* syn, gsl_interp_accel* acc_syn) {
    double nu_c, x, emisfunc, nu_larmor, psquared;
    // this is in the synchrotron regime
    // use the approximation for the pitch angle averaged version from 
    // Aharonian, Kelner & Prosekin 2010, D7
    if (gamma > 2.) {
        nu_c = (3. * constants::charg * b * std::pow(gamma, 2.)) /
               (4. * constants::pi * constants::emgm * constants::cee);
        x = nu / nu_c;
        // This is F(x) , which is not pitch angle averaged
        // if (x <= 1.e-4) {
        //     emisfunc = 4. * constants::pi * std::cbrt(x / 2.) / (sqrt(3.) * 2.68);
        // } else if (x > 50.) {
        //     emisfunc = sqrt(constants::pi * x / 2.) * std::exp(-x);
        // } else {
        //     emisfunc = std::pow(10., gsl_spline_eval(syn, x, acc_syn));
        // }
        if (x <= 700) {
            double x13 = std::cbrt(x);   // more stable than pow(x, 1./3.)
            double x23 = x13 * x13;
            double x43 = x23 * x23;
            double t1 = 1.808 * x13 / std::sqrt(1 + 3.4 * x23);
            double t2 = 1 + 2.21 * x23  + 0.347 * x43;
            double t3 = 1 + 1.353 * x23  + 0.217 * x43;
            emisfunc = t1 * t2 / t3 * std::exp(-x);
        }
        else {
            emisfunc = 1e-305; // floor, = 0
        }
    } else {    // cyclotron regime
        nu_larmor =
            (constants::charg * b) / (2. * constants::pi * constants::emgm * constants::cee);
        x = nu / nu_larmor;
        psquared = std::pow(gamma, 2.) - 1.;
        emisfunc = (2. * psquared) / (1. + 3. * psquared) *
                   std::exp((2. * (1. - x)) / (1. + 3. * psquared));
    }
    return emisfunc;
}

//! Single particle emissivity/absorption coefficient calculations
double cyclosyn_emis(double log_rho, void* pars) {
    CyclosynEmisParams* params = static_cast<CyclosynEmisParams*>(pars);
    double nu = params->nu;
    double b = params->b;
    gsl_spline* syn = params->syn;
    gsl_interp_accel* acc_syn = params->acc_syn;
    gsl_spline* eldis = params->eldis;
    gsl_interp_accel* acc_eldis = params->acc_eldis;

    double gamma, emisfunc, ngamma, nlogp, norm_em;
    gamma = std::sqrt(std::exp(2*log_rho) + 1);
    emisfunc = cyclosyn_kernel(gamma, nu, b, syn, acc_syn);
    ngamma = gsl_spline_eval(eldis, gamma, acc_eldis);
    nlogp = (gamma*gamma - 1)/gamma * ngamma; // dn/dlogp = p dn/dp = p dgamma/dp dn/dgamma
    norm_em = sqrt(3.) * std::pow(constants::charg, 3) * b / constants::emerg;
    return nlogp * emisfunc * norm_em;
}

double cyclosyn_abs(double log_rho, void* pars) {
    CyclosynAbsParams* params = static_cast<CyclosynAbsParams*>(pars);
    double nu = (params->nu);
    double b = (params->b);
    gsl_spline* syn = (params->syn);
    gsl_interp_accel* acc_syn = (params->acc_syn);
    gsl_spline* derivs = (params->derivs);
    gsl_interp_accel* acc_derivs = (params->acc_derivs);

    // using eg. Ghisellini & Svensson 1991, eq 1
    double gamma, rho, emisfunc, pdensp2_diff_logp, nlogp, norm_em, norm_ab, fac_p;
    gamma = std::sqrt(std::exp(2*log_rho) + 1);
    rho = std::exp(log_rho);
    emisfunc = cyclosyn_kernel(gamma, nu, b, syn, acc_syn);
    pdensp2_diff_logp = gsl_spline_eval(derivs, gamma, acc_derivs);
    norm_em = sqrt(3.) * std::pow(constants::charg, 3) * b / constants::emerg;
    norm_ab = - std::pow(nu, -2.) / (8. * constants::pi * constants::emgm);
    fac_p = std::pow(constants::emgm * constants::cee, 3); // from p to rho
    return norm_ab * gamma * rho * norm_em * emisfunc * pdensp2_diff_logp * fac_p;
}

//! Integrals of single particle emissivity/absorption coefficient over particle
//! distribution
double Cyclosyn::emis_integral(double nu, double gmin, double gmax, gsl_spline* eldis,
                               gsl_interp_accel* acc_eldis) {
    double result1, error1;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto F1params = CyclosynEmisParams{nu, bfield, syn_f, syn_acc, eldis, acc_eldis};
    F1.function = &cyclosyn_emis;
    F1.params = &F1params;
    double rho_min = std::sqrt(gmin*gmin - 1);
    double rho_max = std::sqrt(gmax*gmax - 1);
    gsl_integration_qag(&F1, std::log(rho_min), std::log(rho_max), 0, 1e-1, 100, 2, w1, &result1,
                        &error1);
    gsl_integration_workspace_free(w1);

    return result1;
}

double Cyclosyn::abs_integral(double nu, double gmin, double gmax, gsl_spline* derivs,
                              gsl_interp_accel* acc_derivs) {
    double result1, error1;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto F1params = CyclosynAbsParams{nu, bfield, syn_f, syn_acc, derivs, acc_derivs};
    F1.function = &cyclosyn_abs;
    F1.params = &F1params;
    double rho_min = std::sqrt(gmin*gmin - 1);
    double rho_max = std::sqrt(gmax*gmax - 1);
    gsl_integration_qag(&F1, std::log(rho_min), std::log(rho_max), 1e1, 1e1, 100, 2, w1, &result1,
                        &error1);
    gsl_integration_workspace_free(w1);

    return result1;
}

//! Comoving and observed specific luminosity for the input particle distribution
void Cyclosyn::cycsyn_spectrum(double gmin, double gmax, gsl_spline* eldis,
                               gsl_interp_accel* acc_eldis, gsl_spline* eldis_diff,
                               gsl_interp_accel* acc_eldis_diff) {
    double j_emis, alpha_abs;
    // double pitch = 0.73;
    // double acons, elcons, asyn, epsasyn;
    double absfac, tau_syn, tau_syn_obs, absfac_obs;
    double dopfac_cj;

    dopfac_cj = dopfac * (1. - beta * cos(angle)) / (1. + beta * cos(angle));

    size_t size = en_phot.size();
    for (size_t k = 0; k < size; k++) {
        en_phot_obs[k] = en_phot[k] * dopfac;
        if (counterjet == true) {
            en_phot_obs[k + size] = en_phot[k] * dopfac_cj;
        }
        j_emis = emis_integral(en_phot[k] / constants::herg, gmin, gmax, eldis, acc_eldis);
        alpha_abs = abs_integral(en_phot[k] / constants::herg, gmin, gmax, eldis_diff, acc_eldis_diff);

        if (alpha_abs < 1e-150) {
            alpha_abs = 1e-150;
        }
        cyclosyn_absorption_rate[k] = alpha_abs * constants::cee;
        if (std::log10(j_emis) < -150.) {
            num_phot_obs[k] = 0;
            if (counterjet == true) {
                num_phot_obs[k + size] = 0;
            }
        } else {
            // elcons = sqrt(3.) * (constants::charg * constants::charg * constants::charg) * bfield *
            //          sin(pitch) / constants::emerg;
            // acons = -constants::cee * constants::cee /
            //         (8. * constants::pi * std::pow(en_phot[k] / constants::herg, 2.));
            // asyn = acons * elcons * abs;
            // cyclosyn_absorption_rate[k] = asyn * constants::cee * constants::pi;
            
            // epsasyn = emis / (acons * abs);
            
            // // This includes skin depth/viewing angle effects for cylinder case

            // if (geometry == "cylinder") {
            //     tsyn_obs = constants::pi / 2. * asyn * r / (dopfac * sin(angle));
            // } else {
            //     tsyn_obs = constants::pi / 3. * asyn * r;
            // }
            double l_average = r;
            // average path lengths
            if (geometry == "cylinder") {
                l_average *= constants::pi / 2.;
            } else {
                l_average *= constants::pi / 3.;
            }
            double t_esc = l_average / constants::cee; 
            tau_syn = l_average * alpha_abs;
            // numerically stable -( exp(-tsyn) - 1 )
            absfac = - std::expm1(-tau_syn);
            // observed opacities changed by doppler factor and viewing angle
            tau_syn_obs = tau_syn / dopfac;
            if (geometry == "cylinder") {
                tau_syn_obs *= 1/sin(angle); // this is spooky for theta = 0 ? 
            }
            absfac_obs = - std::expm1(-tau_syn_obs) / alpha_abs;
            double cross_section_circle = constants::pi * r * r;
            num_phot[k] =  cross_section_circle * absfac * j_emis/alpha_abs;
            double projected_area = 2. * r * z; // diameter * height -> lacks projection for headon?
            num_phot_obs[k] = projected_area * absfac_obs * j_emis * std::pow(dopfac, dopnum);
            // num_phot_obs[k] = projected_area * r* t_esc * j_emis * std::pow(dopfac, dopnum);

            if (counterjet == true) {
                tau_syn_obs *= dopfac/dopfac_cj;
                absfac_obs = - std::expm1(-tau_syn_obs) / alpha_abs;
                num_phot_obs[k + size] = projected_area * absfac_obs * j_emis * std::pow(dopfac_cj, dopnum);
            } else {
                num_phot_obs[k + size] = 0.;
            }
        }
    }
}

//! Methods to return the cyclosyn scale frequencies for a given Lorentz factor.
//! Returns synchrotron or larmor frequencies depending on whether input gamma is
//! greater or smaller than 2 The second method does the same thing, but it
//! checks the computed arrays and looking for the maximum value of L_nu. Unlike
//! the simple way, this accounts for the fact that the peak may be caused by
//! synchrotron self absorption rather than coinciding with the scale frequency
double Cyclosyn::nu_syn(double gamma) {
    return (3. * constants::charg * bfield * std::pow(gamma, 2.)) /
           (4. * constants::pi * constants::emgm * constants::cee);
}

double Cyclosyn::nu_syn() {
    double temp_lum = 0.;
    size_t temp = 0;
    for (size_t i = 0; i < num_phot.size(); i++) {
        if (num_phot[i] > temp_lum) {
            temp_lum = num_phot[i];
            temp = i;
        }
    }
    return en_phot[temp] / constants::herg;
}

//! Method to set up the frequency array over desired range
void Cyclosyn::set_frequency(double numin, double numax) {
    double nuinc =
        (std::log10(numax) - std::log10(numin)) / static_cast<double>(en_phot.size() - 1);

    for (size_t i = 0; i < en_phot.size(); i++) {
        en_phot[i] =
            std::pow(10., std::log10(numin) + static_cast<double>(i) * nuinc) * constants::herg;
    }
}

//! Method to set magnetic field
void Cyclosyn::set_bfield(double b) { bfield = b; }

//! Method to set the particle mass
void Cyclosyn::set_mass(double mass) { mass_gr = mass; }

void Cyclosyn::test() {
    std::cout << "Bfield: " << bfield << " r: " << r << " z: " << z << " v.angle: " << angle
              << " speed: " << beta << " delta: " << dopfac << " particle mass: " << mass_gr
              << "\n";
}

}    // namespace kariba
