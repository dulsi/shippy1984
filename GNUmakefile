# Crappy makefile by druppy cuz he is lazy commit me fucker.
# v1.0

GAMERZILLA = 1
SDL2      = 1

CC       = gcc
RM       = /bin/rm
CFLAGS   = -O3
LDFLAGS  =
SRCS     = shippy.c
OBJS     = $(patsubst %.c, %.o, $(SRCS))
EXEC     = shippy

SRCS    += shipsdl2.c
CFLAGS  += -DUSE_SDL
LDFLAGS  = `pkg-config --libs sdl2 SDL2_mixer`
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
