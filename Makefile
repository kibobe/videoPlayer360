
#
# http://www.gnu.org/software/make/manual/make.html
#
CC:=g++
INCLUDES:=$(shell pkg-config --cflags sdl SDL_image libavformat libavcodec libswscale libavutil opencv)
CFLAGS:=-Wall -g -D__STDC_CONSTANT_MACROS
LDFLAGS:=$(shell pkg-config --libs sdl SDL_image libavformat libavcodec libswscale libavutil opencv) -lm

videoPlayer.exe: main.o button.o player.o
	$(CC) -o videoPlayer main.o button.o player.o $(LDFLAGS)

main.o: main.cpp button.hpp	
	$(CC) -c $(CFLAGS) main.cpp $(INCLUDES)

player.o: player.cpp player.hpp	
	$(CC) -c $(CFLAGS) player.cpp $(INCLUDES)

button.o: button.cpp button.hpp	
	$(CC) -c $(CFLAGS) button.cpp $(INCLUDES)
	
clean:
	rm -f *.o