@echo off
call "C:\Program Files\Microsoft Visual Studio\18\Insiders\VC\Auxiliary\Build\vcvars32.bat" >nul
cl /nologo /c /std:c++20 /EHsc /MDd /Od /ZI /utf-8 /IGameTechlabWeek2 /Tptmp/build-timing/raw-probe.txt /Fotmp/build-timing/raw.obj /Fdtmp/build-timing/raw.pdb
