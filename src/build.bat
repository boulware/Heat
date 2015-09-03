@echo off

mkdir ..\..\build\heat
pushd ..\..\build\heat
cl -Od -FC -Zi -EHsc -MDd ..\..\heat\src\main.cpp -I ../../_tools/SFML-2.3/include -I ../../_tools/xb^
 -link^
 -LIBPATH:../../_tools/SFML-2.3/lib winmm.lib sfml-system-d.lib opengl32.lib gdi32.lib sfml-window-d.lib freetype.lib jpeg.lib sfml-graphics-d.lib

popd