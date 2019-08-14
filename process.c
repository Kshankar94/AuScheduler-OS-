/*
 * This program will be launched in fork_execv
 
 */
//#include <stdio.h> 
//#include <stdlib.h>

#include <signal.h>
#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

int main(int argc, char *argv[])
{
  sleep(atoi(argv[1]));
}
