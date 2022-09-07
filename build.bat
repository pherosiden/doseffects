@echo off
wcl -zq -5 -fp5 -fpi87 -mc -ecc -od -zp1 -wcd=107 %1 %2
if exist %1.obj del /Q %1.obj
if exist %1.err del /Q %1.err
