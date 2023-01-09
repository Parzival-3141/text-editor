@pushd "%~dp0"


@if not "%1" == "release" (
	@set debug=--debug
)

@set sourcefiles=.\src\main.c .\src\sdl_utils.c .\src\editor.c

:: @set zigcache=--cache-dir .\zig-cache --global-cache-dir C:\Users\pocke\AppData\Local\zig --enable-cache

@set cflags=-lc -Wall -Wno-unused-parameter -Wextra -pedantic %debug%

@set deps=-lSDL2 -lSDL2_ttf -I deps\include\SDL2 -I deps\include\SDL2_ttf -L deps/lib/SDL2 -L deps/lib/SDL2_ttf

zig cc %cflags% -o .\build\wheel.exe %sourcefiles% %deps%

cp deps\lib\SDL2\SDL2.dll .\build\
cp deps\lib\SDL2_ttf\SDL2_ttf.dll .\build\
cp "deps\Hack Regular Nerd Font Complete.ttf" .\build\

@popd
