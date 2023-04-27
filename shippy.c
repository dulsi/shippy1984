/*Shippy 1984 by Ryan Broomfield
Source code and graphics are copyright Ryan Broomfield 2004.
Music is copyright neoblaze 2004.
*/

#ifdef __unix__
/* this must be done before the first include of unistd.h for setresgid */
#define _GNU_SOURCE
#include <unistd.h>
#endif
#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include "shippy.h"
#include "path.h"

#define TEXT_WHITE 0
#define TEXT_RED 1
#define TEXT_YELLOW 2
#define TEXT_CYAN 3

#define SHIPPY 0
#define SHIPPY2 1
#define BULLET 2
#define BULLWAVE 3
#define ORB 4
#define ENEMYSHIPPY 5
#define ENEMYHEART 6
#define ENEMYBULLET 7
#define ENEMYBHEART 8
#define EXPLOSION 9
#define POWERUP 10
#define STAR 11
#define PARTICLE 12
#define LEVELMESSAGE 13
#define MESSAGE 14
#define ANGRYFEZ 15
#define FEZBOMB 16
#define CUTSCENE 17
#define SPLASH -1
#define TITLE 1
#define DEMO 2
#define GAME 3
#define SCORES 4
#define POWER_RAPID 0
#define POWER_HELIX 1

#define SHIPPY_SPECIAL_NONE 0
#define SHIPPY_SPECIAL_FIRED 1
#define SHIPPY_SPECIAL_INITIAL 2
#define SHIPPY_SPECIAL_GAMEOVER 4

#define MAXPLAYERS 2

#ifdef GAMERZILLA
#include <gamerzilla.h>
#endif

struct SHIPPYINFO
{
	int x;
	int y;
	int dx;
	int dy;
	int type;
	int used;
	int special;
	int level;
	int health;
	int lives;
	char *msg;
};

struct HISCORE
{
	char name[4];
	int score;
	int level;
	int last;
};

//Shippy-related stuff here.
#define MAXSCORE 15
struct HISCORE winners[MAXSCORE];
#define MAXSHIPPY 384
struct SHIPPYINFO ShippyObjects[MAXSHIPPY];

int timeattack = 0;
int timelimit = 0;
int diedlast = 0;
int fps = 0;
int frames = 0;
int countframes = 0;
int gamestate = SPLASH;
int powerup = 0;
int powerupframe = 0;
int extralife[MAXPLAYERS] = { 50000, 50000 };
int shipwait = 0;
int leftmonsters = 0;
int numplayers = 1;
int level = 0;
int gameover = 0;
int shots[MAXPLAYERS] = { 0, 0 };
int score[MAXPLAYERS] = { 0, 0 };
int highscore;
char acc[32];
char eff[32];
char bon[64];
char tim[64];
int aiwavelet[240];
int cowardcount = 0;
int game_id = 0;

char curname[MAXPLAYERS][4];
char currchar[MAXPLAYERS] = { 'A', 'A' };
int curlevel[MAXPLAYERS];
int operational = 0;
int missedshots = 0;
int firedshots = 0;

FILE *highscore_fp;

int start_windowed = 0;
int use_arcade_mode = 0;
int screen_width = 960;
int screen_height = 640;

typedef struct cutscenepart_s {
	int who;
	char *message;
} cutscenepart;

typedef struct cutscene_s {
	int num;
	cutscenepart *parts;
} cutscene;

int numcutscenes;
cutscene *cutscenes;
int numdeathscenes;
cutscene *deathscenes;

int compare(const void *a, const void *b)
{
	struct HISCORE *test = (struct HISCORE *)a;
	struct HISCORE *test2 = (struct HISCORE *)b;
	return (*test2).score - (*test).score;
}

void PrintChar(char text, int x, int y, int textcolor)
{

	if (text >= 'A' && text <= 'Z')
	{
		SYSTEM_BLIT((text - 'A') * 8, textcolor * 8, x, y, 8, 8);
	}
	else if (text >= '0' && text <= '9')
	{
		SYSTEM_BLIT(208 + ((text - '0') * 8), textcolor * 8, x, y, 8, 8);
	}
	else if (text == '!')
	{
		SYSTEM_BLIT(288, textcolor * 8, x, y, 8, 8);
	}

}

void PrintMessage(char *text, int x, int y, int textcolor)
{
	while (*text != 0)
	{
		if (*text >= 'A' && *text <= 'Z')
		{
			SYSTEM_BLIT((*text - 'A') * 8, textcolor * 8, x, y, 8, 8);
		}
		else if (*text >= '0' && *text <= '9')
		{
			SYSTEM_BLIT(208 + ((*text - '0') * 8), textcolor * 8, x, y, 8, 8);
		}
		else if (*text == '!')
		{
			SYSTEM_BLIT(288, textcolor * 8, x, y, 8, 8);
		}

		++text;
		x += 8;
	}
}

void DrawOverlay()
{
	int increment;
	char buf[64];
	int test;

	switch (gamestate)
	{
	case DEMO:
		PrintMessage("DEMO MODE!", 88, 112, TEXT_YELLOW);
	case GAME:

		if (timeattack >= 0 && timelimit - timeattack >= 0)
		{
			sprintf(buf, "%i", timelimit - timeattack);
			PrintMessage(buf, 104, 144, TEXT_WHITE);
		}

		for (increment = 0; increment < ShippyObjects[0].lives; ++increment)
		{
			SYSTEM_BLIT(0, 48, increment * 16, 144, 16, 16);
		}
		for (increment = 0; increment < ShippyObjects[1].lives; ++increment)
		{
			SYSTEM_BLIT(16, 48, 224 - (increment * 16), 144, 16, 16);
		}

		sprintf(buf, "%i", score[0]);
		PrintMessage("1UP", 0, 0, TEXT_RED);
		PrintMessage(buf, 0, 8, TEXT_WHITE);
		PrintMessage("HIGH SCORE", 80, 0, TEXT_RED);
		sprintf(buf, "%i", highscore);
		PrintMessage(buf, ((30 - strlen(buf)) / 2) * 8, 8, TEXT_WHITE);
		sprintf(buf, "%i", score[1]);
		PrintMessage("2UP", 215, 0, TEXT_RED);
		PrintMessage(buf, 240 - (strlen(buf) * 8), 8, TEXT_WHITE);

		if (powerupframe > 0)
		{
			switch (powerup)
			{
			case POWER_RAPID:
				PrintMessage("GO UNLIMITED BULLETS!", 64, 152, TEXT_CYAN);
				break;

			case POWER_HELIX:
				PrintMessage("GO GO POWERUP HELIX!", 64, 152, TEXT_CYAN);
				break;

			default:
				PrintMessage("GO GO POWERUP ERROR!", 64, 152, TEXT_CYAN);
				break;
			}
		}
		if (ShippyObjects[0].special == SHIPPY_SPECIAL_INITIAL)
		{
			PrintMessage("ENTER INITIALS!", 0, 146, TEXT_WHITE);
			for (int i = 0; i < 3; i++)
			{
				if (curname[0][i] == ' ')
				{
					PrintChar(currchar[0], 0 + (i * 8), 152, TEXT_YELLOW);
					SYSTEM_BLIT(0, 96, 0 + (i * 8), 152, 8, 8);
					break;
				}
				PrintChar(curname[0][i], 0 + (i * 8), 152, TEXT_WHITE);
			}
		}
		if (ShippyObjects[1].special == SHIPPY_SPECIAL_INITIAL)
		{
			PrintMessage("ENTER INITIALS!", 240 - 8 * 15, 146, TEXT_WHITE);
			for (int i = 0; i < 3; i++)
			{
				if (curname[1][i] == ' ')
				{
					PrintChar(currchar[1], 208 + (i * 8), 152, TEXT_YELLOW);
					SYSTEM_BLIT(0, 96, 208 + (i * 8), 152, 8, 8);
					break;
				}
				PrintChar(curname[1][i], 208 + (i * 8), 152, TEXT_WHITE);
			}
		}
		break;
	case TITLE:
		PrintMessage("SHIPPY 1984!", 56, 16, TEXT_YELLOW);
		if (use_arcade_mode)
		{
			PrintMessage("PRESS 1P OR 2P TO START", 24, 24, TEXT_WHITE);
		}
		else
		{
			PrintMessage("PRESS BUTTON 1 TO START", 24, 24, TEXT_WHITE);
			PrintMessage("PRESS CTRL TO START", 24, 32, TEXT_WHITE);
		}
		SYSTEM_BLIT(32, 48, 32, 48, 16, 16);
		PrintMessage(" 100 POINTS PLUS BONUS", 56, 48, TEXT_CYAN);
		SYSTEM_BLIT(48, 48, 32, 64, 16, 16);
		PrintMessage(" 500 POINTS PLUS BONUS", 56, 64, TEXT_CYAN);
		SYSTEM_BLIT(64, 48, 32, 80, 16, 16);
		PrintMessage("2000 POINTS PLUS BONUS", 56, 80, TEXT_CYAN);
		PrintMessage("    ARE DANGEROUS", 40, 128, TEXT_WHITE);
		SYSTEM_BLIT(0, 88, 40, 128, 24, 8);
		PrintMessage("  IS GOOD FOR YOU", 40, 136, TEXT_WHITE);
		SYSTEM_BLIT(24, 88, 40, 136, 8, 8);
		break;
	case SCORES:
		PrintMessage("ALL TIME BEST PLAYERS", 40, 8, TEXT_YELLOW);
		PrintMessage("NAME  LEVEL  SCORE", 40, 24, TEXT_CYAN);
		for (increment = 0; increment < MAXSCORE - 1; ++increment)
		{
			if (winners[increment].last == 1)
				SYSTEM_BLIT(0, 48, 32, 36 + (8 * increment), 16, 16);
			if (winners[increment].last == 2)
				SYSTEM_BLIT(16, 48, 32, 36 + (8 * increment), 16, 16);
			sprintf(buf, "%i", winners[increment].score);
			test = strlen(buf);
			PrintMessage(buf, 192 - (test * 8), 40 + (8 * increment), TEXT_WHITE);
			sprintf(buf, "%i", winners[increment].level);
			test = strlen(buf);
			PrintMessage(buf, 130 - (test * 8), 40 + (8 * increment), TEXT_WHITE);

			PrintMessage(winners[increment].name, 48, 40 + (8 * increment), TEXT_WHITE);
		}
		break;
	}
}

