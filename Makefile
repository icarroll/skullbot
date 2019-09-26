abjad: abjad.cc
	g++ -m32 -std=gnu++11 abjad.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o abjad.exe

key_page: key_page.cc
	g++ -m32 -std=gnu++11 key_page.cc -I/mingw32/include/cairo -L/mingw32/lib -Wl,-subsystem,windows -lmingw32 -lcairo -mwindows -o key_page.exe
