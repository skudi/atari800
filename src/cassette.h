#include <stdio.h>		/* for FILE and FILENAME_MAX */

#include "atari.h"		/* for UBYTE */

#define CASSETTE_DESCRIPTION_MAX 256

void CASSETTE_Initialise(int *argc, char *argv[]);

int CASSETTE_CheckFile(char *filename, FILE **fp, char *description, int *last_block, int *isCAS);
int CASSETTE_Insert(char *filename);
void CASSETTE_Remove(void);
extern char cassette_filename[FILENAME_MAX];
extern char cassette_description[CASSETTE_DESCRIPTION_MAX];

extern int cassette_current_block;
extern int cassette_max_block;

extern int hold_start;
extern int press_space;

UBYTE CASSETTE_Sio(void);
extern UBYTE cassette_buffer[4096];