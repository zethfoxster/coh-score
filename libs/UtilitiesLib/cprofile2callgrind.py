#!/usr/bin/env python

# Cython currently fails on this module
import csv

import pyximport
pyximport.install(pyimport=True)
import profile2callgrind
profile2callgrind.main()

