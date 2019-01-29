#include "distribution.h"

Korali::Distribution::Distribution(size_t seed) : _seed(seed)
{
  // Initialize Random Generator _range
  const gsl_rng_type * T;
  gsl_rng * r;
//  gsl_rng_env_setup();
  T = gsl_rng_default;
  _range = gsl_rng_alloc (T);
  gsl_rng_set(_range, _seed);
}

double Korali::GaussianDistribution::getDensity(double x) { return gsl_ran_gaussian_pdf(x - _mean, _sigma); }
double Korali::GaussianDistribution::getDensityLog(double x) { return -0.5*log(2*M_PI) - log(_sigma) - 0.5*pow((x-_mean)/_sigma, 2);} // Optimize by pre-calculating constants and using multiplication instead of power.
double Korali::GaussianDistribution::getRandomNumber() { return _mean + gsl_ran_gaussian(_range, _sigma); }

double Korali::UniformDistribution::getDensity(double x) { return gsl_ran_flat_pdf(x, _min, _max);}
double Korali::UniformDistribution::getDensityLog(double x) { if (x >= _min && x <= _max) return -log(_max-_min); return -INFINITY; }
double Korali::UniformDistribution::getRandomNumber() { return gsl_ran_flat(_range, _min, _max); }

double Korali::GammaDistribution::getDensity(double x) { return gsl_ran_gamma_pdf( x, _shape, _rate ); }
double Korali::GammaDistribution::getDensityLog(double x) {	if(x < 0) return -INFINITY; return - gsl_sf_lngamma(_shape) - _shape*log(_rate) + (_shape-1)*log(x) - x/_rate; }
double Korali::GammaDistribution::getRandomNumber() { return gsl_ran_gamma(_range, _shape, _rate); }

double Korali::ExponentialDistribution::getDensity(double x) { return  gsl_ran_exponential_pdf(x, _mean); }
double Korali::ExponentialDistribution::getDensityLog(double x) { if (x < 0) return -INFINITY; 	return - log(_mean) - x/_mean;}
double Korali::ExponentialDistribution::getRandomNumber() { return gsl_ran_exponential(_range, _mean);}
