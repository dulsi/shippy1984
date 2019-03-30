#include <string.h>
#include <stdlib.h>
#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "shippy.h"

SDL_Surface *screen = NULL;
SDL_Surface *BackBuffer = NULL;
SDL_Surface *Graphics = NULL;
SDL_Joystick *Joystick = NULL;
Uint8 key[1337];

Uint32 CLEARCOLOR = 0;
SDL_Rect src;
SDL_Rect dest;

int jdirx = 0;
int jdiry = 0;
int jaction = 0;
int jsecond = 0;
int waitforkey = 360;

volatile int objectsynch = 0;
Uint32 timing;

#define MAX_SAMPLES 8
SDL_Surface *CreateSurfaceFromBitmap(char *bmpfile, Uint32 flags)
{

	SDL_Surface *junktemp;
	SDL_Surface *junktemp2;

	junktemp = SDL_LoadBMP(bmpfile);

	junktemp2 = SDL_ConvertSurface(junktemp, junktemp->format, flags);
	SDL_FreeSurface(junktemp);

	junktemp = SDL_DisplayFormat(junktemp2);
	SDL_FreeSurface(junktemp2);

	return junktemp;
}

int audio_op = 0;
Mix_Music *music = NULL;

struct SAMPLEHOLDER
{
	int loaded;
	char samplename[512];
	Mix_Chunk *sample;
	int voice;
	int counter;
	int istaken;
};

struct SAMPLEHOLDER samples[MAX_SAMPLES];
int audio_op;
void Start_Audio()
{
	int i;
	audio_op = 0;
	for (i = 0; i < MAX_SAMPLES; ++i)
	{
		samples[i].loaded = 0;
		samples[i].samplename[0] = 0;
		samples[i].sample = NULL;
		samples[i].voice = -1;
		samples[i].counter = 0;
		samples[i].istaken = 0;
	}
	music = NULL;
}

void audio_start()
{
	if (Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT, 2, 2048) != 0)
		audio_op = 0;
	else
		audio_op = 1;
}

void audio_play(char *wav)
{
	//check to see if the sample is loaded
	int i;
	if (audio_op == 0)
		return;
	for (i = 0; i < MAX_SAMPLES; ++i)
	{
		if (samples[i].istaken == 0)
		{
			samples[i].sample = Mix_LoadWAV(wav);
			strcpy(samples[i].samplename, wav);
			samples[i].voice = Mix_PlayChannel(-1, samples[i].sample, 0);
			samples[i].loaded = 1;
			samples[i].istaken = 1;
			++samples[i].counter;
			return;
		}
	}
}

void audio_music(char *mfile)
{
	if (audio_op == 0)
		return;

	if (music != NULL)
	{
		Mix_FreeMusic(music);
	}
	music = Mix_LoadMUS(mfile);
	if (!music)
	{
		printf("Mix_LoadMUS(%s): %s\n", mfile, Mix_GetError());
		audio_op = 0;
	}
	else if (Mix_PlayMusic(music, -1) == -1)
	{
		printf("Mix_PlayMusic: %s\n", Mix_GetError());
	}
	// well, there's no music, but most games don't break without music...

}

void audio_exec()
{
	int i;
	if (audio_op == 0)
		return;
	for (i = 0; i < MAX_SAMPLES; ++i)
	{
		if (samples[i].istaken)
		{
			if (samples[i].loaded == 1)
			{
				if (Mix_Playing(samples[i].voice) == 0)
				{
					--samples[i].counter;
					if (samples[i].counter <= 0)
					{
						Mix_HaltChannel(samples[i].voice);
						samples[i].voice = -1;
						Mix_FreeChunk(samples[i].sample);
						samples[i].sample = NULL;
						samples[i].istaken = 0;
					}
					samples[i].loaded = 0;
				}
			}
		}
	}
}

void audio_end()
{
	int i;
	audio_op = 0;
	for (i = 0; i < MAX_SAMPLES; ++i)
	{
		if (samples[i].istaken == 1)
		{
			Mix_HaltChannel(samples[i].voice);
			samples[i].voice = -1;
			Mix_FreeChunk(samples[i].sample);
		}
	}
}

void End_Audio()
{
	audio_end();
	Mix_HaltMusic();
	Mix_CloseAudio();
}

int done = 0;
int gscale = 1;

void SYSTEM_CLEANBMP()
{
	if (BackBuffer != NULL)
		SDL_FreeSurface(BackBuffer);
	if (Graphics != NULL)
		SDL_FreeSurface(Graphics);
	BackBuffer = NULL;
	Graphics = NULL;
}

void SYSTEM_SETVID()
{
	Uint32 flags = SDL_SWSURFACE | SDL_HWPALETTE | SDL_FULLSCREEN;
	if (start_windowed)
		flags &= ~SDL_FULLSCREEN;
	SDL_ShowCursor(SDL_DISABLE);
	screen = SDL_SetVideoMode(screen_width, screen_height, 8, flags);
	if (screen == NULL)
	{
		return;
	}

	SYSTEM_CLEANBMP();

	Graphics = CreateSurfaceFromBitmap(DATADIR "graphics.bmp", SDL_SWSURFACE | SDL_SRCCOLORKEY);
	BackBuffer = CreateSurfaceFromBitmap(DATADIR "splash.bmp", SDL_SWSURFACE | SDL_SRCCOLORKEY);
	SDL_SetColors(screen, Graphics->format->palette->colors, 0, Graphics->format->palette->ncolors);
	SDL_SetColorKey(Graphics, SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));
	SDL_SetColorKey(BackBuffer, SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));
	SDL_SetClipRect(screen, NULL);
	SDL_FillRect(BackBuffer, NULL, CLEARCOLOR);
	CLEARCOLOR = SDL_MapRGB(Graphics->format, 0, 0, 0);
}

