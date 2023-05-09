#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <SDL2/SDL_ttf.h>
#include "shippy.h"
#include "path.h"
#include "font.h"

#define SHIPPY_LEFT 0
#define SHIPPY_RIGHT 1
#define SHIPPY_DOWN 2
#define SHIPPY_UP 3
#define SHIPPY_LCTRL 4
#define SHIPPY_BACKSPACE 5
#define SHIPPY_ESCAPE 6
#define SHIPPY_RETURN 7
#define SHIPPY_LALT 8
#define SHIPPY_D 9
#define SHIPPY_G 10
#define SHIPPY_F 11
#define SHIPPY_R 12
#define SHIPPY_Q 13
#define SHIPPY_S 14
#define SHIPPY_1 15
#define SHIPPY_2 16
#define SHIPPY_SPACE 17
#define SHIPPY_NONE 18

SDL_Window *screen = NULL;
SDL_Renderer *renderer = NULL;
SDL_Texture *BackBuffer = NULL;
SDL_Texture *Graphics = NULL;
SDL_GameController *Joystick[2] = { NULL, NULL };
Uint8 key[1337];
static bool fullscreen = true;
static int modes[2][3] = { {240, 160, 1}, {480, 320, 2} };
static unsigned int frame = 0;

SDL_Rect src;
SDL_Rect dest;

int jdirx[2] = { 0, 0 };
int jdiry[2] = { 0, 0 };
int jaction[2] = { 0, 0 };
int jsecond[2] = { 0, 0 };
int waitforkey[2] = { 360, 360 };
int players[2] = { 0, 0 };
int mode = -1;

struct keyconfig {
	int down;
	int up;
	int right;
	int left;
	int action1;
	int action2;
	int cancel;
	int start;
};

struct keyconfig keyconfigs[2] =
{
	{
		.down = SHIPPY_DOWN,
		.up = SHIPPY_UP,
		.right = SHIPPY_RIGHT,
		.left = SHIPPY_LEFT,
		.action1 = SHIPPY_LCTRL,
		.action2 = SHIPPY_SPACE,
		.action2 = SHIPPY_SPACE,
		.cancel = SHIPPY_BACKSPACE,
		.start = SHIPPY_1
	},
	{
		.down = SHIPPY_F,
		.up = SHIPPY_R,
		.right = SHIPPY_G,
		.left = SHIPPY_D,
		.action1 = SHIPPY_Q,
		.action2 = SHIPPY_NONE,
		.cancel = SHIPPY_S,
		.start = SHIPPY_2
	}
};

volatile int objectsynch = 0;
Uint32 timing;

TTF_Font *font;

typedef struct fontcache_s {
	int w, h;
	SDL_Texture *tex;
	const char *msg;
	int used;
} fontcache;

#define MAX_FONTCACHE 20
fontcache textcache[MAX_FONTCACHE];

SDL_Texture *CreateSurfaceFromBitmap(char *bmpfile, Uint32 flags)
{
	SDL_Surface *junktemp;
	char fname[1024];
	strcpy(fname, get_data_path());
	strcat(fname, bmpfile);

	junktemp = SDL_LoadBMP(fname);

	SDL_SetColorKey(junktemp, SDL_TRUE, SDL_MapRGB(junktemp->format, 255, 0, 255));
	SDL_Texture *sdlTexture = SDL_CreateTextureFromSurface(renderer, junktemp);
	SDL_FreeSurface(junktemp);

	return sdlTexture;
}

int audio_op = 0;
Mix_Music *music = NULL;
Mix_Chunk *soundeffects[MAX_SOUNDS];
const char *soundfiles[MAX_SOUNDS] = {
	"die.wav",
	"fanfare.wav",
	"helix.wav",
	"hit.wav",
	"shot.wav",
	"splash.wav"
};

#define MAX_SAMPLES 8
struct SAMPLEHOLDER
{
	int loaded;
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
	for (int i = 0; i < MAX_SOUNDS; i++)
	{
		char fname[1024];
		strcpy(fname, get_data_path());
		strcat(fname, soundfiles[i]);
		soundeffects[i] = Mix_LoadWAV(fname);
	}
}

