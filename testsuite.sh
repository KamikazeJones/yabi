#!/bin/bash

# test while loop with (C)ontinue on zero
~/tcc/tcc.exe -run yabi.c -i -x '_01s _aj [0000 J-j _4s J-D C0000 _41s J+. D J ]0000 _40 .H' -c JIHGFDCBA@

# Hello world - example for S- and R-Stack, and while loop
~/tcc/tcc.exe -run yabi.c -x '_48s_65s_6css_6fs_20s_17s_60+Ds_6fs_12s_60+Ds_6cs_64s _bj [0000 Sr _1s J-j D ]0000 _bj [0000 R. _1s J-j D ]0000 H' -c 'Hello world'

# Hello world - example for S- and R-Stack, and while loop, use patch
~/tcc/tcc.exe -run yabi.c -p -x '_48s_65s_6css_6fs_20s_17s_60+Ds_6fs_12s_60+Ds_6cs_64s _bj [0000 Sr _1s J-j D ]0000 _bj [0000 R. _1s J-j D ]0000 H' -c 'Hello world'

# Hello world - example patch jump addresses only - produce simple byte code
~/tcc/tcc.exe -run yabi.c -P -x '_48s_65s_6css_6fs_20s_17s_60+Ds_6fs_12s_60+Ds_6cs_64s _bj [0000 Sr _1s J-j D ]0000 _bj [0000 R. _1s J-j D ]0000 H' -c '_48s_65s_6css_6fs_20s_17s_60+Ds_6fs_12s_60+Ds_6cs_64s _bj [0018 Sr _1s J-j D ]000e _bj [0018 R. _1s J-j D ]000e H'

# test for storing and fetching
~/tcc/tcc.exe -run yabi.c -p -d -x 'Pm_20sM+P                           _0i_48!_4i_65!_0i@._04i@.H' -c 'He'
#
# yabi.c -x 'pss_19+DPHello world!     _7i_cj [0000 I J-j ]0000
# write number (hex)
#~/tcc/tcc.exe -run yabi.c -d -x '_1234 i Ps_c+rp  H  _1s _4j [0000  _000fs I A D s _30+ D r I >>>> i J-j ]0000 _4j [0000 R . J-j]0000 D R_4s-D p H' -c '1234'

# test for r,R,store and x_load
# ~/tcc/tcc.exe -run yabi.c -P -x ''
