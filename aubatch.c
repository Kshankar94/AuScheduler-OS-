/*
 *
 * <Our professor, Dr. Qin, gave us the code structure for cmd_dispatch, menu_execute, show menu and executor methods. Other than those, I implemented the FIFO, SJF and Priority algorithms, and other methods. > 
 * Xiao Qin
 * Department of Computer Science and Software Engineering
 * Auburn University
 *
 * Compilation Instruction:
 * gcc commandline_parser.c -o commandline_parser
 * ./commandline_parser
 *
 */

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

/* Error Code */
#define EINVAL       1
#define E2BIG        2

#define MAXMENUARGS  8 
#define MAXCMDLINE   64 

void menu_execute(char *line, int isargs); 

void compare();
void performance_eval();
int compare_func();
int swap();
pthread_t cmdid;
pthread_t execid;
char *policy_name="fcfs";
pthread_mutex_t cmd_queue_lock;  /* Lock for critical sections */
pthread_cond_t cmd_buf_not_full; /* Condition variable for buf_not_full */
pthread_cond_t cmd_buf_not_empty;

u_int buf_head = 0;
u_int buf_tail = 0;
u_int count = 0;
int total = 0;
double wait_time = 0;
double turn_time = 0;
double cpu_time = 0;
int status;
/*struct job{
	const char *name;
	int time;
	int priority;
	
};*/
struct {
	char *name;
	int time;
	int priority;
	time_t arriv;
	time_t start;
	time_t end;
	
} job[100], temp;

/*
 * The list command - displays statistics of jobs.
 */
int cmd_list(int nargs, char **args){
	int i=0;
	printf("Scheduling policy:%s\n",policy_name);
	printf("Name  CPU_Time  Pri\n");	
	while(buf_tail+i<buf_head){
		printf(":  %d     %d\n", job[buf_tail+i].time, 			job[buf_tail+i].priority);	
		i++;	
	}
	return 0;
}



/*
 * The quit command.
 */
int cmd_quit(int nargs, char **args) {
	printf("Please display performance information before exiting AUbatch!\n");
	double avg_turn_time = turn_time/total;
	double avg_wait_time = wait_time/total;
	double avg_cpu_time =  cpu_time/total;
	double throughput = 1/avg_turn_time;
	printf("Total number of submitted jobs:%d\n", total);
	printf("Average turn time:%f\n", avg_turn_time);
	printf("Average wait time:%f\n", avg_wait_time);
	printf("Average cpu time:%f\n", avg_cpu_time);
	printf("Throughput:%f\n", throughput);
	pthread_cancel(cmdid);
	pthread_cancel(execid);
		
        pthread_exit(NULL);
}

/*
 * The run command - submit a job.
 */
int cmd_run(int nargs, char **args) {
	if (nargs != 4) {
		printf("Usage: run <job> <time> <priority>\n");
		return EINVAL;
	}
        
        /* Use execv to run the submitted job in AUbatch */
        printf("use execv to run the job in AUbatch.\n");
        //printf("%s\n",args[3]);
int i;	
for(i=1; i<nargs; i++){
pthread_mutex_lock(&cmd_queue_lock);
while (count == 100) {

            pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
        }
if(i==1)
{
job[buf_head].name = args[1];}
if(i==2){job[buf_head].time = atoi(args[2]);}
if(i==3){job[buf_head].priority = atoi(args[3]);} 
pthread_mutex_unlock(&cmd_queue_lock);
} 

time_t T1=time(NULL);
job[buf_head].arriv = T1;
//sort the struct before sending it to dispatcher

buf_head+=1;
count+=1;
if(strcmp(policy_name,"fcfs")!=0){
compare(count, policy_name);
}
total++;
pthread_cond_signal(&cmd_buf_not_empty);
return 0; /* if succeed */
}

/*
 * Display menu information
 */