void audio_play(int sound)
{
	//check to see if the sample is loaded
	int i;
	if (audio_op == 0)
		return;
	for (i = 0; i < MAX_SAMPLES; ++i)
	{
		if (samples[i].istaken == 0)
		{
			samples[i].sample = soundeffects[sound];
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
		Mix_HaltMusic();
		Mix_FreeMusic(music);
		music = NULL;
	}
	if (mfile != NULL)
	{
		char fname[1024];
		strcpy(fname, get_data_path());
		strcat(fname, mfile);
		music = Mix_LoadMUS(fname);
		if (!music)
		{
			// Cross compiled SDL2_mixer doesn't support .xm
			strcpy(fname + strlen(fname) - 2, "ogg");
			music = Mix_LoadMUS(fname);
			if (!music)
			{
				printf("Mix_LoadMUS(%s): %s\n", mfile, Mix_GetError());
				audio_op = 0;
			}
		}
		if ((music) && (Mix_PlayMusic(music, -1) == -1))
		{
			printf("Mix_PlayMusic: %s\n", Mix_GetError());
		}
		// well, there's no music, but most games don't break without music...
	}
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
		}
	}
	for (i = 0; i < MAX_SOUNDS; ++i)
	{
		Mix_FreeChunk(soundeffects[i]);
	}
}

void End_Audio()
{
	audio_end();
	Mix_HaltMusic();
	Mix_CloseAudio();
}

int done = 0;

void SYSTEM_CLEANBMP()
{
	if (BackBuffer != NULL)
		SDL_DestroyTexture(BackBuffer);
	if (Graphics != NULL)
		SDL_DestroyTexture(Graphics);
	BackBuffer = NULL;
	Graphics = NULL;
}

void SYSTEM_SETVID()
{
	Uint32 flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	if (start_windowed)
	{
		flags &= ~SDL_WINDOW_FULLSCREEN_DESKTOP;
		fullscreen = false;
	}
	screen = SDL_CreateWindow("Shippy1984 by Ryan Broomfield SDL2 VERSION", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, screen_width, screen_height, flags);
	SDL_ShowCursor(SDL_DISABLE);
	renderer = SDL_CreateRenderer(screen, -1, 0);

	if (screen == NULL)
	{
		return;
	}
	SYSTEM_SETMODE(1);
}

void SYSTEM_SETMODE(int num)
{
	if (mode != num)
	{
		mode = num;
		SYSTEM_CLEANBMP();
		if (mode == 0)
		{
			Graphics = CreateSurfaceFromBitmap("graphics.bmp", 0);
			BackBuffer = CreateSurfaceFromBitmap("splash.bmp", 0);
		}
		else
		{
			Graphics = CreateSurfaceFromBitmap("graphics2.bmp", 0);
			BackBuffer = CreateSurfaceFromBitmap("splash2.bmp", 0);
		}
		SDL_RenderSetLogicalSize(renderer, modes[mode][0], modes[mode][1]);

		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
		SDL_RenderClear(renderer);
	}
}

int SYSTEM_INIT()
{
	int i;

	if ((SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER) == -1))
	{
		return 1;
	}
	timing = SDL_GetTicks();

	if (TTF_Init() == -1)
	{
		return 1;
	}

	atexit(SDL_Quit);
	SYSTEM_SETVID();

	audio_start();

	int joyIndex = 0;
	int maxJoysticks = SDL_NumJoysticks();
	for (i = 0; i < 2; i++)
	{
		for(; joyIndex < maxJoysticks; ++joyIndex)
		{
			if (SDL_IsGameController(joyIndex))
			{
				Joystick[i] = SDL_GameControllerOpen(joyIndex);
				++joyIndex;
				break;
			}
		}
	}

	font = GetDefaultFont(get_data_path());
	for (i = 0; i < MAX_FONTCACHE; i++)
	{
		textcache[i].used = 0;
	}

	return 0;
}

int SYSTEM_CLEAN()
{
	SYSTEM_CLEANBMP();
	End_Audio();
	for (int i = 0; i < 2; i++)
	{
		if (Joystick[i])
			SDL_GameControllerClose(Joystick[i]);
	}
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(screen);
	SDL_Quit();
	return 0;
}

int SYSTEM_GETKEY(int scancode)
{
	return 0;

}

void SYSTEM_BG()
{
/*    if(BackBuffer!=NULL) SDL_FreeSurface(BackBuffer);
    BackBuffer= CreateSurfaceFromBitmap(bmp,SDL_SWSURFACE|SDL_SRCCOLORKEY);
    if(BackBuffer==NULL) done=1;
    else SDL_SetColorKey(BackBuffer,SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));*/
}

