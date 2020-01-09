abjad: abjad.cc
	g++ -m32 -std=gnu++11 abjad.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o abjad.exe