int IsHit(int x1, int y1, int r1, int x2, int y2, int r2)
{
	int d = r1 + r2;
	int a = x2 - x1;
	int b = y2 - y1;
	if (a > d || a < -d || b > d || b < -d)
		return 0;
	return a * a + b * b < d * d;
}

int AddObject(int type, int x, int y, int level, int special, int health, char *msg, int dx, int dy)
{
	int increment;

	for (increment = 0; increment < MAXSHIPPY; ++increment)
	{
		if (ShippyObjects[increment].used == 0)
		{
			ShippyObjects[increment].type = type;
			ShippyObjects[increment].x = x;
			ShippyObjects[increment].y = y;
			ShippyObjects[increment].used = 1;
			ShippyObjects[increment].special = special;
			ShippyObjects[increment].level = level;
			ShippyObjects[increment].dy = dy;
			ShippyObjects[increment].dx = dx;
			ShippyObjects[increment].msg = msg;
			ShippyObjects[increment].health = health;
			if (((type == SHIPPY) || (type == SHIPPY2)) && (ShippyObjects[increment].special == SHIPPY_SPECIAL_GAMEOVER))
				ShippyObjects[increment].lives = -1;
			else
				ShippyObjects[increment].lives = 2;
			return 1;
		}
	}
	//Uh-oh
	return 0;
}

