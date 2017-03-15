//Benjamin Sarkis
//Anthony Ratte

#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

int main(int argc, char *argv[]){
	struct timeval start, end, backend; //These will hold the timestamps.
	struct rusage stats; //This will hold the usage stats.
	long fault, reclaim; //Report faults incurred in the program.
	long lastfault = 0; //Running tally of faults.
	long lastreclaim = 0;
	int ms = 0;
	int i; //All these letters are for loops.
	int j;
	int k;
	int l;
	int done; //Controller variables for background tasks. Not as related as their name implies. This one checks if we're done storing the background task in an array.
	int doneid; //The PID of the background task that just completed. 0 if one didn't just complete.
	int defined = 2; //Number of highest legal command
	//The input character	
	char *input = (char*) malloc(10);
	//Arguments to the ls command
	int argslim = 50;	
	char *lsargs = (char*) malloc(argslim*sizeof(char));
	//Variables related to the LS command and/or max size of arguments and filepaths.
	int pathlim = 500;
	char *patharg = (char*) malloc(pathlim*sizeof(char));
	char *dirarg = (char*) malloc(pathlim*sizeof(char));
	char dirret[500];
	char *lscoms[3];
	//Variables related to storing background tasks. These arrays get filled as they're scheduled and cleaned out as they finish.
	int bgpids[100] = {0};
	int bgid = 0;
	char *bgnames[100] = {0};
	struct timeval bgtimes[100];
	
	//This double array of character pointers will be how we store
	// user made commands
	//Rows:  Signify a new command
	//Coulumns:  Represents arguments of each command, the 0th column is the command itself.
	//Each spot contains a string, which is a char pointer.
	char *allCommands[32][32];
		
	//An array of just the strings to be printed on the initial prompt
	char *displayNames[32];
	//An array that states whether the corresponding user-defined function is background-enabled.
	int isBackground[32];

	//Buffer for reading new user made commands
	int bufferLimit = 128;	
	char *inbuff = (char*) malloc(bufferLimit * sizeof(char));
	ssize_t newUserCommand;
	char *tokbuff = (char*) malloc(170);
	
	//When we use a user defined command, this variable holds the filenames and arguments for passing into execvp.
	char *selected = (char*) malloc(170);
	char *selectedFilePath = (char*) malloc(170);
	char *wew = (char*) malloc(170);
	
	//Main loop
	while(1){
		int r = 0;
		int c = 0;
		printf("===== Command Antics =====\n");
		printf("What command would you like to run?\n");
		printf("	0. whoami   : Prints out the result of the whoami command\n");
		printf("	1. last   : Prints out the result of the last command\n");
		printf("	2. ls   : Prints out the result of the ls command\n");
		while(c < (defined - 2)){ //display all user added commands
			printf("	%d. %s", (c + 3), displayNames[c]);
			c++;
		} 
		printf("	a. add command: Adds a new command to the menu\n");
		printf("	c. change directory: Changes process working directory\n");
		printf("	e. exit: Leave Mid-day Commander\n");
		printf("	p. pwd: Prints working directory\n");
		printf("	r. Show all background tasks.\n");
		printf("Option?: ");

		//User specifies which command they want
		getline(&input,&pathlim,stdin);
		input[strlen(input)-1] = 0;
		
		if(atoi(input) > defined){ //Gave a number input that isn't defined yet. defined++ when a is used!
			fprintf(stderr, "No function defined for given input yet, exiting.\n");
			exit(1);
		}
		if(input[0] == '2' && input[1] == '\0'){ //LS
			printf("-- Directory Listing --\n");
			printf("Arguments?: ");
	
			getline(&lsargs, &argslim, stdin); 
			printf("Path?: ");
	
			getline(&patharg, &pathlim, stdin);
			patharg[strlen(patharg)-1] = 0; //remove newline from input.
		}
		if(input[0] == 'a'){
			//All of our user defined commands are stored in a 2D array of strings (which is to say a 3d array of chars).
			//A row is a single user-made command. Every column is an argument in a command.
			//Extra columns are NULL if fewer than the max args are supplied.
			///using getline so we don't have to malloc a buffer
			printf("--Add a command--\n");
			printf("Command to add?\n");
			memset(inbuff, '\0', sizeof(inbuff)); //I really wanted this buffer cleared...
			//jury-rigged ignore command
			//newUserCommand = getline(&inbuff, &bufferLimit, stdin);
			newUserCommand = getline(&inbuff, &bufferLimit, stdin);
			//if (-1 != newUserCommand) {
			//	puts(inbuff); //I don't know why I thought this would help.
			//}
			if(inbuff == NULL){
				fprintf(stderr, "Unable to allocate buffer, exiting\n");
				exit(1);
			}
			defined = defined + 1; //Up the tally of defined functions.
			//Store what will be seen in initial prompt
			
			displayNames[defined - 3] = (char *)malloc(170);
			strcpy(displayNames[defined - 3], "User added command : ");
			strcat(displayNames[defined - 3], inbuff); //displaynames is stored seperately from the actual commands, just a plain string to show the user.
			inbuff[strlen(inbuff)-1] = 0; //again, dropping off a newline
			//The start of the next row is the new command
			j = strlen(inbuff);
			while(!isBackground[defined-3] && j >= 0){ //Look for ampersand, delete if found and enable for background.
				if (inbuff[j] == '&'){
					inbuff[j] = 0;
					isBackground[defined-3] = 1;
				}
				j--;
			}
			allCommands[defined-3][0] = (char *)malloc(170);
			strcpy(allCommands[defined-3][0], strtok(inbuff, " ")); //Command goes in the first column.
			//Each subsequent strtok is a potential argument to the user command
			for(int i = 1; i < 32 && inbuff != NULL; i++){ //Shove further arguments into later columns in the same row. If there aren't any more, fill the column with NULL. Do note that strcpy and NULL don't get along well...
				//tokbuff
				allCommands[defined-3][i] = (char *)malloc(170);
				tokbuff = strtok(NULL, " ");
				//printf("tokbuff = %s\n", tokbuff);
				if (tokbuff == NULL){
					allCommands[defined-3][i] = NULL;
				}				
				else {
					strcpy(allCommands[defined-3][i], tokbuff);
				}	
			}
			printf("Okay, added with ID %d!\n",defined);
		}
		if(input[0] == 'c'){ //change directory
			printf("New Directory?: ");
			getline(&dirarg,&pathlim,stdin);
			dirarg[strlen(dirarg)-1] = 0;
			chdir(dirarg);
		}
		if(input[0] == 'r'){ //Print all background tasks. If bgpids[l] != 0, that implies there is a background task running with ID l.
			printf("Background Tasks Running:\n");
			for(l = 0; l < 100; l++){
				if(bgpids[l]) printf("%s is running with PID %d.\n", bgnames[l], bgpids[l]);
			}
		}
		if(input[0] == 'p'){ //Print the working directory.
			printf("Current Working Directory: \n");
			getcwd(dirret, 500);
			printf("%s\n", dirret);
		}
		if(input[0] == 'e' || input[0] == EOF || input[1] == EOF){ //Exit. Ignore the EOF checks, they didn't work...
			printf("Checking for any running background tasks...\n");
			for(l = 0; l < 100; l++){ //If there are background tasks, don't exit until they finish. Check the PID of the background task and wait for that one specifically! Do this for every task. If out of order, they'll just cascade as their turn to be waited on reaches them.
				if(bgpids[l]) printf("%s is running with PID %d. Will close when finished...\n", bgnames[l], bgpids[l]);
				waitpid(bgpids[l],NULL,0);
			}
			printf("Logging you out, Commander. \n");
			exit(0);
		}
		if (feof(stdin)){ //How to actually check for EOF...
			for(l = 0; l < 100; l++){
				if(bgpids[l]) printf("%s is running with PID %d. Will close when finished...\n", bgnames[l], bgpids[l]);
				waitpid(bgpids[l],NULL,0);
			}
			printf("Logging you out, Commander. \n");
			exit(0);
		}
			
		int rc = fork(); //Fork off a child. Even if calling a parent-executed function, the child will immediately exit.

		if (rc < 0) {		//fork failed
			fprintf (stderr, "fork failed\n");
		}
		else if (rc == 0){ //child process, exec will depend on what inputs are entered
			if (input[0] == '0' && input[1] == '\0'){ //exec whoami
				execl("/bin/sh","sh","-c", "whoami", (char*)0);
			}
			else if (input[0] == '1' && input[1] == '\0'){  //exec last
				execl("/bin/sh", "sh", "-c", "last", (char *)0);
			}

			else if (input[0] == '2' && input[1] == '\0'){	//exec ls
				//pipe lsargs and path arg into a variable
				lsargs[strlen(lsargs)-1] = 0;
				lscoms[0] = lsargs;
				lscoms[1] = patharg;
				lscoms[2] = NULL;
				execvp("ls", lscoms); 
			}
			//For all user defined commands
			else if ((atoi(input) > 2) && (atoi(input) <= defined)){
				selected = allCommands[atoi(input) - 3][0];
				//Need to account for all possible 31 command arguments
				execvp(selected, allCommands[atoi(input) - 3]); 
			}
			else exit(0); //I have nothing to do!
		}
		else{ //the parent process
			if(isBackground[atoi(input)-3]){ //We need different action if we just ran a background process.
				done = 0;
				while(!done){ //Keep looking for a spot in the array to put our background process.
					if(bgpids[bgid] == 0) {
						bgpids[bgid] = rc; //The PID goes in the first empty slot.
						gettimeofday(&bgtimes[bgid], NULL); //Record roughly when we started running.
						bgnames[bgid] = (char *) malloc(170); //Put the name in the same index.
						strcpy(bgnames[bgid],displayNames[atoi(input-3)]);
						printf("Okay, running with background ID %d (PID %d)\n", bgid, bgpids[bgid]);
						printf("KNOWN BUG: All background processes use the name of the first-defined one. They all function as they should, though...\n\n");
						if (bgid == 100) bgid = 0; //Rotate the index.
						done = 1;
					}
				bgid++; //I realize this leads to a rather unreliable ID system, but I guess that's what we have individual PIDs for...
				}		
			}
			else {
				gettimeofday(&start, NULL); //Timestamp when we started our command.
				waitpid(rc, NULL, 0); //Don't go anywhere until a child changes state. Make sure it's the one we just ran and not a background process!
				gettimeofday(&end, NULL); //Timestamp when we ended our command.
				getrusage(RUSAGE_CHILDREN, &stats); //This will get usage of every terminated child we waited for.
				fault = (stats.ru_majflt - lastfault);
				reclaim = (stats.ru_minflt - lastreclaim);
				ms = (1000000*(end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec));

				//Print out the stats. Use fault and reclaim, last- versions keep track of prior usages
				printf("\n-- Statistics --\n");
				printf("Elapsed Time: %d microseconds\n", ms);
				printf("Page Faults (Reclaimed): %ld \n", reclaim);
				printf("Page Faults: %ld \n\n", fault);
				lastfault += fault; //Keep a running tally of all the faults so we don't double-count them.
				lastreclaim += reclaim;
			}
			doneid = wait3(NULL, WNOHANG, &stats); //Check if a background process is done.
			if (doneid){
				for(k = 0; k < 100; k++){
					if(bgpids[k] == doneid){
						printf("\n-- Job Complete [%d] -- \n", k);
						printf("Background %s (PID: %d) has finished.\n", bgnames[k], bgpids[k]);
						bgpids[k] = 0; //Clean out the PID once the task is done. This indicates the task is no longer running.
						free(bgnames[k]); //Don't want to malloc twice
						gettimeofday(&backend, NULL); //Timestamp when we ended our command. Can't reuse the other one otherwise we mess it up.
						fault = (stats.ru_majflt); //wait3 only returns the single child's faults. Don't blame him for his siblings' wrongdoings!
						reclaim = (stats.ru_minflt);
						ms = (1000000*(backend.tv_sec - bgtimes[k].tv_sec) + (backend.tv_usec - bgtimes[k].tv_usec));
						//Print out the stats. Use fault and reclaim, last- versions keep track of prior usages
						printf("\n-- Background Job Statistics --\n");
						printf("Elapsed Time: %d microseconds (inc. wait time to show)\n", ms);
						printf("Page Faults (Reclaimed): %ld \n", reclaim);
						printf("Page Faults: %ld \n\n", fault);
						lastfault += fault; //It's still a child so its faults count against the collective.
						lastreclaim += reclaim;
					}
				}
			} 
		}
	}
}

