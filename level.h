#ifndef LEVEL_H
#define LEVEL_H

/*---------------------------------------------------------------------------*/

#define MAXSTR 256
#define MAXLVL 26
#define MAXNAM 8

#define DEFAULT_NAME "Player"

extern char player[];

const char *level_coin_n(int, int);
const char *level_coin_c(int, int);
const char *level_coin_s(int, int);

const char *level_time_n(int, int);
const char *level_time_c(int, int);
const char *level_time_s(int, int);

void level_init(const char *, const char *, const char *);
void level_free(const char *);

int  level_exists(int);
int  level_opened(int);

int  curr_count(void);
int  curr_score(void);
int  curr_coins(void);
int  curr_balls(void);
int  curr_level(void);

void level_goto(int, int, int, int);
int  level_goal(void);
int  level_pass(void);
int  level_fail(void);

void level_score(int);
void level_snap(int);
void level_shot(int);
void level_song(void);

/*---------------------------------------------------------------------------*/

#endif
