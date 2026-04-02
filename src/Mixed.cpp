#include <cmath>
#include <iostream>

#include <gsl/gsl_integration.h>
#include <gsl/gsl_math.h>
#include <gsl/gsl_sf_bessel.h>

#include "kariba/Mixed.hpp"
#include "kariba/Particles.hpp"
#include "kariba/constants.hpp"

namespace kariba {

//! Class constructors to initialize object
Mixed::Mixed(size_t size) : Particles(size) {
    thnorm = 1.;
    plnorm = 1.;

    mass_gr = constants::emgm;
    mass_kev = constants::emgm * constants::gr_to_kev;
}

//! Methods to set momentum/energy arrays and number density arrays
void Mixed::set_p(double ucom, double bfield, double betaeff, double r, double fsc) {
    pmin_pl = av_th_p();
    pmax_pl = std::max(max_p(ucom, bfield, betaeff, r, fsc), pmax_th);

    double pinc = (std::log10(pmax_pl) - std::log10(pmin_th)) / static_cast<double>(p.size() - 1);

    for (size_t i = 0; i < p.size(); i++) {
        p[i] = std::pow(10., std::log10(pmin_th) + static_cast<double>(i) * pinc);
        gamma[i] = std::pow(std::pow(p[i] / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);
    }
}

//! Same as above, but assuming a fixed maximum Lorentz factor
void Mixed::set_p(double gmax) {
    pmin_pl = av_th_p();
    pmax_pl = std::pow(std::pow(gmax, 2.) - 1., 1. / 2.) * mass_gr * constants::cee;

    double pinc = (std::log10(pmax_pl) - std::log10(pmin_th)) / static_cast<double>(p.size() - 1);

    for (size_t i = 0; i < p.size(); i++) {
        p[i] = std::pow(10., std::log10(pmin_th) + static_cast<double>(i) * pinc);
        gamma[i] = std::pow(std::pow(p[i] / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);
    }
}

void Mixed::set_ndens() {
    for (size_t i = 0; i < p.size(); i++) {
        if (p[i] <= pmin_pl) {
            ndens[i] = thnorm * std::pow(p[i], 2.) * std::exp(-gamma[i] / theta);
        } else if (p[i] < pmax_th) {
            ndens[i] = thnorm * std::pow(p[i], 2.) * std::exp(-gamma[i] / theta) +
                       plnorm * std::pow(p[i], -pspec) * std::exp(-p[i] / pmax_pl);
        } else {
            ndens[i] = plnorm * std::pow(p[i], -pspec) * std::exp(-p[i] / pmax_pl);
        }
    }
    initialize_gdens();
    gdens_differentiate();
}

//! methods to set the temperature, pl fraction, and normalizations. Temperature
//! must be in ergs, no factor kb
void Mixed::set_temp_kev(double T) {
    Temp = T;
    theta = T * constants::kboltz_kev2erg / (mass_gr * constants::cee * constants::cee);
    double emin_th = (1. / 100.) * T;
    double emax_th = 20. * T;
    double gmin_th, gmax_th;

    gmin_th = emin_th / mass_kev + 1.;
    gmax_th = emax_th / mass_kev + 1.;
    pmin_th = std::pow(std::pow(gmin_th, 2.) - 1., 1. / 2.) * mass_gr * constants::cee;
    pmax_th = std::pow(std::pow(gmax_th, 2.) - 1., 1. / 2.) * mass_gr * constants::cee;
}

void Mixed::set_pspec(double s1) { pspec = s1; }

void Mixed::set_plfrac(double f) { plfrac = f; }

void Mixed::set_plfrac(double Le, double r, double eldens) {
    double gpmax =
        sqrt(pmax_pl * pmax_pl / (mass_gr * constants::cee * mass_gr * constants::cee) + 1.);
    double sum = 0;
    double dx = std::log10(gamma[2] / gamma[1]);
    for (size_t i = 0; i < p.size(); i++) {
        sum += std::log(10.) * std::pow(gamma[i], -pspec + 2.) * std::exp(-gamma[i] / gpmax) * dx;
    }
    double Ue = Le / (constants::pi * r * r * constants::cee);
    double K = std::max(Ue / (sum * mass_gr * constants::cee * constants::cee), 0.);

    sum = 0.;
    for (size_t i = 0; i < gamma.size(); i++) {
        sum += std::log(10.) * std::pow(gamma[i], -pspec + 1.) * std::exp(-gamma[i] / gpmax) * dx;
    }
    double n_nth = K * sum;
    plfrac = n_nth / eldens;
}

void Mixed::set_norm(double n) {
    thnorm = (1. - plfrac) * n / (std::pow(mass_gr * constants::cee, 3.) * theta * K2(1. / theta));
    plnorm = plfrac * n * (1. - pspec) /
             (std::pow(pmax_pl, (1. - pspec)) - std::pow(pmin_pl, (1. - pspec)));
}

//! Method to solve steady state continuity equation. NOTE: KN cross section not
//! included in IC cooling
void Mixed::cooling_steadystate(double ucom, double n0, double bfield, double r, double betaeff) {
    double Urad = std::pow(bfield, 2.) / (8. * constants::pi) + ucom;
    double pdot_ad = betaeff * constants::cee / r;
    double pdot_rad = (4. * constants::sigtom * constants::cee * Urad) /
                      (3. * mass_gr * std::pow(constants::cee, 2.));
    double tinj = r / constants::cee;

    double gam_min, gam_max;
    gam_min = std::pow(std::pow(pmin_pl / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);
    gam_max = std::pow(std::pow(pmax_th / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);

    double integral, error;
    gsl_function F1;
    auto params =
        InjectionMixedParams{pspec, theta, thnorm, plnorm, mass_gr, gam_min, gam_max, pmax_pl};
    F1.function = &injection_mixed_int;
    F1.params = &params;

    for (size_t i = 0; i < ndens.size(); i++) {
        if (i < ndens.size() - 1) {
            gsl_integration_workspace* w1;
            w1 = gsl_integration_workspace_alloc(100);
            gsl_integration_qag(&F1, gamma[i], gamma[i + 1], 1e1, 1e1, 100, 1, w1, &integral,
                                &error);
            gsl_integration_workspace_free(w1);

            ndens[i] =
                (integral / tinj) / (pdot_ad * p[i] / (mass_gr * constants::cee) +
                                     pdot_rad * (gamma[i] * p[i] / (mass_gr * constants::cee)));
        } else {
            ndens[ndens.size() - 1] =
                ndens[ndens.size() - 2] * std::pow(p[p.size() - 1] / p[p.size() - 2], -pspec - 1);
        }
    }
    // the last bin is set by arbitrarily assuming cooled distribution; this is
    // necessary because the integral
    // above is undefined for the last bin

    // The last step requires a renormalization. The reason is that the result
    // of gsl_integration_qag strongly depends on the value of "size". Without
    // doing anything fancy, this can be fixed simply by ensuring that the total
    // integrated number of density equals n0 (which we know), and rescaling the
    // array ndens[i] by the appropriate constant.
    double renorm = count_particles() / n0;

    for (size_t i = 0; i < ndens.size(); i++) {
        ndens[i] = ndens[i] / renorm;
    }
    initialize_gdens();
    gdens_differentiate();
}

//! Method to calculate maximum momentum of non thermal particles based on
//! acceleration and cooling timescales
double Mixed::max_p(double ucom, double bfield, double betaeff, double r, double fsc) {
    double Urad, escom, accon, syncon, b, c, gmax;
    Urad = std::pow(bfield, 2.) / (8. * constants::pi) + ucom;
    escom = betaeff * constants::cee / r;
    syncon = (4. * constants::sigtom * Urad) / (3. * mass_gr * constants::cee);
    accon = (3. * fsc * constants::charg * bfield) / (4. * mass_gr * constants::cee);

    b = escom / syncon;
    c = accon / syncon;

    gmax = (-b + std::pow(std::pow(b, 2.) + 4. * c, 1. / 2.)) / 2.;

    return std::pow(std::pow(gmax, 2.) - 1., 1. / 2.) * mass_gr * constants::cee;
}

//! Evaluate Bessel function for MJ normalization
double Mixed::K2(double x) {
    double res;

    if (x < 0.1) {
        res = 2. / x / x;
    } else {
        res = gsl_sf_bessel_Kn(2, x);
    }

    return res;
}

//! Methods to calculate number density and average energy in thermal part
double th_num_dens_int(double x, void* pars) {
    ThParams* params = static_cast<ThParams*>(pars);
    double t = params->t;
    double n = params->n;
    double m = params->m;

    double gam_int = std::pow(std::pow(x / (m * constants::cee), 2.) + 1., 1. / 2.);

    return n * std::pow(x, 2.) * std::exp(-gam_int / t);
}

double av_th_p_int(double x, void* pars) {
    ThParams* params = static_cast<ThParams*>(pars);
    double t = params->t;
    double n = params->n;
    double m = params->m;

    double gam_int = std::pow(std::pow(x / (m * constants::cee), 2.) + 1., 1. / 2.);

    return n * std::pow(x, 3.) * std::exp(-gam_int / t);
}

double Mixed::count_th_particles() {
    double integral1, error1;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto params = ThParams{theta, thnorm, mass_gr};
    F1.function = &th_num_dens_int;
    F1.params = &params;
    gsl_integration_qag(&F1, pmin_th, pmax_th, 0, 1e-7, 100, 1, w1, &integral1, &error1);
    gsl_integration_workspace_free(w1);

    return integral1;
}

double Mixed::av_th_p() {
    double integral1, error1, integral2;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto params = ThParams{theta, thnorm, mass_gr};
    F1.function = av_th_p_int;
    F1.params = &params;
    gsl_integration_qag(&F1, pmin_th, pmax_th, 0, 1e-7, 100, 1, w1, &integral1, &error1);
    gsl_integration_workspace_free(w1);
    integral2 = count_th_particles();

    return integral1 / integral2;
}

double Mixed::av_th_gamma() {
    double avp;
    avp = av_th_p();

    return std::pow(std::pow(avp / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);
}

//! Methods to calculate number density and average energy in non-thermal part
double pl_num_dens_int(double x, void* pars) {
    PlParams* params = static_cast<PlParams*>(pars);
    double s = params->s;
    double n = params->n;

    return n * std::pow(x, -s);
}

double av_pl_p_int(double x, void* pars) {
    PlParams* params = static_cast<PlParams*>(pars);
    double s = params->s;
    double n = params->n;

    return n * std::pow(x, -s + 1.);
}

double Mixed::count_pl_particles() {
    double integral1, error1;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto params = PlParams{pspec, plnorm};
    F1.function = &pl_num_dens_int;
    F1.params = &params;
    gsl_integration_qag(&F1, pmin_pl, pmax_pl, 0, 1e-7, 100, 1, w1, &integral1, &error1);
    gsl_integration_workspace_free(w1);

    return integral1;
}

double Mixed::av_pl_p() {
    double integral1, error1, integral2;
    gsl_integration_workspace* w1;
    w1 = gsl_integration_workspace_alloc(100);
    gsl_function F1;
    auto params = PlParams{pspec, plnorm};
    F1.function = &av_pl_p_int;
    F1.params = &params;
    gsl_integration_qag(&F1, pmin_pl, pmax_pl, 0, 1e-7, 100, 1, w1, &integral1, &error1);
    gsl_integration_workspace_free(w1);
    integral2 = count_pl_particles();

    return integral1 / integral2;
}

double Mixed::av_pl_gamma() {
    double avp;
    avp = av_pl_p();

    return std::pow(std::pow(avp / (mass_gr * constants::cee), 2.) + 1., 1. / 2.);
}

//! simple method to check quantities.
void Mixed::test() {
    std::cout << "Mixed distribution;" << std::endl;
    std::cout << "Temperature in keV: " << Temp << std::endl;
    std::cout << "Number density: " << count_particles() << std::endl;
    std::cout << "Thermal monetum limits: " << pmin_th << " " << pmax_th << std::endl;
    std::cout << "Non-thermal momentum limits: " << pmin_pl << " " << pmax_pl << std::endl;
}

//! Injection function to be integrated in cooling
double injection_mixed_int(double x, void* pars) {
    InjectionMixedParams* params = static_cast<InjectionMixedParams*>(pars);
    double s = params->s;
    double t = params->t;
    double nth = params->nth;
    double npl = params->npl;
    double m = params->m;
    double min = params->min;
    double max = params->max;
    double cutoff = params->cutoff;

    double mom_int = std::pow(std::pow(x, 2.) - 1., 1. / 2.) * m * constants::cee;

    if (x <= min) {
        return nth * std::pow(mom_int, 2.) * std::exp(-x / t);
    } else if (x < max) {
        return nth * std::pow(mom_int, 2.) * std::exp(-x / t) +
               npl * std::pow(mom_int, -s) * std::exp(-mom_int / cutoff);
    } else {
        return npl * std::pow(mom_int, -s) * std::exp(-mom_int / cutoff);
    }
}

}    // namespace kariba