void SYSTEM_DRAW_BG(char *bmp)
{
	SYSTEM_CLEARSCREEN();
	src.x = 0;
	src.y = 0;
	src.w = modes[mode][0];
	src.h = modes[mode][1];
	dest.x = 0;
	dest.y = 0;
	dest.w = modes[mode][0];
	dest.h = modes[mode][1];
	if (SDL_RenderCopy(renderer, BackBuffer, &src, &dest) != 0)
	{
		printf("SYSTEM_BLIT ERROR! \n");
	}
}

/* NEW SYSTEM_FINISHRENDER() BY JONATHAN GILBERT 1-28-2004 */
void SYSTEM_FINISHRENDER()
{
	for (int i = 0; i < MAX_FONTCACHE; i++)
	{
		if (textcache[i].used == 2)
			textcache[i].used = 1;
		else if (textcache[i].used == 1)
		{
			SDL_DestroyTexture(textcache[i].tex);
			textcache[i].used = 0;
		}
	}
	SDL_RenderPresent(renderer);
	frame = frame + 1;
}

int SYSTEM_CLEARSCREEN()
{
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	return 0;
}

void SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy)
{
	if (sy == 48)
	{
		if (sx == 48 || sx == 64)
		{
			int mult = ((frame % (6 * 12)) / 12);
			if (mult > 3)
				mult = 6 - mult;
			sx += 80 * mult;
		}
		else if (sx == 32)
		{
			int mult = ((frame % (4 * 12)) / 12);
			sx += 80 * mult;
		}
	}
	src.x = sx * modes[mode][2];
	src.y = sy * modes[mode][2];
	src.w = szx * modes[mode][2];
	src.h = szy * modes[mode][2];
	dest.x = x * modes[mode][2];
	dest.y = y * modes[mode][2];
	dest.w = szx * modes[mode][2];
	dest.h = szy * modes[mode][2];
	if (SDL_RenderCopy(renderer, Graphics, &src, &dest) != 0)
	{
		printf("SYSTEM_BLIT ERROR! \n");
	}

}

void SYSTEM_COMICBUBLE(int dir, int y, const char *msg)
{
	SDL_Rect dest, src;
	int available = -1;
	int use = -1;
	for (int i = 0; i < MAX_FONTCACHE; i++)
	{
		if (textcache[i].used == 0)
		{
			if (available == -1)
				available = i;
			continue;
		}
		if (textcache[i].msg == msg)
		{
			use = i;
			break;
		}
	}
	if (use == -1)
	{
		use = available;
		textcache[use].msg = msg;
		SDL_Color c;
		c.r = 0;
		c.g = 0;
		c.b = 0;
		c.a = 255;
		SDL_Surface *img = TTF_RenderUTF8_Solid(font, msg, c);
		textcache[use].tex = SDL_CreateTextureFromSurface(renderer, img);
		textcache[use].w = img->w;
		textcache[use].h = img->h;
		SDL_FreeSurface(img);
	}
	textcache[use].used = 2;
	SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	dest.w = textcache[use].w + (textcache[use].w % 2);
	if (dir == 1)
		dest.x = (58 + 116) * modes[mode][2] - dest.w;
	else
		dest.x = 58 * modes[mode][2];
	dest.y = y * modes[mode][2];
	dest.h = 20;
	SDL_RenderFillRect(renderer, &dest);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SYSTEM_BLIT(128, 64 + dir * 32, (dest.x / modes[mode][2]) - 8, y, 8, 16);
	SYSTEM_BLIT(128, 80 + dir * 32, (dest.x + dest.w - 1) / modes[mode][2], y, 8, 16);
	src.x = 0;
	src.y = 0;
	src.w = textcache[use].w;
	src.h = textcache[use].h;
	dest.y = (y + 1) * modes[mode][2];
	dest.w = textcache[use].w;
	dest.h = textcache[use].h;
	if (SDL_RenderCopy(renderer, textcache[use].tex, &src, &dest) != 0)
	{
		printf("SYSTEM_BLIT ERROR! \n");
	}
}

