@echo off
wcl386 -5 -d0 -fp5 -fpi87 -wx -zq -ecc -ot -ol -op -zp1 -bcl=dos32a -wcd=107 %1 %2
del %1.obj
del %1.err
