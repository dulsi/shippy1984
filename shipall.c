#include <allegro.h>
#include <aldumb.h>
#include "shippy.h"
int jdirx=0;
int jdiry=0;
int jaction=0;
int jsecond=0;
int waitforkey=360;

volatile int objectsynch = 0;

DUH *music=NULL;
AL_DUH_PLAYER *dp=NULL;

#define MAX_SAMPLES 8

struct SAMPLEHOLDER
{
    int loaded;
    char samplename[512];
    SAMPLE *playing;
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
        samples[i].playing=NULL;
        samples[i].voice=0;
        samples[i].counter=0;
        samples[i].istaken=0;
    }
    music = NULL;
}
    
void audio_start()
{
    audio_op = 1;
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
            samples[i].playing = load_sample(wav);
            strcpy(samples[i].samplename,wav);
            samples[i].voice = play_sample(samples[i].playing, 200, 128, 1000, 0);
            samples[i].loaded = 1;
            samples[i].istaken=1;
            ++samples[i].counter;            
            return;
        }
    }
}

void audio_music(char *mfile)
{
    if(dp!=NULL) al_stop_duh(dp);
    dp = NULL;
    if(music!=NULL) unload_duh(music);
    music = NULL;
    if(mfile == NULL) return;
	music = dumb_load_it(mfile);
	if(!music) music = dumb_load_xm(mfile);
    if(!music) music = dumb_load_s3m(mfile);
    if(!music) music = dumb_load_mod(mfile);

    dp = al_start_duh(music, 2, 0, 1.0f ,4096, 22050);


}
    
