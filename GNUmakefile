# Crappy makefile by druppy cuz he is lazy commit me fucker.
# v1.0

ALLEGRO  = 0
SDL = 0
GAMERZILLA = 1
ifeq ($(ALLEGRO),1)
SDL      = 0
SDL2      = 0
else
ifeq ($(SDL),1)
SDL      = 1
SDL2      = 0
else
SDL      = 0
SDL2      = 1
endif
endif

CC       = gcc
RM       = /bin/rm
CFLAGS   = -O3
LDFLAGS  =
SRCS     = shippy.c
OBJS     = $(patsubst %.c, %.o, $(SRCS))
EXEC     = shippy

ifeq ($(SDL2),1)
SRCS    += shipsdl2.c
CFLAGS  += -DUSE_SDL
LDFLAGS  = `sdl2-config --libs` -lSDL2_mixer
else
ifeq ($(SDL),1)
SRCS    += shipsdl.c
CFLAGS  += -DUSE_SDL
LDFLAGS  = `sdl-config --libs` -lSDL_mixer
else
SRCS    += shipall.c
CFLAGS  += -DUSE_ALLEGRO
LDFLAGS  = -laldmb -ldumb `allegro-config --libs`
endif
endif
ifeq ($(GAMERZILLA), 1)
CFLAGS  += -DGAMERZILLA `pkg-config --cflags gamerzilla`
LDFLAGS  += `pkg-config --libs gamerzilla`
endif

all: $(OBJS) $(EXEC)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)

clean:
	$(RM) -f *.o $(EXEC)