int SYSTEM_INIT()
{

	if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) == -1))
	{
		return 1;
	}
	timing = SDL_GetTicks();

	atexit(SDL_Quit);
	SYSTEM_SETVID();
	SDL_WM_SetCaption("Shippy1984 by Ryan Broomfield SDL VERSION", NULL);

	audio_start();

	Joystick = SDL_JoystickOpen(0);

	return 0;
}

int SYSTEM_CLEAN()
{
	SYSTEM_CLEANBMP();
	End_Audio();
	if (Joystick)
		SDL_JoystickClose(Joystick);
	return 0;
}

int SYSTEM_GETKEY(int scancode)
{
	return 0;

}

void SYSTEM_BG(char *bmp)
{
	if (BackBuffer != NULL)
		SDL_FreeSurface(BackBuffer);
	BackBuffer = CreateSurfaceFromBitmap(bmp, SDL_SWSURFACE | SDL_SRCCOLORKEY);
	if (BackBuffer == NULL)
		done = 1;
	else
		SDL_SetColorKey(BackBuffer, SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));

}

void SYSTEM_DRAW_BG()
{
}

/* NEW SYSTEM_FINISHRENDER() BY JONATHAN GILBERT 1-28-2004 */
void SYSTEM_FINISHRENDER()
{
	src.x = 0;
	src.y = 0;
	src.w = 240;
	src.h = 160;

	dest.x = 0;
	dest.y = 0;
	dest.w = screen_width;
	dest.h = screen_height;

	if (SDL_MUSTLOCK(BackBuffer))
		SDL_LockSurface(BackBuffer);
	if (SDL_MUSTLOCK(screen))
		SDL_LockSurface(screen);

	SDL_SoftStretch(BackBuffer, &src, screen, &dest);

	if (SDL_MUSTLOCK(BackBuffer))
		SDL_UnlockSurface(BackBuffer);
	if (SDL_MUSTLOCK(screen))
		SDL_UnlockSurface(screen);

	SDL_UpdateRect(screen, 0, 0, 0, 0);
}

int SYSTEM_CLEARSCREEN()
{
	if (SDL_FillRect(BackBuffer, NULL, CLEARCOLOR) == -1)
	{
		printf("CLS ERROR! \n");
		return 1;
	}
	return 0;
}

void SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy)
{
	src.x = sx;
	src.y = sy;
	src.w = szx;
	src.h = szy;
	dest.x = x;
	dest.y = y;
	dest.w = szx;
	dest.h = szy;
	if (SDL_BlitSurface(Graphics, &src, BackBuffer, &dest) == -1)
	{
		printf("SYSTEM_BLIT ERROR! \n");
	}

}

void SYSTEM_POLLINPUT()
{
	jaction = 0;
	jsecond = 0;
	jdirx = 0;
	jdiry = 0;

	if (key[SDLK_ESCAPE])
		done = 1;
	if (key[SDLK_RETURN] && key[SDLK_LALT])
		SDL_WM_ToggleFullScreen(screen);

	if (waitforkey > 0)
	{
		--waitforkey;
		return;
	}

	jdirx = 0;
	jdiry = 0;
	jaction = 0;
	jsecond = 0;

	if (Joystick)
	{
		SDL_JoystickUpdate();
		jaction = SDL_JoystickGetButton(Joystick, 0);
		jsecond = SDL_JoystickGetButton(Joystick, 1);
		jdirx = SDL_JoystickGetAxis(Joystick, 0) / (50 * 256);
		jdiry = SDL_JoystickGetAxis(Joystick, 1) / (65 * 256);
	}

	if (jdirx == 0)
		jdirx = (key[SDLK_RIGHT] - key[SDLK_LEFT]) * 2;
	if (jdiry == 0)
		jdiry = key[SDLK_DOWN] - key[SDLK_UP];
	if (jaction == 0)
		jaction = key[SDLK_LCTRL];
	if (jsecond == 0)
		jsecond = key[SDLK_BACKSPACE];

}

void SYSTEM_IDLE()
{
	Uint32 test = SDL_GetTicks();

	if (test < timing)
		SDL_Delay(1);

	while (test > timing)
	{
		timing += 14;
		++objectsynch;
	}

	SDL_Event event;
	/* Poll for events. SDL_PollEvent() returns 0 when there are no  */
	/* more events on the event queue, our while loop will exit when */
	/* that occurs.                                                  */
	while (SDL_PollEvent(&event))
	{
		/* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
		if (event.type == SDL_QUIT)
		{
			done = 1;
		}
		if (event.type == SDL_KEYDOWN)
		{
			key[event.key.keysym.sym] = 1;
		}
		else if (event.type == SDL_KEYUP)
		{
			key[event.key.keysym.sym] = 0;
		}
	}

}

int main(int argc, char *argv[])
{
	return SHIPPY_MAIN(argc, argv);
}
