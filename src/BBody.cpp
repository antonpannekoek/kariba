#include <cmath>
#include <iostream>

#include "kariba/BBody.hpp"
#include "kariba/constants.hpp"

namespace kariba {

BBody::BBody(size_t size) : Radiation(size) {}    // default of size = 40 in definition
//! Methods to set BB quantities
void BBody::set_temp_k(double T) {
    double emin, emax, einc;

    Tbb = T;

    emin = 1e-3 * constants::kboltz * Tbb;
    emax = 30. * constants::kboltz * Tbb;

    einc = (std::log10(emax) - std::log10(emin)) / static_cast<double>(en_phot.size() - 1);

    for (size_t i = 0; i < en_phot.size(); i++) {
        en_phot[i] = std::pow(10., std::log10(emin) + static_cast<double>(i) * einc);
        en_phot_obs[i] = en_phot[i];
    }
}

void BBody::set_temp_kev(double T) {
    set_temp_k(T * constants::kboltz_kev2erg / constants::kboltz);
}

void BBody::set_temp_hz(double nu) {
    set_temp_k((constants::herg * nu) / (2.82 * constants::kboltz));
}


void BBody::set_lum(double L) {
    Lbb = L;
    normbb = Lbb / (constants::sbconst * std::pow(Tbb, 4.));
}

//! Method to set BB spectrum
void BBody::bb_spectrum() {
    for (size_t i = 0; i < num_phot.size(); i++) {
        
        if (en_phot_obs[i] / (Tbb * constants::kboltz) < 700) {
            num_phot[i] = normbb * 2. * constants::pi * constants::herg *
                      std::pow(en_phot_obs[i] / constants::herg, 3.) /
                      (std::pow(constants::cee, 2.) * 
                      (std::exp(en_phot_obs[i] / (Tbb * constants::kboltz)) - 1.));
        }
        else {
            num_phot[i] = 1e-150; // small, =0
        }

        num_phot_obs[i] = num_phot[i];
    }
}

//! Methods to return BB temperature, luminosity, energy density at a given
//! distance d (or for a given radius d of the source)
double BBody::temp_kev() const { return Tbb * constants::kboltz / constants::kboltz_kev2erg; }

double BBody::temp_k() const { return Tbb; }

double BBody::temp_hz() const { return 2.82 * constants::kboltz * Tbb / constants::herg; }

double BBody::lum() const { return Lbb; }

double BBody::norm() const { return normbb; }

double BBody::Urad(double d) const {
    return Lbb / (4. * constants::pi * std::pow(d, 2.) * constants::cee);
}

void BBody::test() {
    std::cout << "Black body temperature in K: " << temp_k() << " in keV: " << temp_kev()
              << std::endl;
    std::cout << "Black body bolometric luminosity in erg s^-1 " << Lbb << std::endl;
}

}    // namespace kariba