void UpdatePlayerInput(int player)
{
	jaction[player] = 0;
	jsecond[player] = 0;
	jdirx[player] = 0;
	jdiry[player] = 0;
	players[player] = 0;
	if (waitforkey[player] > 0)
	{
		--waitforkey[player];
	}
	else
	{
		if (Joystick[player])
		{
			SDL_JoystickUpdate();
			players[player] = SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_START);
			jaction[player] = SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_A);
			jsecond[player] = SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_B);
			jdirx[player] = SDL_GameControllerGetAxis(Joystick[player], SDL_CONTROLLER_AXIS_LEFTX) / (50 * 256);
			jdiry[player] = SDL_GameControllerGetAxis(Joystick[player], SDL_CONTROLLER_AXIS_LEFTY) / (65 * 256);
			if (jdirx[player] == 0)
				jdirx[player] = (SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_DPAD_RIGHT) - SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_DPAD_LEFT)) * 2;
			if (jdiry[player] == 0)
				jdiry[player] = (SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_DPAD_DOWN) - SDL_GameControllerGetButton(Joystick[player], SDL_CONTROLLER_BUTTON_DPAD_UP)) * 2;
		}

		if (jdirx[player] == 0)
			jdirx[player] = (key[keyconfigs[player].right] - key[keyconfigs[player].left]) * 2;
		if (jdiry[player] == 0)
			jdiry[player] = key[keyconfigs[player].down] - key[keyconfigs[player].up];
		if (jaction[player] == 0)
			jaction[player] = key[keyconfigs[player].action1];
		if (jaction[player] == 0)
			jaction[player] = key[keyconfigs[player].action2];
		if (jsecond[player] == 0)
			jsecond[player] = key[keyconfigs[player].cancel];

		if (players[player] == 0)
			players[player] = key[keyconfigs[player].start];
	}
}

void SYSTEM_POLLINPUT()
{
	for (int i = 0; i < 2; i++)
	{
		UpdatePlayerInput(i);
	}

	if (key[SHIPPY_ESCAPE])
		done = 1;
	if (players[0] && players[1])
		done = 1;
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
		if ((event.type == SDL_KEYDOWN) || (event.type == SDL_KEYUP))
		{
			int keyIndx = -1;
			switch (event.key.keysym.sym)
			{
			case SDLK_LEFT:
				keyIndx = SHIPPY_LEFT;
				break;
			case SDLK_RIGHT:
				keyIndx = SHIPPY_RIGHT;
				break;
			case SDLK_DOWN:
				keyIndx = SHIPPY_DOWN;
				break;
			case SDLK_UP:
				keyIndx = SHIPPY_UP;
				break;
			case SDLK_LCTRL:
				keyIndx = SHIPPY_LCTRL;
				break;
			case SDLK_BACKSPACE:
				keyIndx = SHIPPY_BACKSPACE;
				break;
			case SDLK_ESCAPE:
				keyIndx = SHIPPY_ESCAPE;
				break;
			case SDLK_RETURN:
				keyIndx = SHIPPY_RETURN;
				break;
			case SDLK_LALT:
				keyIndx = SHIPPY_LALT;
				break;
			case SDLK_d:
				keyIndx = SHIPPY_D;
				break;
			case SDLK_g:
				keyIndx = SHIPPY_G;
				break;
			case SDLK_f:
				keyIndx = SHIPPY_F;
				break;
			case SDLK_r:
				keyIndx = SHIPPY_R;
				break;
			case SDLK_q:
				keyIndx = SHIPPY_Q;
				break;
			case SDLK_s:
				keyIndx = SHIPPY_S;
				break;
			case SDLK_1:
				keyIndx = SHIPPY_1;
				break;
			case SDLK_2:
				keyIndx = SHIPPY_2;
				break;
			case SDLK_SPACE:
				keyIndx = SHIPPY_SPACE;
				break;
			default:
				break;
			}
			if (keyIndx == -1)
			{
			}
			else if (event.type == SDL_KEYDOWN)
			{
				key[keyIndx] = 1;
			}
			else if (event.type == SDL_KEYUP)
			{
				key[keyIndx] = 0;
			}
		}
		if ((event.type == SDL_KEYDOWN) && key[SHIPPY_LALT] && (event.key.keysym.sym == SDLK_RETURN) && (!use_arcade_mode))
		{
			fullscreen = !fullscreen;
			SDL_SetWindowFullscreen(screen, fullscreen ? SDL_WINDOW_FULLSCREEN : 0);
		}
	}

}

int main(int argc, char *argv[])
{
	return SHIPPY_MAIN(argc, argv);
}
