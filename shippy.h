#ifndef __SHIPPY_H__
#define __SHIPPY_H__

#include "externs.h"

#ifndef DATADIR
#define DATADIR "data/"
#endif

#define SOUND_DIE 0
#define SOUND_FANFARE 1
#define SOUND_HELIX 2
#define SOUND_HIT 3
#define SOUND_SHOT 4
#define SOUND_SPLASH 5
#define MAX_SOUNDS 6

void Start_Audio();
void audio_start();
void audio_play(int sound);
void audio_music(char *mfile);
void audio_exec();
void audio_end();
void End_Audio();
void syncher();
void SCRAPPABLE_CLOSE();
void SYSTEM_CLEANBMP();
void SYSTEM_SETVID();
void SYSTEM_SETMODE(int num);
void SYSTEM_BG();
void SYSTEM_DRAW_BG();
int SYSTEM_INIT();
int SYSTEM_CLEAN();
int SYSTEM_GETKEY(int scancode);
void SYSTEM_FINISHRENDER();
int SYSTEM_CLEARSCREEN();
void SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy);
void SYSTEM_COMICBUBLE(int dir, int y, const char *msg);
void SYSTEM_POLLINPUT();
void SYSTEM_IDLE();
int compare(const void *a, const void *b);
void PrintChar(char text, int x, int y, int textcolor);
void PrintMessage(char *text, int x, int y, int textcolor);
void DrawOverlay();
int IsHit(int x1, int y1, int r1, int x2, int y2, int r2);
int AddObject(int type, int x, int y, int level, int special, int health, char *msg, int dx, int dy);
void NewGame(int mlevel);
void RenderShippy(int objnumber);
void DoAi(int number);
void StoreHS();
void RestoreHS();
int GetGameState();
void StartGameState();
void InitShippy();
void ExecShippy();
int SHIPPY_MAIN(int argc, char *argv[]);

#endif
