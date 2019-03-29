#include <SDL/SDL.h>
#include <SDL/SDL_mixer.h>
#include "shippy.h"

SDL_Surface *screen = NULL;
SDL_Surface *BackBuffer = NULL;
SDL_Surface *Graphics = NULL;
Uint8 key[1337];

Uint32 CLEARCOLOR = 0;
SDL_Rect src;
SDL_Rect dest;

int jdirx=0;
int jdiry=0;
int jaction=0;
int jsecond=0;
int waitforkey=360;

volatile int objectsynch = 0;
Uint32 timing;

#define MAX_SAMPLES 8
SDL_Surface *CreateSurfaceFromBitmap(char *bmpfile,Uint32 flags)
{

    SDL_Surface *junktemp;
    SDL_Surface *junktemp2;

    junktemp=SDL_LoadBMP(bmpfile);
    
    junktemp2=SDL_ConvertSurface(junktemp, junktemp->format,flags);
    SDL_FreeSurface(junktemp);
    
    junktemp=SDL_DisplayFormat(junktemp2);
    SDL_FreeSurface(junktemp2);

    return junktemp;
}

int audio_op=0;
Mix_Music *music=NULL;

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
    audio_op=0;
    for(i=0;i<MAX_SAMPLES;++i)
    {
        samples[i].loaded=0;
        samples[i].samplename[0]=0;
        samples[i].sample=NULL;
        samples[i].voice=-1;
        samples[i].counter=0;
        samples[i].istaken=0;
    }
    music = NULL;
}
    

void audio_start()
{
    if(Mix_OpenAudio(MIX_DEFAULT_FREQUENCY, MIX_DEFAULT_FORMAT , 2, 2048)!=0) audio_op = 0;
    else audio_op = 1;    
}
    
void audio_play(char *wav)
{
    //check to see if the sample is loaded
    int i;    
    if(audio_op==0) return;
    for(i=0;i<MAX_SAMPLES;++i)
    {
        if(samples[i].istaken==0)
        {
            samples[i].sample = Mix_LoadWAV(wav);
            strcpy(samples[i].samplename,wav);
            samples[i].voice = Mix_PlayChannel(-1, samples[i].sample, 0);
            samples[i].loaded = 1;
            samples[i].istaken=1;
            ++samples[i].counter;            
            return;
        }
    }
}

void audio_music(char *mfile)
{
    if(audio_op==0) return;

    if(music!=NULL)
    {
        Mix_FreeMusic(music);
    }
    music=Mix_LoadMUS(mfile);
    if(!music)
    {
        printf("Mix_LoadMUS(%s): %s\n", mfile,Mix_GetError());
        audio_op = 0;
    }
    else if(Mix_PlayMusic(music, -1)==-1)
    {
        printf("Mix_PlayMusic: %s\n", Mix_GetError());
    }
    // well, there's no music, but most games don't break without music...
   
}
    
void audio_exec()
{
    int i;
    if(audio_op==0) return;
    for(i=0;i<MAX_SAMPLES;++i)
    {
        if(samples[i].istaken)
        {
            if(samples[i].loaded==1)
            {
                if(Mix_Playing(samples[i].voice)==0)
                {
                    --samples[i].counter;   
                    if(samples[i].counter<=0)
                    {
                        Mix_HaltChannel(samples[i].voice);
                        samples[i].voice = -1;
                        Mix_FreeChunk(samples[i].sample);
                        samples[i].sample=NULL;
                        samples[i].istaken=0;
                    }
                    samples[i].loaded=0;
                }
            }
        }
    }
}
   
void audio_end()
{
    int i;
    audio_op = 0;
    for(i=0;i<MAX_SAMPLES;++i)
    {
        if(samples[i].istaken==1)
        {
            Mix_HaltChannel(samples[i].voice);
            samples[i].voice = -1;
            Mix_FreeChunk(samples[i].sample);
        }
    }
}
    
    
void End_Audio()
{
    Mix_HaltMusic();
    Mix_CloseAudio();
    audio_end();
}

    

int done = 0;
int gscale = 1;


void SYSTEM_CLEANBMP()
{
    if(BackBuffer!=NULL) SDL_FreeSurface(BackBuffer);
    if(Graphics!=NULL) SDL_FreeSurface(Graphics);
    BackBuffer=NULL;
    Graphics=NULL;
}


void SYSTEM_SETVID()
{
    screen = SDL_SetVideoMode(480, 320, 8, SDL_SWSURFACE | SDL_HWPALETTE);
    if ( screen == NULL )
    {
        return;
    }

    SYSTEM_CLEANBMP();

    Graphics = CreateSurfaceFromBitmap("data/graphics.bmp",SDL_SWSURFACE|SDL_SRCCOLORKEY);
    BackBuffer= CreateSurfaceFromBitmap("data/splash.bmp",SDL_SWSURFACE|SDL_SRCCOLORKEY);
    SDL_SetColors(screen, Graphics->format->palette->colors, 0,Graphics->format->palette->ncolors);
    SDL_SetColorKey(Graphics,SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));
    SDL_SetColorKey(BackBuffer,SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));
    SDL_SetClipRect(screen, NULL);
    SDL_FillRect(BackBuffer, NULL, CLEARCOLOR);
    CLEARCOLOR = SDL_MapRGB(Graphics->format, 0, 0, 0);
}


