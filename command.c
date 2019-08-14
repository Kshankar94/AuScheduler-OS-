
#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include "command.h"


/*Help menu*/
static const char *helpmenu[] = {
	"[run] <job> <time> <priority>       ",
	"[quit] Exit AUbatch                 ",
	"[help] Print help menu              ",
	"[list] display the job status       ",
	"[fcfs] change the scheduling policy to fcfs ",
	"[sjf] change the scheduling to sjf",
	"[priority] change the scheduling policy to priority",
        /* Please add more menu options below */
	NULL
};


int cmd_helpmenu(int n, char **a)
{
	(void)n;
	(void)a;

	showmenu("AUbatch help menu", helpmenu);
	return 0;
}

