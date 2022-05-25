@echo off
wcl -zq -5 -fp5 -fpi87 -mc -ecc -od -zp1 -wcd=107 %1 %2
del /Q %1.obj
del /Q %1.err
