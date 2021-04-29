#!/usr/bin/env python


# This is the negative square -0.5*(x^2)
# Proportional to log Normal with 0 mean and 1 var
def model(s):
  v = s["Parameters"][0]
  r = -0.5 * v * v
  s["F(x)"] = r
  s["logP(x)"] = r
  s["logLikelihood"] = r
  s["grad(logP(x))"] = [-v]
  s["H(logP(x))"] = [[-1.0]]
