@echo off

set BUILD=r
set OPT=-O2
set PROJECTNAME=heat

IF %BUILD% EQU d (
   set BUILDSWITCH=-MDd
   set SUF=-d
) ELSE (
  set BUILDSWITCH=-MD
  set SUF=
)


mkdir ..\..\build\%PROJECTNAME%
pushd ..\..\build\%PROJECTNAME%

cl %OPT% -FC -Zi -EHsc %BUILDSWITCH% ..\..\%PROJECTNAME%\src\main.cpp^
 ../../_tools/_personal/timer.cpp^
 -I ../../_tools/_personal^
 -I ../../_tools/SFML-2.3/include^
 -I ../../_tools/glew-1.13.0/include^
 -I ../../_tools/freeglut-2.8.1/include^
 -link -LIBPATH:../../_tools/SFML-2.3/lib^
 winmm.lib sfml-system%SUF%.lib opengl32.lib gdi32.lib sfml-window%SUF%.lib freetype.lib jpeg.lib sfml-graphics%SUF%.lib glew32.lib

popd