void NewGame(int mlevel)
{
	int increment;
	int adder;
	int bonus;
	cutscene *currentscene;

	currentscene = NULL;
	for (increment = 0; increment < MAXPLAYERS; ++increment)
		shots[increment] = 0;
	powerupframe = 0;
	switch (mlevel)
	{
	case -2:
		for (int i = 0; i < MAXPLAYERS; i++)
			score[i] = 0;
		for (increment = 0; increment < MAXSHIPPY; ++increment)
		{
			ShippyObjects[increment].used = 0;
		}
		shipwait = 600;
		break;
	case -1:
		for (int i = 0; i < MAXPLAYERS; i++)
			score[i] = 0;
		for (increment = 0; increment < MAXSHIPPY; ++increment)
		{
			ShippyObjects[increment].used = 0;
		}
		for (increment = 0; increment < 20; ++increment)
		{
			AddObject(STAR, rand() % 232 + 8, rand() % 152 + 8, rand() % 200 + 40, rand() % 4 + 2, 100, NULL, 0, 0);
		}
		shipwait = 600;
		break;
	case 1:
		for (increment = MAXPLAYERS; increment < MAXSHIPPY; ++increment)
		{
			ShippyObjects[increment].used = 0;
		}
		if (diedlast != 0)
		{
			diedlast = 0;
			for (increment = 0; increment < MAXPLAYERS; increment++)
			{
				if ((ShippyObjects[increment].special != SHIPPY_SPECIAL_GAMEOVER) && (ShippyObjects[increment].health <= -1))
				{
					ShippyObjects[increment].health = 10;
				}
			}
			if ((mode > 0) && (numdeathscenes > 0))
				currentscene = &deathscenes[rand() % numdeathscenes];
		}
		else if ((mode > 0) && (mlevel <= numcutscenes))
		{
			currentscene = &cutscenes[mlevel - 1];
		}
		if (currentscene == NULL)
			AddObject(LEVELMESSAGE, 0, 0, mlevel, 180, 0, NULL, 0, 0);
		for (increment = 0; increment < 20; ++increment)
		{
			AddObject(STAR, rand() % 232 + 8, rand() % 152 + 8, rand() % 200 + 40, rand() % 4 + 2, 100, NULL, 0, 0);
		}
		for (increment = 0; increment < 15; ++increment)
		{
			AddObject(ENEMYSHIPPY, 8 + ((increment % 15) * 16), 16 + ((increment / 15) * 32), 1, 50 + rand() % 100, 10, NULL, 0, 0);
		}
		leftmonsters = 15;
		if (currentscene != NULL)
		{
			increment = 100;
			for (int i = 0; i < currentscene->num; i++)
			{
				AddObject(CUTSCENE, 0, 103, 400 - increment, 300, currentscene->parts[i].who, currentscene->parts[i].message, 0, 0);
				increment += 100;
			}
		}
		else
		{
			if (use_arcade_mode == 0)
			{
				AddObject(MESSAGE, 0, 96, 300, 300, 0, "PRESS BUTTON 1 TO FIRE GUNS!", 0, 0);
				AddObject(MESSAGE, 0, 88, 300, 300, 0, "PRESS CTRL TO FIRE GUNS!", 0, 0);
			}
			AddObject(MESSAGE, 0, 80, 300, 300, 0, "READY", 0, 0);
			AddObject(MESSAGE, 0, 80, 180, 480, 0, "GO!", 0, 0);
		}
		for (int i = 0; i < MAXPLAYERS; i++)
			extralife[i] = 50000;
		shipwait = 300;
		timelimit = (72 * 9) + (72 * mlevel);
		timeattack = -shipwait;
		missedshots = 0;
		firedshots = 0;
		cowardcount = 0;

		break;
	default:
		if ((diedlast != 0) && (mode > 0) && (numdeathscenes > 0))
			currentscene = &deathscenes[rand() % numdeathscenes];
		else if ((mode > 0) && (mlevel <= numcutscenes))
		{
			currentscene = &cutscenes[mlevel - 1];
		}
		for (increment = MAXPLAYERS; increment < MAXSHIPPY; ++increment)
		{
			ShippyObjects[increment].used = 0;
		}
		if (currentscene == NULL)
			AddObject(LEVELMESSAGE, 0, 0, mlevel, 300, 0, NULL, 0, 0);

		for (increment = 0; increment < 20; ++increment)
		{
			AddObject(STAR, rand() % 232 + 8, rand() % 152 + 8, rand() % 200 + 40, rand() % 4 + 2, 100, NULL, 0, 0);
		}
		adder = 15 + (2 * mlevel);
		if (adder > 30)
			adder = 30;
		for (increment = 0; increment < adder; ++increment)
		{
			AddObject(ENEMYSHIPPY, 8 + ((increment % 15) * 16), 16 + ((increment / 15) * 32), 1, 50 + rand() % 100, 10, NULL, 0, 0);
		}
		leftmonsters = adder;
		if (mlevel > 5)
		{
			adder = mlevel - 5;
			if (adder > 10)
				adder = 10;
			for (increment = 0; increment < adder; ++increment)
			{
				AddObject(ENEMYHEART, 8 + (increment * 16), (rand() % 110) + 16, (rand() % 2) ? -1 : 1, 50 + rand() % 100, 10, NULL, rand() % 220 + 8, rand() % 110 + 8);

			}
			leftmonsters += adder;
		}
		if (mlevel > 10)
		{
#ifdef GAMERZILLA
			if (mlevel == 11)
			{
				bool achieved;
				if ((GamerzillaGetTrophy(game_id, "Expert Pilot", &achieved)) && (!achieved))
					GamerzillaSetTrophy(game_id, "Expert Pilot");
			}
			if (mlevel == 21)
			{
				bool achieved;
				if ((GamerzillaGetTrophy(game_id, "Master Pilot", &achieved)) && (!achieved))
					GamerzillaSetTrophy(game_id, "Master Pilot");
			}
#endif
			adder = mlevel - 10;
			if (adder > 10)
				adder = 10;
			for (increment = 0; increment < adder; ++increment)
			{
				AddObject(ANGRYFEZ, (rand() % 15) * 16, (rand() % 60) + 32, (rand() % 2) ? -1 : 1, 50 + rand() % 100, 10, NULL, rand() % 220 + 8, rand() % 120 + 8);

			}
			leftmonsters += adder;
		}

		if (diedlast != 0)
		{
			for (increment = 0; increment < MAXPLAYERS; increment++)
			{
				if ((ShippyObjects[increment].special != SHIPPY_SPECIAL_GAMEOVER) && (ShippyObjects[increment].health <= -1))
				{
					ShippyObjects[increment].health = 10;
				}
			}
			if (currentscene != NULL)
			{
				increment = 100;
				for (int i = 0; i < currentscene->num; i++)
				{
					AddObject(CUTSCENE, 0, 103, 400 - increment, 300, currentscene->parts[i].who, currentscene->parts[i].message, 0, 0);
					increment += 100;
				}
			}
			else
			{
				AddObject(MESSAGE, 0, 92, 120, 300, 0, "READY", 0, 0);
				AddObject(MESSAGE, 0, 92, 180, 480, 0, "GO!", 0, 0);
			}

			shipwait = 300;
			timelimit = (72 * 9) + (36 * mlevel);
			timeattack = -shipwait;
			missedshots = 0;
			firedshots = 0;
			diedlast = 0;
			return;
		}

		bonus = 0;

		if (firedshots == 0)
		{

			bonus = 0;
			sprintf(acc, "COWARD AWARD! 0 SHOTS FIRED!");
			cowardcount++;
#ifdef GAMERZILLA
			if (cowardcount == 3)
			{
				bool achieved;
				if ((GamerzillaGetTrophy(game_id, "Chicken", &achieved)) && (!achieved))
					GamerzillaSetTrophy(game_id, "Chicken");
			}
#endif
		}
		else
		{
			cowardcount = 0;
			if (firedshots == missedshots)
			{
				bonus = 0;
				sprintf(acc, "TRIGGERHAPPY! HIT NO ENEMIES!");
			}

			else if (firedshots >= 1 && missedshots >= 1)
			{
				bonus = ((firedshots * 100) / (firedshots - missedshots)) * 20;
				sprintf(acc, "ACCURACY %i OF %i!", firedshots - missedshots, firedshots);
			}
			else if (firedshots >= 1 && missedshots == 0)
			{
				bonus = 5000;
				sprintf(acc, "MARKSMAN 5000 POINTS");
#ifdef GAMERZILLA
				bool achieved;
				if ((GamerzillaGetTrophy(game_id, "Marksman", &achieved)) && (!achieved))
					GamerzillaSetTrophy(game_id, "Marksman");
#endif
			}
		}

		if (timeattack >= timelimit)
		{
			sprintf(eff, "NO TIME LEFT!");
			sprintf(bon, "TOTAL BONUS %i!", bonus);

		}
		else
		{
			sprintf(eff, "TIME LEFT %i", timelimit - timeattack);
			bonus += (timelimit - timeattack) * 10;
			sprintf(bon, "TOTAL TIME ATTACK BONUS %i!", bonus);
		}

		for (int i = 0; i < MAXPLAYERS; i++)
			if ((ShippyObjects[i].special != SHIPPY_SPECIAL_GAMEOVER) && (ShippyObjects[i].special != SHIPPY_SPECIAL_INITIAL) && (ShippyObjects[i].health > 0))
				score[i] += bonus;
		AddObject(MESSAGE, 0, 32, 180, 180, 0, acc, 0, 0);
		AddObject(MESSAGE, 0, 40, 180, 180, 0, eff, 0, 0);
		AddObject(MESSAGE, 0, 48, 180, 180, 0, bon, 0, 0);
		if (currentscene != NULL)
		{
			increment = 100;
			for (int i = 0; i < currentscene->num; i++)
			{
				AddObject(CUTSCENE, 0, 103, 400 - increment, 300, currentscene->parts[i].who, currentscene->parts[i].message, 0, 0);
				increment += 100;
			}
		}
		else
		{
			AddObject(MESSAGE, 0, 92, 120, 300, 0, "READY", 0, 0);
			AddObject(MESSAGE, 0, 92, 180, 480, 0, "GO!", 0, 0);
		}

		shipwait = 300;
		timelimit = (72 * 9) + (36 * mlevel);
		timeattack = -shipwait;
		missedshots = 0;
		firedshots = 0;
		diedlast = 0;

		for (increment = 0; increment < MAXPLAYERS; increment++)
		{
			if ((ShippyObjects[increment].special != SHIPPY_SPECIAL_GAMEOVER) && (ShippyObjects[increment].health <= -1))
			{
				ShippyObjects[increment].health = 10;
			}
		}
		break;
	}

}

