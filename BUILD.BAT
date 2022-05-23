@echo off
wcl -zq -5 -fp5 -fpi87 -mc -ecc -od -zp1 -wcd=107 %1 %2
del %1.obj
del %1.err
