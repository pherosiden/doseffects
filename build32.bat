@echo off
wcl386 -5 -d0 -fp5 -fpi87 -wx -zq -ecc -ot -ol -op -zp1 -bcl=dos32a -wcd=107 %1 %2
if exist %1.obj del /Q %1.obj
if exist %1.err del /Q %1.err