void showmenu(const char *name, const char *x[])
{
	int ct, half, i;

	printf("\n");
	printf("%s\n", name);
	
	for (i=ct=0; x[i]; i++) {
		ct++;
	}
	half = (ct+1)/2;

	for (i=0; i<half; i++) {
		printf("    %-36s", x[i]);
		if (i+half < ct) {
			printf("%s", x[i+half]);
		}
		printf("\n");
	}

	printf("\n");
}

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


/* command policy*/
int cmd_policy(int nargs, char **args){
policy_name = args[0];
printf("The scheduling policy is switched to %s", policy_name);
printf("All the %d ", count);
printf(" waiting jobs have been rescheduled");
}

/*command test*/
int cmd_test(int nargs, char **args){
	if (nargs != 7) {
		printf("Usage: test <benchmark> <policy> <num_of_jobs> <priority_levels> <min_CPU_time> <max_CPU_time>\n");
		return EINVAL;
	}
	char *benchmark=args[1];
	policy_name=args[2];
	int jobs=atoi(args[3]);
	int priorities=atoi(args[4]);
	int cpu_min_time=atoi(args[5]);
	int cpu_max_time=atoi(args[6]);
	int range= cpu_min_time- cpu_max_time;
	for(int k=0;k<jobs;k++){
		pthread_mutex_lock(&cmd_queue_lock);
        	while (count == 100) {
            		pthread_cond_wait(&cmd_buf_not_full, &cmd_queue_lock);
        	}
        	job[buf_head].name="./process";
		job[buf_head].time=(rand()%(range))+cpu_min_time;
		job[buf_head].priority=(rand()%priorities);
		job[buf_head].start=clock();
		count++;
		buf_head++;
		if(strcmp(policy_name,"fcfs")!=0){
             compare(count, policy_name);
              }
        	if (buf_head == 100){
        	    buf_head = 0;
		}

		pthread_cond_signal(&cmd_buf_not_empty);  
        	/* Unlok the shared command queue */
        	pthread_mutex_unlock(&cmd_queue_lock);

	}
}

/*
 *  Command table.
 */
static struct {
	const char *name;
	int (*func)(int nargs, char **args);
} cmdtable[] = {
	/* commands: single command must end with \n */
	{ "?\n",	cmd_helpmenu },
	{ "h\n",	cmd_helpmenu },
	{ "help\n",	cmd_helpmenu },
	{ "r",		cmd_run },
	{ "run",	cmd_run },
	{ "q\n",	cmd_quit },
	{ "quit\n",	cmd_quit },
	{ "list\n",     cmd_list},
	{ "fcfs\n",  	cmd_policy},
	{ "sjf\n", 	cmd_policy},
	{ "priority\n", cmd_policy},
	{ "test",       cmd_test},
        /* Please add more operations below. */
        {NULL, NULL}
};




/*sorting algorithm for sjfs and priority*/
void compare(int struct_size, char *policy)
{
int select;
if(strcmp(policy, "sjf")==0){
policy_name = "sjf";
select = 1;
}
if(strcmp(policy, "priority")==0){
policy_name = "priority";
select = 2;
}
int i; int j;
  for (i = 1; i < struct_size; i++){
      for (j = 0; j < struct_size - i; j++) {
	if(select ==1){
		if(compare_func(job[j].time, job[j+1].time)== 1){
     	            swap( j, j+1);  
             }
          }
	if(select == 2){
		if(compare_func(job[j].priority, job[j+1].priority)== 1){
     	            swap( j, j+1);  
             }
}
}
}
}


int compare_func(int a, int b){
if(a<b){
return -1;
}
else if(a>b){
return 1;
}
return 0;
}

int swap(int num1, int num2){
temp = job[num1];
job[num1] = job[num2];
job[num2] = temp;

}

