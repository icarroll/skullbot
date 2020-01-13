abjad: abjad.cc
	g++ -m32 -std=gnu++11 abjad.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o abjad.exe

goblin: goblin.cc
	g++ -m32 -std=gnu++11 goblin.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o goblin.exe

cover: cover.cc
	g++ -m32 -std=gnu++11 cover.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o cover.exe