void RenderShippy(int objnumber)
{

	char buf[30];
	if (ShippyObjects[objnumber].used == 0)
		return;
	switch (ShippyObjects[objnumber].type)
	{
	case SHIPPY:
		if (ShippyObjects[objnumber].health > 0)
		{
			SYSTEM_BLIT(0, 48, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 16, 16);
		}
		break;
	case SHIPPY2:
		if (ShippyObjects[objnumber].health > 0)
		{
			SYSTEM_BLIT(16, 48, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 16, 16);
		}
		break;
	case ENEMYSHIPPY:
		SYSTEM_BLIT(32, 48, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 16, 16);

		break;
	case ANGRYFEZ:
		SYSTEM_BLIT(64, 48, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 16, 16);
		break;
	case ENEMYHEART:
		SYSTEM_BLIT(48, 48, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 16, 16);
		break;
	case EXPLOSION:
		if (ShippyObjects[objnumber].special <= 31)
		{
			SYSTEM_BLIT((ShippyObjects[objnumber].special / 8) * 8, 80, ShippyObjects[objnumber].x - 8, ShippyObjects[objnumber].y - 8, 8, 8);
		}
		break;

	case STAR:
		SYSTEM_BLIT((ShippyObjects[objnumber].level / 64), 104, ShippyObjects[objnumber].x, ShippyObjects[objnumber].y, 1, 1);
		break;

	case PARTICLE:
		SYSTEM_BLIT(((255 - ShippyObjects[objnumber].level) / 64), 105, ShippyObjects[objnumber].x / 64, ShippyObjects[objnumber].y / 64, 1, 1);
		break;

	case POWERUP:
		SYSTEM_BLIT(24, 88, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;
	case LEVELMESSAGE:
		if (shipwait < 300)
		{
			sprintf(buf, "LEVEL %i", ShippyObjects[objnumber].level);
			PrintMessage(buf, ((31 - strlen(buf)) / 2) * 8, 104, TEXT_CYAN);
		}
		break;

	case MESSAGE:
		if (ShippyObjects[objnumber].special < ShippyObjects[objnumber].level)
			PrintMessage(ShippyObjects[objnumber].msg, ((31 - strlen(ShippyObjects[objnumber].msg)) / 2) * 8, ShippyObjects[objnumber].y, TEXT_WHITE);
		break;

	case CUTSCENE:
		if (ShippyObjects[objnumber].special < ShippyObjects[objnumber].level)
		{
			if (ShippyObjects[objnumber].health == 0)
				SYSTEM_BLIT(32, 64, 8, 49, 48, 64);
			else
				SYSTEM_BLIT(80, 64, 175, 49, 48, 64);
			SYSTEM_COMICBUBLE(ShippyObjects[objnumber].health, ShippyObjects[objnumber].y, ShippyObjects[objnumber].msg);
		}
		break;

	case ENEMYBULLET:
		SYSTEM_BLIT(0, 88, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;

	case FEZBOMB:
		SYSTEM_BLIT(16, 88, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;

	case ENEMYBHEART:
		SYSTEM_BLIT(8, 88, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;

	case BULLET:
		SYSTEM_BLIT(0, 96, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;

	case BULLWAVE:
		SYSTEM_BLIT(8, 96, ShippyObjects[objnumber].x - 4, ShippyObjects[objnumber].y - 4, 8, 8);
		break;

	}
}

int analyze()
{
	int tx;
	int checksum = 0;
	int checksum2 = 0;
	int movement = 0;
	int distance, distance2;

	ShippyObjects[0].dx = 1;

	for (tx = ShippyObjects[0].x - 12; tx < ShippyObjects[0].x + 12; ++tx)
	{
		if (tx >= 0 && tx <= 239)
			checksum += aiwavelet[tx];
	}
	if (checksum > 256)
	{
		checksum = 0;
		checksum2 = 0;

		distance = ShippyObjects[0].x - 8;
		distance2 = 232 - ShippyObjects[0].x;
		if (distance > distance2)
			distance = distance2;
		for (tx = ShippyObjects[0].x; tx >= ShippyObjects[0].x - distance; --tx)
		{
			if (tx >= 0 && tx <= 239)
				checksum += aiwavelet[tx] + ((ShippyObjects[0].x - tx));
		}
		for (tx = ShippyObjects[0].x; tx <= ShippyObjects[0].x + distance; ++tx)
		{
			if (tx >= 0 && tx <= 239)
				checksum2 += aiwavelet[tx] + ((tx - ShippyObjects[0].x));
		}
		if (checksum < checksum2)
			movement = -2;
		else
			movement = 2;

	}
	else
		movement = 0;

	memset(aiwavelet, 0, 240 * sizeof(int));
	return movement;

}

void DoAi(int number)
{
	if (ShippyObjects[number].used == 0)
		return;
	int increment;
	int runsim;

	switch (ShippyObjects[number].type)
	{
	case LEVELMESSAGE:
		--ShippyObjects[number].special;
		if (ShippyObjects[number].special < 0)
			ShippyObjects[number].used = 0;
		break;

	case MESSAGE:
		--ShippyObjects[number].special;
		if (ShippyObjects[number].special < 0)
			ShippyObjects[number].used = 0;
		break;

	case CUTSCENE:
		--ShippyObjects[number].special;
		if (ShippyObjects[number].special == ShippyObjects[number].level)
		{
			for (increment = 0; increment < MAXSHIPPY; ++increment)
			{
				if (increment != number && ShippyObjects[increment].used != 0 && ShippyObjects[increment].type == CUTSCENE)
				{
					if (ShippyObjects[increment].special < ShippyObjects[increment].level)
					{
						ShippyObjects[increment].y -= 12;
					}
				}
			}
		}
		if (ShippyObjects[number].special < 0)
			ShippyObjects[number].used = 0;
		break;

	case SHIPPY:
	case SHIPPY2:

		if (ShippyObjects[number].health == 0)
		{
			ShippyObjects[number].x = 150;
			ShippyObjects[number].y = 144;
			if (ShippyObjects[number].lives >= 0)
				--ShippyObjects[number].lives;
			if (ShippyObjects[number].lives < 0)
			{
				if ((ShippyObjects[number].special != SHIPPY_SPECIAL_INITIAL) && (ShippyObjects[number].special != SHIPPY_SPECIAL_GAMEOVER))
				{
					for (int i = 0; i < MAXSCORE; i++)
						if (winners[i].last == number + 1)
							winners[i].last = 0;
					if (score[number] <= winners[13].score)
					{
						ShippyObjects[number].special = SHIPPY_SPECIAL_GAMEOVER;
					}
					else
					{
						ShippyObjects[number].special = SHIPPY_SPECIAL_INITIAL;
						strcpy(curname[number], "   ");
						operational = 1;
						waitforkey[number] = 50;
						currchar[number] = 'A';
						curlevel[number] = level;
					}
				}
				if (ShippyObjects[number].special == SHIPPY_SPECIAL_INITIAL)
				{
					if (jdirx[number] <= -1)
					{
						--currchar[number];
						if (currchar[number] < 'A')
							currchar[number] = 'Z';
						waitforkey[number] = 15;
					}
					if (jdirx[number] >= 1)
					{
						++currchar[number];
						if (currchar[number] > 'Z')
							currchar[number] = 'A';
						waitforkey[number] = 15;
					}

					int initialdone = 0;
					if (jaction[number])
					{
						for (int i = 0; i < 3; i++)
						{
							if (curname[number][i] == ' ')
							{
								curname[number][i] = currchar[number];
								currchar[number] = 'A';
								waitforkey[number] = 20;
								if (i == 2)
									initialdone = 1;
								break;
							}
						}
					}

					if (jsecond[number])
					{
						for (int i = 1; i < 3; i++)
						{
							if (curname[number][i] == ' ')
							{
								curname[number][i - 1] = ' ';
							}
						}
						currchar[number] = 'A';
						waitforkey[number] = 20;
					}
					if (initialdone)
					{
						curname[number][3] = 0;
						strcpy(winners[14].name, curname[number]);
						winners[14].level = curlevel[number];
						winners[14].score = score[number];
						winners[14].last = number + 1;
						qsort(winners, MAXSCORE, sizeof(struct HISCORE), compare);
						waitforkey[number] = 360;
						ShippyObjects[number].special = SHIPPY_SPECIAL_GAMEOVER;
						return;
					}
				}
			}
			else
				ShippyObjects[number].health = -180;
			return;
		}
		if (ShippyObjects[number].health < 0)
		{
			return;
		}

		if (gamestate == GAME)
			runsim = jaction[number];
		else
		{
			if (rand() % 60 > 30)
				runsim = 1;
			else
				runsim = 0;

		}

		if (runsim && shipwait == 0)
		{
			if (powerupframe > 0)
			{

				switch (powerup)
				{
				case POWER_RAPID:
					if (ShippyObjects[number].special == SHIPPY_SPECIAL_NONE)
					{
						if (shots[number] < 10)
						{
							AddObject(BULLET, ShippyObjects[number].x, ShippyObjects[number].y, number, 0, 0, NULL, 0, 0);
							audio_play("shot.wav");
							++shots[number];
							++firedshots;
						}
						ShippyObjects[number].special = SHIPPY_SPECIAL_FIRED;
					}

					break;

				case POWER_HELIX:
					if (ShippyObjects[number].special == SHIPPY_SPECIAL_NONE)
					{
						if (shots[number] < 6)
						{
							audio_play("helix.wav");
							AddObject(BULLWAVE, ShippyObjects[number].x - 6, ShippyObjects[number].y - 8, number, -16, 0, NULL, 0, 0);
							AddObject(BULLWAVE, ShippyObjects[number].x + 6, ShippyObjects[number].y - 8, number, 16, 0, NULL, 0, 0);
							shots[number] += 2;
							firedshots += 2;

						}
						ShippyObjects[number].special = SHIPPY_SPECIAL_FIRED;
					}

					break;

				}

			}
			else
			{
				if (ShippyObjects[number].special == SHIPPY_SPECIAL_NONE)
				{
					if (shots[number] < 3)
					{
						AddObject(BULLET, ShippyObjects[number].x, ShippyObjects[number].y, number, 0, 0, NULL, 0, 0);
						audio_play("shot.wav");
						++shots[number];
						++firedshots;
					}
					ShippyObjects[number].special = SHIPPY_SPECIAL_FIRED;
				}
			}
		}
		else
		{
			ShippyObjects[number].special = SHIPPY_SPECIAL_NONE;
		}

		if (gamestate == GAME)
		{

			ShippyObjects[number].x += jdirx[number];
			ShippyObjects[number].y += jdiry[number];
		}
		else
		{

			if (shipwait == 0)
				ShippyObjects[number].x += analyze();
			else
			{
				if (ShippyObjects[number].x > 118)
					ShippyObjects[number].x -= 2;
				if (ShippyObjects[number].x < 118)
					ShippyObjects[number].x += 2;
			}

		}

		if (ShippyObjects[number].x < 8)
			ShippyObjects[number].x = 8;
		if (ShippyObjects[number].y < 96)
			ShippyObjects[number].y = 96;
		if (ShippyObjects[number].x > 232)
			ShippyObjects[number].x = 232;
		if (ShippyObjects[number].y > 144)
			ShippyObjects[number].y = 144;

#ifdef GODMODE
		if (jsecond[0])
			leftmonsters = 0;
#endif
		break;

	case ENEMYHEART:
	case ENEMYSHIPPY:
	case ANGRYFEZ:
		if (ShippyObjects[number].type == ENEMYHEART)
		{
			if (IsHit(ShippyObjects[number].dx, ShippyObjects[number].dy, 32, ShippyObjects[number].x, ShippyObjects[number].y, 8))
			{
				ShippyObjects[number].dx = (rand() % 220) + 8;
				ShippyObjects[number].dy = (rand() % 100) + 8;

			}
			else
			{
				if (ShippyObjects[number].x < ShippyObjects[number].dx)
					ShippyObjects[number].x++;
				else if (ShippyObjects[number].x > ShippyObjects[number].dx)
					ShippyObjects[number].x--;
				if (ShippyObjects[number].y < ShippyObjects[number].dy)
					ShippyObjects[number].y++;
				else if (ShippyObjects[number].y > ShippyObjects[number].dy)
					ShippyObjects[number].y--;
			}
		}
		else if (ShippyObjects[number].type == ENEMYSHIPPY)
		{
			ShippyObjects[number].x += ShippyObjects[number].level;

		}
		else if (ShippyObjects[number].type == ANGRYFEZ)
		{

			if (IsHit(ShippyObjects[number].dx, ShippyObjects[number].dy, 64, ShippyObjects[number].x, ShippyObjects[number].y, 8))
			{
				ShippyObjects[number].dx = (rand() % 15) * 16;
				ShippyObjects[number].dy = ((rand() % 6) * 16) + 32;
				ShippyObjects[number].lives = 0;

			}
			else
			{
				if (ShippyObjects[number].lives != 0)
				{
					if (ShippyObjects[number].x < ShippyObjects[0].x - 48)
						ShippyObjects[number].x++;
					else if (ShippyObjects[number].x > ShippyObjects[0].x + 48)
						ShippyObjects[number].x--;
					if (ShippyObjects[number].y < ShippyObjects[number].dy)
						ShippyObjects[number].y++;
					else if (ShippyObjects[number].y > ShippyObjects[number].dy)
						ShippyObjects[number].y--;
				}
			}

		}

		if (ShippyObjects[number].special <= 0)
		{
			if (shipwait == 0)
			{
				if (ShippyObjects[number].type == ENEMYHEART)
					AddObject(ENEMYBHEART, ShippyObjects[number].x, ShippyObjects[number].y, 0, 0, 0, NULL, 0, 0);
				else if (ShippyObjects[number].type == ENEMYSHIPPY)
					AddObject(ENEMYBULLET, ShippyObjects[number].x, ShippyObjects[number].y, 0, 0, 0, NULL, 0, 0);
				else if (ShippyObjects[number].type == ANGRYFEZ)
				{
					AddObject(ENEMYBULLET, ShippyObjects[number].x, ShippyObjects[number].y, 0, 0, 0, NULL, 0, 0);
					AddObject(FEZBOMB, ShippyObjects[number].x, ShippyObjects[number].y, 0, 0, 0, NULL, 0, 0);
					ShippyObjects[number].lives = 1;
				}
			}

			ShippyObjects[number].special = 200 + (rand() % 400);
		}
		else
		{
			--ShippyObjects[number].special;
		}

		if (ShippyObjects[number].x < -7 || (ShippyObjects[number].x > 247 && ShippyObjects[number].type == ENEMYSHIPPY))
		{
			ShippyObjects[number].y += 16;
			ShippyObjects[number].level = -ShippyObjects[number].level;
			if ((ShippyObjects[number].y > 120 && ShippyObjects[number].y <= 130) || leftmonsters < 5 + level)
			{
				if ((ShippyObjects[number].y / 4) & 1)
				{
					if (ShippyObjects[number].level < 0)
						--ShippyObjects[number].level;
					else
						++ShippyObjects[number].level;
				}
			}

		}
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if ((ShippyObjects[i].special == SHIPPY_SPECIAL_GAMEOVER) || (ShippyObjects[i].special == SHIPPY_SPECIAL_INITIAL))
				continue;
			if (IsHit(ShippyObjects[number].x, ShippyObjects[number].y, 8, ShippyObjects[i].x, ShippyObjects[i].y, 8))
			{
#ifndef GODMODE
				if (ShippyObjects[i].health > 0 && shipwait == 0)
				{
					ShippyObjects[i].health = 0;
					for (runsim = 0; runsim < MAXSHIPPY; ++runsim)
					{
						AddObject(PARTICLE,
											ShippyObjects[number].x * 64,
											ShippyObjects[number].y * 64, (rand() % 3 > 1) ? 255 : 0, 255, 0, NULL, ((rand() % 3) - 1) * (64 + (rand() % 256)), ((rand() % 3) - 1) * (64 + (rand() % 256)));
					}

					audio_play("die.wav");
					ShippyObjects[number].type = EXPLOSION;
					ShippyObjects[number].special = 4;
					--leftmonsters;
				}
#endif
			}

		}

		if (ShippyObjects[number].y < 8)
			ShippyObjects[number].y = 8;
		if (ShippyObjects[number].y > 147)
		{
			ShippyObjects[number].used = 0;
			--leftmonsters;
		}

		break;

	case EXPLOSION:
		--ShippyObjects[number].special;
		if (ShippyObjects[number].special < 0)
		{
			if ((rand() % 100) > 1)
			{
				ShippyObjects[number].used = 0;
			}
			else
			{
				ShippyObjects[number].type = POWERUP;
				ShippyObjects[number].dx = rand() % 2;
			}
		}
		break;

	case BULLET:
	case BULLWAVE:

		if (ShippyObjects[number].type == BULLET)
		{
			ShippyObjects[number].y -= 2;

		}
		if (ShippyObjects[number].type == BULLWAVE)
		{
			ShippyObjects[number].y -= 2;
			if (ShippyObjects[number].special == 1 || ShippyObjects[number].special == -1)
			{
				ShippyObjects[number].special = -ShippyObjects[number].special * 32;
			}
			if (ShippyObjects[number].special < 0)
			{
				--ShippyObjects[number].x;
				++ShippyObjects[number].special;
			}
			else
			{
				++ShippyObjects[number].x;
				--ShippyObjects[number].special;
			}
		}

		if (ShippyObjects[number].y < 0 || ShippyObjects[number].x < 0 || ShippyObjects[number].x > 239 || ShippyObjects[number].y > 159)
		{
			ShippyObjects[number].used = 0;
			--shots[ShippyObjects[number].level];
			++missedshots;
			return;
		}
		else
		{
			for (increment = 0; increment < MAXSHIPPY; ++increment)
			{
				if ((ShippyObjects[increment].type ==
						 ENEMYSHIPPY
						 || ShippyObjects[increment].type ==
						 ANGRYFEZ
						 || ShippyObjects[increment].type ==
						 ENEMYHEART || ShippyObjects[increment].type == ENEMYBHEART || ShippyObjects[increment].type == FEZBOMB) && ShippyObjects[increment].used == 1)
				{
					if (IsHit(ShippyObjects[number].x, ShippyObjects[number].y, 2, ShippyObjects[increment].x, ShippyObjects[increment].y, 8))
					{
						ShippyObjects[increment].used = 0;
						ShippyObjects[number].used = 0;

						if (ShippyObjects[increment].type == ENEMYSHIPPY || ShippyObjects[increment].type == ENEMYHEART || ShippyObjects[increment].type == ANGRYFEZ)
							--leftmonsters;
						switch (ShippyObjects[increment].type)
						{
						case ANGRYFEZ:
							score[ShippyObjects[number].level] += 2000 + ShippyObjects[increment].y;
							--shots[ShippyObjects[number].level];
							for (runsim = 0; runsim < 3; ++runsim)
							{
								AddObject(EXPLOSION, ShippyObjects[increment].x + ((rand() % 3) - 1) * 4, ShippyObjects[increment].y + ((rand() % 3) - 1) * 4, 0, rand() % 48, 0, NULL, 0, 0);

							}
							audio_play("hit.wav");
							break;

						case ENEMYSHIPPY:
							score[ShippyObjects[number].level] += 100 + ShippyObjects[increment].y;
							--shots[ShippyObjects[number].level];
							for (runsim = 0; runsim < 3; ++runsim)
							{
								AddObject(EXPLOSION, ShippyObjects[increment].x + ((rand() % 3) - 1) * 4, ShippyObjects[increment].y + ((rand() % 3) - 1) * 4, 0, rand() % 48, 0, NULL, 0, 0);
							}
							audio_play("hit.wav");
							break;

						case FEZBOMB:
							AddObject(ENEMYBHEART, ShippyObjects[number].x, ShippyObjects[number].y - 16, 0, 0, 0, NULL, 0, 0);
						case ENEMYHEART:
							if (ShippyObjects[increment].type == ENEMYHEART)
								score[ShippyObjects[number].level] += 500 + ShippyObjects[increment].y;
						case ENEMYBHEART:
							--shots[ShippyObjects[number].level];
							for (runsim = 0; runsim < 3; ++runsim)
							{
								AddObject(EXPLOSION, ShippyObjects[increment].x + ((rand() % 3) - 1) * 4, ShippyObjects[increment].y + ((rand() % 3) - 1) * 4, 0, rand() % 48, 0, NULL, 0, 0);
							}
							audio_play("hit.wav");
							break;

						}
						return;
					}
				}

			}

		}
		break;

	case STAR:
		ShippyObjects[number].y += ShippyObjects[number].special;
		if (ShippyObjects[number].y > 159)
			ShippyObjects[number].y = 0;
		break;

	case PARTICLE:
		ShippyObjects[number].y += ShippyObjects[number].dy;
		ShippyObjects[number].x += ShippyObjects[number].dx;
		ShippyObjects[number].special -= 4;
		ShippyObjects[number].level -= 4;

		if (ShippyObjects[number].y < 0
				|| ShippyObjects[number].y > 20416
				|| ShippyObjects[number].x < 0 || ShippyObjects[number].x > 15296 || (ShippyObjects[number].special < 0 || ShippyObjects[number].level < 0))
			ShippyObjects[number].used = 0;
		break;

	case POWERUP:
		ShippyObjects[number].y += 1;
		if (ShippyObjects[number].y > 144)
			ShippyObjects[number].y = 144;
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (IsHit(ShippyObjects[number].x, ShippyObjects[number].y, 4, ShippyObjects[i].x, ShippyObjects[i].y, 8) && ShippyObjects[i].health > 0)
			{
				++ShippyObjects[i].level;
				ShippyObjects[number].used = 0;
				powerup = ShippyObjects[number].dx;
				powerupframe = 360;
				return;
			}
		}

		break;

	case FEZBOMB:
	case ENEMYBHEART:
	case ENEMYBULLET:

		if (ShippyObjects[number].type == ENEMYBHEART)
		{
			if (ShippyObjects[number].x < ShippyObjects[0].x)
				++ShippyObjects[number].x;
			else
				--ShippyObjects[number].x;

		}
		if (ShippyObjects[number].type == FEZBOMB)
		{
			if (ShippyObjects[number].x < ShippyObjects[0].x - 6)
				++ShippyObjects[number].x;
			else if (ShippyObjects[number].x > ShippyObjects[0].x + 6)
				--ShippyObjects[number].x;
		}
		++ShippyObjects[number].y;

		if (gameover == 0)
		{
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				if ((ShippyObjects[i].special == SHIPPY_SPECIAL_GAMEOVER) || (ShippyObjects[i].special == SHIPPY_SPECIAL_INITIAL))
					continue;
				if (IsHit(ShippyObjects[number].x, ShippyObjects[number].y, 2, ShippyObjects[i].x, ShippyObjects[i].y, 8))
				{
	#ifndef GODMODE
					if (ShippyObjects[i].health > 0)
					{
						ShippyObjects[i].health = 0;
						for (runsim = 0; runsim < MAXSHIPPY; ++runsim)
						{
							AddObject(PARTICLE,
												ShippyObjects
												[number].x *
												64,
												ShippyObjects
												[number].y * 64, (rand() % 3 > 1) ? 255 : 0, 255, 0, NULL, ((rand() % 3) - 1) * (64 + (rand() % 256)), ((rand() % 3) - 1) * (64 + (rand() % 256)));
						}

						audio_play("die.wav");
						ShippyObjects[number].type = EXPLOSION;
						ShippyObjects[number].special = 4;
						return;
					}
	#endif
				}
			}
		}
		if (ShippyObjects[number].x > 0 && ShippyObjects[number].x < 239)
		{
			if (ShippyObjects[number].y > 159)
				ShippyObjects[number].used = 0;

			if (ShippyObjects[0].y - ShippyObjects[number].y > -8 && ShippyObjects[number].type == ENEMYBULLET)
			{
				if (ShippyObjects[0].y - ShippyObjects[number].y <= 8 && ShippyObjects[0].y - ShippyObjects[number].y >= -8)
				{
					aiwavelet[ShippyObjects[number].x] += 65536;
					aiwavelet[ShippyObjects[number].x + 1] += 65536;
					aiwavelet[ShippyObjects[number].x - 1] += 65536;
				}
				else if (ShippyObjects[0].y - ShippyObjects[number].y > aiwavelet[ShippyObjects[number].x])
				{
					aiwavelet[ShippyObjects[number].x] += 240 - (ShippyObjects[0].y - ShippyObjects[number].y);
					aiwavelet[ShippyObjects[number].x + 1] += 240 - (ShippyObjects[0].y - ShippyObjects[number].y);
					aiwavelet[ShippyObjects[number].x - 1] += 240 - (ShippyObjects[0].y - ShippyObjects[number].y);
				}
			}
		}

		break;

	}

}

void StoreHS()
{
	int i;
	size_t writeSize, writeActual;

	if (highscore_fp == NULL)
		return;

	rewind(highscore_fp);
	char data[1024];
	for (i = 0; i < 14; i++)
	{
		writeSize = snprintf(data, 1024, "%-3s %10d %10d\n", winners[i].name, winners[i].level, winners[i].score);
		writeActual = fwrite(data, 1, writeSize, highscore_fp);
		if (writeSize != writeActual)
		{
			printf("Error: Failed to write high scores\n");
			break;
		}
	}
	fflush(highscore_fp);
}

void RestoreHS()
{
	int i;

	if (highscore_fp != NULL)
	{
		char data[1024];
		size_t readSize = fread(data, 1, 1024, highscore_fp);
		if (readSize == sizeof(struct HISCORE) * 14)
		{
			memcpy(winners, data, sizeof(struct HISCORE) * 14);
			highscore = winners[0].score;
			return;
		}
		int where = 0;
		bool success = true;
		for (i = 0; i < 14; i++)
		{
			if (readSize < where + 26)
			{
				printf("Error: Failed to read high scores\n");
				success = false;
				break;
			}
			memcpy(winners[i].name, data + where, 3);
			winners[i].name[3] = 0;
			where += 4;
			winners[i].level = atol(data + where);
			where += 11;
			winners[i].score = atol(data + where);
			where += 11;
			while ((where < readSize) && ((data[where] == '\n') || (data[where] == '\r')))
				where++;
			winners[i].last = 0;
		}
		if (success)
			return;
	}

	strcpy(winners[0].name, "RAB");
	winners[0].level = 14;
	winners[0].score = 250000;

	highscore = winners[0].score;

	strcpy(winners[1].name, "SCM");
	winners[1].level = 12;
	winners[1].score = 240000;

	strcpy(winners[2].name, "BDH");
	winners[2].level = 11;
	winners[2].score = 230000;

	strcpy(winners[3].name, "JRP");
	winners[3].level = 9;
	winners[3].score = 200000;

	strcpy(winners[4].name, "JDE");
	winners[4].level = 8;
	winners[4].score = 175000;

	strcpy(winners[5].name, "BJM");
	winners[5].level = 8;
	winners[5].score = 150000;

	strcpy(winners[6].name, "RML");
	winners[6].level = 7;
	winners[6].score = 125000;

	strcpy(winners[7].name, "JDW");
	winners[7].level = 7;
	winners[7].score = 100000;

	strcpy(winners[8].name, "NPC");
	winners[8].level = 4;
	winners[8].score = 75000;

	strcpy(winners[9].name, "CDC");
	winners[9].level = 4;
	winners[9].score = 65000;

	strcpy(winners[10].name, "OMG");
	winners[10].level = 4;
	winners[10].score = 60000;

	strcpy(winners[11].name, "BMB");
	winners[11].level = 4;
	winners[11].score = 55000;

	strcpy(winners[12].name, "EJP");
	winners[12].level = 4;
	winners[12].score = 50000;

	strcpy(winners[13].name, "SCB");
	winners[13].level = 1;
	winners[13].score = 40000;
	for (i = 0; i < 14; ++i)
		winners[i].last = 0;

}

void ReadCutScene(FILE *cutfile, cutscene *cut)
{
	char line[1024];
	if (NULL == fgets(line, 1024, cutfile))
	{
		fprintf(stderr, "Error reading cutscenes\n");
		exit(2);
	}
	cut->num = atoi(line);
	cut->parts = (cutscenepart *)malloc(sizeof(cutscenepart) * cut->num);
	for (int j = 0; j < cut->num; j++)
	{
		if (NULL == fgets(line, 1024, cutfile))
		{
			fprintf(stderr, "Error reading cutscenes\n");
			exit(2);
		}
		cut->parts[j].who = atoi(line);
		if (NULL == fgets(line, 1024, cutfile))
		{
			fprintf(stderr, "Error reading cutscenes\n");
			exit(2);
		}
		int len = strlen(line);
		if (len > 0 && (line[len - 1] == '\n'))
			line[len - 1] = 0;
		cut->parts[j].message = strdup(line);
	}
}

void ReadCutScenes(FILE *cutfile, int *num, cutscene **cuts)
{
	char line[1024];
	if (NULL == fgets(line, 1024, cutfile))
	{
		fprintf(stderr, "Error reading cutscenes\n");
		exit(2);
	}
	*num = atoi(line);
	*cuts = (cutscene *)malloc(sizeof(cutscene) * (*num));
	for (int i = 0; i < *num; i++)
	{
		ReadCutScene(cutfile, (*cuts) + i);
	}
}

void InitCutScenes()
{
	char fname[1024];
	FILE *cutscenefile;
	numcutscenes = 0;
	numdeathscenes = 0;
	strcpy(fname, get_data_path());
	strcat(fname, "cutscenes.txt");
	cutscenefile = fopen(fname, "r");
	if (cutscenefile == NULL)
		return;
	ReadCutScenes(cutscenefile, &numcutscenes, &cutscenes);
	ReadCutScenes(cutscenefile, &numdeathscenes, &deathscenes);
	fclose(cutscenefile);
}

#define GAMESTATE_ACTIVE 0
#define GAMESTATE_GAMEOVER 1
#define GAMESTATE_NEXTLEVEL 2
#define GAMESTATE_REDOLEVEL 3
#define GAMESTATE_INITIALS 5

int GetGameState()
{
	int test = 1;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (ShippyObjects[i].special != SHIPPY_SPECIAL_GAMEOVER)
		{
			test = 0;
		}
	}
	if (test)
		return GAMESTATE_GAMEOVER;
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if ((ShippyObjects[i].special != SHIPPY_SPECIAL_GAMEOVER) && (ShippyObjects[i].special != SHIPPY_SPECIAL_INITIAL))
		{
			if (ShippyObjects[i].health >= 0)
			{
				if (leftmonsters == 0)
					return GAMESTATE_NEXTLEVEL;
				else
					return GAMESTATE_ACTIVE;
			}
		}
	}
	for (int i = 0; i < MAXPLAYERS; i++)
	{
		if (ShippyObjects[i].special != SHIPPY_SPECIAL_GAMEOVER)
		{
			if (ShippyObjects[i].health < 0)
			{
				return GAMESTATE_REDOLEVEL;
			}
		}
	}
	return GAMESTATE_INITIALS;
}