void audio_exec()
{
    int i;
    if(dp!=NULL) al_poll_duh(dp);
    if(audio_op==0) return;
    for(i=0;i<MAX_SAMPLES;++i)
    {
        if(samples[i].istaken)
        {
            if(samples[i].loaded==1)
            {
                if(voice_check(samples[i].voice)==NULL)
                {
                    --samples[i].counter;   
                    if(samples[i].counter<=0)
                    {
                        destroy_sample(samples[i].playing);
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
    if(dp!=NULL) al_stop_duh(dp);
    if(music!=NULL) unload_duh(music);
    for(i=0;i<MAX_SAMPLES;++i)
    {
        if(samples[i].istaken==1)
        {
            if(samples[i].playing) destroy_sample(samples[i].playing);
        }
    }
}
    
void End_Audio()
{
    audio_end();
}
//This changes later
int bitsperpixel = 0;
BITMAP *BackBuffer = NULL;
BITMAP *Graphics = NULL;

int done = 0;
int gscale = 1;

void syncher()
{
         ++objectsynch;

}

END_OF_FUNCTION(syncher)


void SCRAPPABLE_CLOSE()
{
    done = 1;
}

void SYSTEM_CLEANBMP()
{
    if(Graphics!=NULL)
    {
        destroy_bitmap(Graphics);
    }
    if(BackBuffer!=NULL)
    {
        destroy_bitmap(BackBuffer);
    }
    Graphics = NULL;
    BackBuffer = NULL;

}


void SYSTEM_SETVID()
{
    int GraphicsFlag;
    BITMAP *mytest2;
    RGB mypal[256];    
    set_color_depth(8);
    bitsperpixel = 8;
	set_color_conversion(COLORCONV_TOTAL);
    SYSTEM_CLEANBMP();
    

#ifdef ALLEGRO_DOS
    GraphicsFlag=set_gfx_mode(GFX_VGA, 320, 200, 0, 0);    
    if(GraphicsFlag!=0)
    {
        GraphicsFlag=set_gfx_mode(GFX_SAFE, 320, 200, 0, 0);
        if(GraphicsFlag!=0) exit(-1);
    }
	

#else
    GraphicsFlag=set_gfx_mode(GFX_AUTODETECT_WINDOWED, 480, 320, 0, 0);    
    if(GraphicsFlag!=0)
    {
        GraphicsFlag=set_gfx_mode(GFX_SAFE, 480, 320, 0, 0);
        if(GraphicsFlag!=0) exit(-1);
    }
#endif

    BackBuffer=create_system_bitmap(240,160);
    Graphics=create_system_bitmap(320,128);

    mytest2 = load_bmp("data/graphics.bmp",mypal);
    if(mytest2==NULL) return;
	blit(mytest2,Graphics,0,0,0,0,320,128);
	destroy_bitmap(mytest2);

    clear_to_color(BackBuffer,1);

#ifdef ALLEGRO_DOS
	mypal[0].r=0;
	mypal[0].g=0;
	mypal[0].b=0;
#endif

    set_palette(mypal);
}

int SYSTEM_BG(char *bmp)
{
    BITMAP *mytest2;
  	mytest2 = load_bmp(bmp,NULL);
    if(mytest2==NULL) return;
	blit(mytest2,BackBuffer,0,0,0,0,240,160);
	destroy_bitmap(mytest2);


}
int SYSTEM_INIT()
{
    int AllegroInitFlag;
    int GraphicsFlag;
    int width,height;
    BITMAP *mytest2;

    AllegroInitFlag = allegro_init();

    atexit(&dumb_exit);
    dumb_register_stdfiles();

    LOCK_VARIABLE(objectsynch);
    LOCK_FUNCTION(syncher);


    install_timer();
    install_int_ex(syncher, MSEC_TO_TIMER(14));
    install_joystick(JOY_TYPE_AUTODETECT);


    detect_digi_driver(DIGI_AUTODETECT);
    if(install_sound(DIGI_AUTODETECT,MIDI_NONE,NULL)==-1)
    {
        return -1;
    }
    
    Start_Audio();
    set_window_title("Shippy1984 by Ryan Broomfield ALLEGRO VERSION");
    install_keyboard();
    
    set_window_close_hook(SCRAPPABLE_CLOSE);



    SYSTEM_SETVID();    

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
    return key[scancode];
}

int SYSTEM_FINISHRENDER()
{
#ifdef ALLEGRO_DOS
    blit(BackBuffer,screen,0,0,40,20,240,160);
#else
    acquire_screen();
    stretch_blit(BackBuffer,screen,0,0,240,160,0,0,480,320);
    release_screen();
#endif
}
int SYSTEM_CLEARSCREEN()
{
    clear_to_color(BackBuffer,1);
    return 0;
}

int SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy)
{
    masked_blit(Graphics,BackBuffer,sx,sy,x,y,szx,szy);
}

void SYSTEM_POLLINPUT()
{
    int tx,ty;
    jaction = 0;
    jsecond = 0;
    jdirx = 0;
    jdiry = 0;


    if(SYSTEM_GETKEY(KEY_ESC)) done = 1;
    if(waitforkey>0)
    {
        --waitforkey;
        return;
    }

    if(num_joysticks>0)
    {
        poll_joystick();
        jaction = joy[0].button[0].b;
        jsecond = joy[0].button[1].b;
        jdirx = (joy[0].stick[0].axis[0].d1 - joy[0].stick[0].axis[0].d2)*2;
        jdiry = (joy[0].stick[0].axis[1].d1 - joy[0].stick[0].axis[1].d2);
    }
    if(jdirx == 0) jdirx=(SYSTEM_GETKEY(KEY_LEFT) - SYSTEM_GETKEY(KEY_RIGHT)) * 2;
    if(jdiry == 0) jdiry=SYSTEM_GETKEY(KEY_UP) - SYSTEM_GETKEY(KEY_DOWN);
    if(jaction == 0) jaction = SYSTEM_GETKEY(KEY_LCONTROL);
    if(jsecond == 0) jsecond = SYSTEM_GETKEY(KEY_BACKSPACE);
}

void SYSTEM_IDLE()
{

}

int main(int argc, char*argv[])
{
	SHIPPY_MAIN();

}
END_OF_MAIN()
