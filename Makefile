abjad: abjad.cc
	g++ -g -std=gnu++11 abjad.cc -I/mingw64/include/cairo -L/mingw64/lib -lcairo -o abjad.exe
#	g++ -m32 -std=gnu++11 abjad.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o abjad.exe

cover: cover.cc
	g++ -m32 -std=gnu++11 cover.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o cover.exe