void StartGameState()
{
	audio_music("shippy.xm");
	gamestate = GAME;
	operational = 0;
	if (players[1])
		numplayers = 2;
	else
		numplayers = 1;
	gameover = 0;
	level = 0;
	diedlast = 0;
	leftmonsters = 0;
	for (int increment = 0; increment < MAXPLAYERS; increment++)
	{
		score[increment] = 0;
		ShippyObjects[increment].used = 0;
	}
	AddObject(SHIPPY, 128, 300, 0, SHIPPY_SPECIAL_NONE, 100, NULL, 0, 0);
	if (numplayers == 2)
		AddObject(SHIPPY2, 128, 300, 0, SHIPPY_SPECIAL_NONE, 100, NULL, 0, 0);
	else
		AddObject(SHIPPY2, 128, 300, 0, SHIPPY_SPECIAL_GAMEOVER, 0, NULL, 0, 0);
};

void InitShippy()
{

	RestoreHS();

	gamestate = SPLASH;
	waitforkey[0] = waitforkey[1] = 180;
	done = 0;
	level = 0;
	shipwait = 600;
	gameover = 0;
	operational = 150;

}

void ExecShippy()
{
	int increment;
	if (gamestate != SPLASH)
		SYSTEM_CLEARSCREEN();
	switch (gamestate)
	{

	case SPLASH:
		if (operational > 30)
		{
			SYSTEM_CLEARSCREEN();
			int diff = 150 - operational;
			PrintMessage("SHIPPY1984 BIOS POST", 8, 8, TEXT_CYAN);
			PrintMessage("ROM 1 0X01", 8, 24, TEXT_RED);
			PrintMessage("ROM 2 0X02", 8, 32, TEXT_RED);
			PrintMessage("GFX 1 0X03", 8, 40, TEXT_RED);
			PrintMessage("SFX 1 0X04", 8, 48, TEXT_RED);
			PrintMessage("MUS 1 0X05", 8, 56, TEXT_RED);
			if (diff > 20)
				PrintMessage("OK!", 104, 24, TEXT_YELLOW);
			if (diff > 40)
				PrintMessage("OK!", 104, 32, TEXT_YELLOW);
			if (diff > 60)
				PrintMessage("OK!", 104, 40, TEXT_YELLOW);
			if (diff > 80)
				PrintMessage("OK!", 104, 48, TEXT_YELLOW);
			if (diff > 100)
				PrintMessage("OK!", 104, 56, TEXT_YELLOW);
		}

		if (operational == 30)
		{
			SYSTEM_BG("splash.bmp");
		}
		if (operational <= 30)
		{
			SYSTEM_DRAW_BG();
		}

		if (operational > 0)
		{
			--operational;
			if (operational == 0)
				audio_play("splash.wav");
		}
		if (waitforkey[0] == 0)
		{
			if (use_arcade_mode == 1)
				PrintMessage("PRESS 1P OR 2P", 64, 98, TEXT_WHITE);
			else
				PrintMessage("PRESS CTRL OR BUTTON 1", 32, 98, TEXT_WHITE);

		}
		if (shipwait > 0)
			--shipwait;
		if (jdiry[0] > 0)
			SYSTEM_SETMODE(0);
		else if (jdiry[0] < 0)
			SYSTEM_SETMODE(1);
		if (jaction[0] || jsecond[0] || players[0] || players[1])
		{
			StartGameState();
		}
		else if (shipwait == 0)
		{
			gamestate = TITLE;
			done = 0;
			level = 0;
			shipwait = 0;
			gameover = 0;
			SYSTEM_CLEARSCREEN();
			operational = 0;

		}
		break;

	case GAME:

		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (score[i] > extralife[i])
			{
				ShippyObjects[i].lives++;
				extralife[i] *= 2;
				audio_play("fanfare.wav");

			}
		}
		if (done == 0)
		{
			++timeattack;
			for (increment = 0; increment < MAXSHIPPY; ++increment)
			{

				DoAi(increment);
			}
			if (gameover == 0)
			{
				int curstate = GetGameState();
				switch (curstate)
				{
					case GAMESTATE_REDOLEVEL:
						diedlast = 1;
						for (int i = 0; i < MAXPLAYERS; i++)
						{
							if (ShippyObjects[i].health < 0)
							{
								++ShippyObjects[i].health;
								if (ShippyObjects[i].health >= -1)
								{
									ShippyObjects[i].health = 10;
									NewGame(level);
								}
							}
						}
						break;
					case GAMESTATE_GAMEOVER:
						AddObject(MESSAGE, 0, 48, 360, 480, 0, "THE ANGRY FEZ HAS WON!", 0, 0);
						AddObject(MESSAGE, 0, 56, 240, 480, 0, "GOOD LUCK NEXT TRY!", 0, 0);
						AddObject(MESSAGE, 0, 80, 360, 720, 0, "THANK YOU FOR PLAYING!", 0, 0);
						waitforkey[0] = waitforkey[1] = 480;
						shipwait = 600;

						gameover = 1;
						break;
					case GAMESTATE_NEXTLEVEL:
						for (int i = 0; i < MAXPLAYERS; i++)
							shots[i] = 0;
						++level;
						NewGame(level);
						break;
					default:
						break;
				}
			}
			if (shipwait > 0)
				--shipwait;
			if (powerupframe > 0)
				--powerupframe;
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				if (score[i] > highscore)
					highscore = score[i];
			}
			if (gameover == 1)
			{
				PrintMessage("GAME OVER!", 88, 112, TEXT_RED);
				if ((jaction[0]) || (shipwait == 0))
				{
					waitforkey[0] = waitforkey[1] = 360;
					gamestate = SCORES;
					powerupframe = 0;
					shipwait = 0;
					leftmonsters = 0;
					level = 0;
					gameover = 0;
					operational = 0;
					return;
				}
			}
			return;
		}
		break;

	case TITLE:
	case SCORES:
		if (operational == 0)
		{
			NewGame(-1);
			operational = 1;
			if (gamestate == TITLE)
				audio_music("title.xm");
		}

		if (done == 0 && operational == 1)
		{
			for (increment = 0; increment < MAXSHIPPY; ++increment)
			{
				DoAi(increment);
			}
			if (shipwait > 0)
				--shipwait;

			if (jdiry[0] > 0)
				SYSTEM_SETMODE(0);
			else if (jdiry[0] < 0)
				SYSTEM_SETMODE(1);
			if (jaction[0] || players[0] || players[1])
			{
				if (gamestate == SCORES)
				{
					gamestate = TITLE;
					powerupframe = 0;
					shipwait = 0;
					leftmonsters = 0;
					level = 0;
					gameover = 0;
					operational = 0;
					waitforkey[0] = waitforkey[1] = 30;
					return;
				}
				else
				{
					StartGameState();
				}
			}
			if (shipwait == 0)
			{
				shipwait = 0;
				if (gamestate == TITLE)
				{
					gamestate = SCORES;
					operational = 0;
					powerupframe = 0;
					shipwait = 0;
					leftmonsters = 0;
					level = 0;
					gameover = 0;

				}
				else
				{
					for (increment = 0; increment < 14; ++increment)
					{
						if (winners[increment].level == -1)
							winners[increment].level = 0;
					}
					StartGameState();
					gamestate = DEMO;
					operational = 1800;
					powerupframe = 0;
					shipwait = 0;
				}
			}
		}
		break;
	case DEMO:
		for (int i = 0; i < MAXPLAYERS; i++)
		{
			if (score[i] > extralife[i])
			{
				ShippyObjects[i].lives++;
				extralife[i] *= 2;
				audio_play("fanfare.wav");

			}
		}

		if (leftmonsters > 0 && done == 0 && gameover == 0 && operational > 0)
		{
			++timeattack;
			for (increment = 0; increment < MAXSHIPPY; ++increment)
			{
				DoAi(increment);
			}
			if (gameover == 0)
			{
				int over = 2;
				int initials = 1;
				for (int i = 0; i < MAXPLAYERS; i++)
				{
					if (ShippyObjects[i].special != SHIPPY_SPECIAL_GAMEOVER)
					{
						if ((over > 0) && ((ShippyObjects[i].special == SHIPPY_SPECIAL_INITIAL) || (ShippyObjects[i].health < 0)))
							over = 1;
						else
							over = 0;
						if (ShippyObjects[i].special != SHIPPY_SPECIAL_INITIAL)
							initials = 0;
					}
				}
				if ((over == 1) && (initials == 0))
				{
					diedlast = 1;
					for (int i = 0; i < MAXPLAYERS; i++)
					{
						if (ShippyObjects[i].health < 0)
						{
							++ShippyObjects[i].health;
							if (ShippyObjects[i].health >= -1)
							{
								ShippyObjects[i].health = 10;
								NewGame(level);
							}
						}
					}
				}
				else if (over == 2)
				{
					AddObject(MESSAGE, 0, 48, 360, 480, 0, "THE ANGRY FEZ HAS WON!", 0, 0);
					AddObject(MESSAGE, 0, 56, 240, 480, 0, "GOOD LUCK NEXT TRY!", 0, 0);
					AddObject(MESSAGE, 0, 80, 360, 720, 0, "THANK YOU FOR PLAYING!", 0, 0);
					waitforkey[0] = waitforkey[1] = 480;
					shipwait = 600;

					gameover = 1;
				}
			}
			if (shipwait > 0)
				--shipwait;
			if (powerupframe > 0)
				--powerupframe;
			for (int i = 0; i < MAXPLAYERS; i++)
			{
				if (score[i] > highscore)
					highscore = score[i];
			}
			--operational;
			if (jaction[0] || players[0] || players[1])
			{
				operational = 0;
				waitforkey[0] = waitforkey[1] = 30;
			}
			if (jdirx[0] > 0)
				SYSTEM_SETMODE(1);
			else if (jdirx[0] < 0)
				SYSTEM_SETMODE(0);
			return;
		}
		if (gameover == 0)
		{
			for (int i = 0; i < MAXPLAYERS; i++)
				shots[i] = 0;
			++level;
			NewGame(level);
		}
		if (operational <= 0)
		{
			gamestate = SPLASH;
			powerupframe = 0;
			leftmonsters = 0;
			level = 0;
			gameover = 0;
			operational = 30;
			NewGame(-2);
			audio_music(NULL);
		}
		break;
	}
}