/*dispatcher thread*/
void *executor(void *ptr) {
    char *message;
    u_int i;

    message = (char *) ptr;
    printf("%s \n", message);
        /* lock and unlock for the shared process queue */
        printf("In executor: count = %d\n", count);
	for (int i =0; i<count+1; i++){
	pthread_mutex_lock(&cmd_queue_lock);
        sleep(2);
	while (count == 0) {
            pthread_cond_wait(&cmd_buf_not_empty, &cmd_queue_lock);
        }
	printf("%s\n", "success");
        /* Run the command scheduled in the queue */
        count--;
 	i--;
        //job[buf_tail].start = clock();
	time_t T=time(NULL);
	job[buf_tail].start = T;
	/*struct tm tm = *localtime(&T);
	sprintf(job[buf_tail].start,"%d:%d:%d",tm.tm_hour, tm.tm_min, tm.tm_sec);*/
	
	pid_t forked = fork();
		
		if (forked==0){	
	char wait[10];
	sprintf(wait, "%d", job[buf_tail].time);
	execv("./process",(char*[]){"./process",wait, NULL});

		}
           if ((forked = wait(&status)) == -1){
		
		
           }
else{
		
        performance_eval(job[buf_tail].arriv, job[buf_tail].start, job[buf_tail].end, job[buf_tail].time);  
        /* Move buf_tail forward, this is a circular queue */ 
        buf_tail++;
        if (buf_tail == 100)
            buf_tail = 0;

        pthread_cond_signal(&cmd_buf_not_full);
        /* Unlok the shared command queue */
        pthread_mutex_unlock(&cmd_queue_lock);
}
	}
        	
    pthread_exit(NULL);
}


/*performance metrics*/
void performance_eval(time_t arriv, time_t start, time_t end, int tim){
time_t T2=time(NULL);
end = T2;
cpu_time+=tim;
double turn_diff = (double)((end) - (arriv));

turn_time+=abs(turn_diff);
double wait_diff = (double)((turn_time)-(tim));
wait_diff = (wait_diff)/(CLOCKS_PER_SEC);
wait_time+=abs(wait_diff);
}


/*
 * Process a single command.
 */
int cmd_dispatch(char *cmd)
{
	time_t beforesecs, aftersecs, secs;
	u_int32_t beforensecs, afternsecs, nsecs;
	char *args[MAXMENUARGS];
	int nargs=0;
	char *word;
	char *context;
 	int i, result;

	for (word = strtok_r(cmd, " ", &context);
	     word != NULL;
	     word = strtok_r(NULL, " ", &context)) {

		if (nargs >= MAXMENUARGS) {
			printf("Command line has too many words\n");
			return E2BIG;
		}
		args[nargs++] = word;
	}

	if (nargs==0) {
		return 0;
	}

	for (i=0; cmdtable[i].name; i++) {
		if (*cmdtable[i].name && !strcmp(args[0], cmdtable[i].name)) {
			assert(cmdtable[i].func!=NULL);
            
            /*Qin: Cal	l function through the cmd_table */
			result = cmdtable[i].func(nargs, args);
			return result;
		}
	}

	printf("%s: Command not found\n", args[0]);
	return EINVAL;
}

void *command_line(void *ptr)
{
	
	char *buffer;
        size_t bufsize = 64;
        
        buffer = (char*) malloc(bufsize * sizeof(char));
        if (buffer == NULL) {
 		perror("Unable to malloc buffer");
 		exit(1);
	}
 
    while (1) {
		printf("> [? for menu]: ");
		getline(&buffer, &bufsize, stdin);
		cmd_dispatch(buffer);
	}
     
}
/*
 * Command line main loop.
 */
int main()
{
	//count = 0;
	//buf_head = 0;
	//buf_tail = 0;
	char *message = "command_line"; 
	char *message2 = "Executor Thread";
        int id1;
	int id2;
	id1 = pthread_create(&cmdid,NULL,command_line,(void*) message);
        id2 = pthread_create(&execid, NULL, executor, (void*) message2);
	pthread_mutex_init(&cmd_queue_lock, NULL);
    	pthread_cond_init(&cmd_buf_not_full, NULL);
    	pthread_cond_init(&cmd_buf_not_empty, NULL);
	pthread_join(cmdid, NULL);
        pthread_join(execid, NULL);
	return 0;

	
}



