#! /usr/bin/env python3
from subprocess import call

r = call(["python3", "run-cmaes.py"])
if r!=0:
  exit(r)
  
r = call(["python3", "run-cmaes-gradient.py"])
if r!=0:
  exit(r)

r = call(["python3", "run-dea.py"])
if r!=0:
  exit(r)

exit(0)