int SYSTEM_INIT()
{
   
    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1))
    { 
        return 1;
    }
    timing = SDL_GetTicks();

    atexit(SDL_Quit);
    SYSTEM_SETVID();    
    SDL_WM_SetCaption("Shippy1984 by Ryan Broomfield SDL VERSION", NULL);

    audio_start();

    return 0;
}

int SYSTEM_CLEAN()
{
    SYSTEM_CLEANBMP();
    End_Audio();
    return 0;
}

int SYSTEM_GETKEY(int scancode)
{
    return 0;

}

int SYSTEM_BG(char *bmp)
{
    if(BackBuffer!=NULL) SDL_FreeSurface(BackBuffer);
    BackBuffer= CreateSurfaceFromBitmap(bmp,SDL_SWSURFACE|SDL_SRCCOLORKEY);
    if(BackBuffer==NULL) done=1;
    else SDL_SetColorKey(BackBuffer,SDL_SRCCOLORKEY, SDL_MapRGB(Graphics->format, 255, 0, 255));

}


/* NEW SYSTEM_FINISHRENDER() BY JONATHAN GILBERT 1-28-2004 */ 
int SYSTEM_FINISHRENDER()
{
  int x, y, w;
  Uint8 *in;
  Uint16 *out;
 
  if (SDL_MUSTLOCK(BackBuffer))
    SDL_LockSurface(BackBuffer);
  if (SDL_MUSTLOCK(screen))
    SDL_LockSurface(screen);
 
  in = BackBuffer->pixels;
  out = (Uint16 *)screen->pixels;
 
  w = BackBuffer->pitch;
 
  for (y=0; y<160; y++)
  {
    for (x=0; x<w; x += 2)
    {
      Uint32 sample = *(Uint16 *)&in[x];
 
      sample = (sample & 0xFF) | (sample >> 8 << 16);
      sample *= 257;
 
      *(Uint32 *)(&out[x]) = sample;
      *(Uint32 *)(&out[x + w]) = sample;
    }
 
    in += w;
    out += w + w;
  }
 
  if (SDL_MUSTLOCK(BackBuffer))
    SDL_UnlockSurface(BackBuffer);
  if (SDL_MUSTLOCK(screen))
    SDL_UnlockSurface(screen);
 
  SDL_UpdateRect(screen, 0, 0, 0, 0);
}


int SYSTEM_CLEARSCREEN()
{
    if(SDL_FillRect(BackBuffer, NULL, CLEARCOLOR)==-1)
    {
        printf("CLS ERROR! \n");
    }
}

int SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy)
{
    src.x = sx;
    src.y = sy;
    src.w=szx;
    src.h=szy;
    dest.x=x;
    dest.y=y;
    dest.w=szx;
    dest.h=szy;
    if(SDL_BlitSurface(Graphics,&src,BackBuffer,&dest)==-1)
    {
        printf("SYSTEM_BLIT ERROR! \n");
    }
    
}

void SYSTEM_POLLINPUT()
{
    int tx,ty;
    jaction = 0;
    jsecond = 0;
    jdirx = 0;
    jdiry = 0;


    if(key[SDLK_ESCAPE]) done = 1;
    if(waitforkey>0)
    {
        --waitforkey;
        return;
    }

/*    if(num_joysticks>0)
    {
        poll_joystick();
        jaction = joy[0].button[0].b;
        jsecond = joy[0].button[1].b;
        jdirx = (joy[0].stick[0].axis[0].d1 - joy[0].stick[0].axis[0].d2)*2;
        jdiry = (joy[0].stick[0].axis[1].d1 - joy[0].stick[0].axis[1].d2);
    }
*/

    jdirx = 0;
    jdiry = 0;
    jaction = 0;
    jsecond = 0;

    if(jdirx == 0) jdirx=(key[SDLK_RIGHT] - key[SDLK_LEFT]) * 2;
    if(jdiry == 0) jdiry=key[SDLK_DOWN] - key[SDLK_UP];
    if(jaction == 0) jaction = key[SDLK_LCTRL];
    if(jsecond == 0) jsecond = key[SDLK_BACKSPACE];


}

void SYSTEM_IDLE()
{
    Uint32 test = SDL_GetTicks();
    while(test>timing)
    {
        timing+=14;
        ++objectsynch;
    }

  SDL_Event event;
  /* Poll for events. SDL_PollEvent() returns 0 when there are no  */
  /* more events on the event queue, our while loop will exit when */
  /* that occurs.                                                  */
    while( SDL_PollEvent( &event ) )
    {
        /* We are only worried about SDL_KEYDOWN and SDL_KEYUP events */
        if(event.type==SDL_QUIT)
        {
            done=1;
        }
        if(event.type==SDL_KEYDOWN)
        {
            key[event.key.keysym.sym]=1;
        }
        else if(event.type==SDL_KEYUP)
        {
            key[event.key.keysym.sym]=0;
        }
    }
    
}

int main(int argc, char*argv[])
{

	SHIPPY_MAIN();

}
