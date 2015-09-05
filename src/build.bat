@echo off

set PROJECTNAME=heat

mkdir ..\..\build\%PROJECTNAME%
pushd ..\..\build\%PROJECTNAME%
cl -O2 -FC -Zi -EHsc -MD ..\..\%PROJECTNAME%\src\main.cpp^
 -I ../../_tools/SFML-2.3/include^
 -I ../../_tools/glew-1.13.0/include^
 -I ../../_tools/freeglut-2.8.1/include^
 -link -LIBPATH:../../_tools/SFML-2.3/lib^
 winmm.lib sfml-system.lib opengl32.lib gdi32.lib sfml-window.lib freetype.lib jpeg.lib sfml-graphics.lib glew32.lib

popd