#ifndef __SHIPPY_H__
#define __SHIPPY_H__

#include "externs.h"

#ifndef DATADIR
#define DATADIR "data/"
#endif

void Start_Audio();
void audio_start();
void audio_play(char *wav);
void audio_music(char *mfile);
void audio_exec();
void audio_end();
void End_Audio();
void syncher();
void SCRAPPABLE_CLOSE();
void SYSTEM_CLEANBMP();
void SYSTEM_SETVID();
void SYSTEM_BG();
int SYSTEM_INIT();
int SYSTEM_CLEAN();
int SYSTEM_GETKEY(int scancode);
void SYSTEM_FINISHRENDER();
int SYSTEM_CLEARSCREEN();
void SYSTEM_BLIT(int sx, int sy, int x, int y, int szx, int szy);
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
void InitShippy();
void ExecShippy();
int SHIPPY_MAIN(int argc, char *argv[]);

#endif