static void show_usage(char *name, FILE * f)
{
	fprintf(f,
					"Usage: %s [-w] [-a] [-h]\n\n"
					"Options:\n"
					"-w\tWindowed, start in a window (default fullscreen)\n"
					"-a\tArcade mode\n" "-h\tHelp, display this help and exit\n", name);
}

int SHIPPY_MAIN(int argc, char *argv[])
{
	int i;
	char *saveDir = get_user_path();
	char *saveName = NULL;
	int len = strlen(saveDir);
	saveName = (char *)malloc(len + 100);
	strcpy(saveName, saveDir);
	strcat(saveName, "shippy.hs");
	highscore_fp = fopen(saveName, "r+");
#ifdef __unix__
	if (highscore_fp == NULL)
	{
		highscore_fp = fopen("/usr/share/shippy/shippy.hs", "r");
	}
#endif
	RestoreHS();
	if (highscore_fp != NULL)
	{
		fclose(highscore_fp);
	}
	highscore_fp = fopen(saveName, "w+");
	StoreHS();
	rewind(highscore_fp);
#ifdef GAMERZILLA
	GamerzillaStart(false, saveDir);
	{
		char fname[1024];
		strcpy(fname, get_data_path());
		strcat(fname, "gamerzilla/shippy.game");
		game_id = GamerzillaSetGameFromFile(fname, get_data_path());
	}
#endif
	InitCutScenes();

	for (i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-w"))
			start_windowed = 1;
		else if (!strcmp(argv[i], "-a"))
		{
			use_arcade_mode = 1;
			screen_width = 480;
			screen_height = 320;
		}
		else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "-?") || !strcmp(argv[i], "-help") || !strcmp(argv[i], "--help"))
		{
			show_usage(argv[0], stdout);
			return 0;
		}
		else
		{
			fprintf(stderr, "Error: unknown argument: %s\n", argv[i]);
			show_usage(argv[0], stderr);
			return 1;
		}
	}

	srand(time(0));
	if (SYSTEM_INIT() != 0)
		return 1;
	InitShippy();
	while (done == 0)
	{
		SYSTEM_IDLE();
		if (objectsynch > 0)
		{
			SYSTEM_POLLINPUT();
			audio_exec();
			ExecShippy();
			--objectsynch;
			++countframes;
			if (countframes >= 60)
			{
				fps = frames;
				frames = 0;
				countframes = 0;
			}

			for (i = 0; i < MAXSHIPPY; ++i)
			{
				RenderShippy(i);
			}
			DrawOverlay();
			SYSTEM_FINISHRENDER();
			++frames;
		}
	}
	StoreHS();
#ifdef GAMERZILLA
	GamerzillaQuit();
#endif
	SYSTEM_CLEAN();
	return 0;
}
