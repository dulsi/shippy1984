# Crappy makefile by druppy cuz he is lazy commit me fucker.
# v1.0

ALLEGRO  = 0
ifeq ($(ALLEGRO),1)
SDL      = 0
else
SDL      = 1
endif

CC       = gcc
RM       = /bin/rm
CFLAGS   = -O3
LDFLAGS  =
SRCS     = shippy.c
OBJS     = $(patsubst %.c, %.o, $(SRCS))
EXEC     = shippy

ifeq ($(SDL),1)
SRCS    += shipsdl.c
CFLAGS  += -DUSE_SDL
LDFLAGS  = `sdl-config --libs` -lSDL_mixer
else
SRCS    += shipall.c
CFLAGS  += -DUSE_ALLEGRO
LDFLAGS  = -laldmb -ldumb `allegro-config --libs`
endif

all: $(OBJS) $(EXEC)

%.o: %.c %.h
	$(CC) $(CFLAGS) -c $<

$(EXEC): $(OBJS)
	$(CC) -o $(EXEC) $(OBJS) $(LDFLAGS)

clean:
	$(RM) -f *.o $(EXEC)
