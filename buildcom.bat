@echo off
wcl -zq -3 -fp3 -fpi87 -mt -ox -zp1 -wcd=107 %1
if exist %1.obj del /Q %1.obj
if exist %1.err del /Q %1.err
