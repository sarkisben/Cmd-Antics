#Cmd-Antics
Anthony Ratte & Benjamin Sarkis

This is Project One for CS3013: operating Systems.

This program is based off the typical three state scaffolding used for the forking processes, one for the error, one for the parent process, and one for the child process.

We began our implementation by creating a user prompt that told the user to enter one of three commands:
   0 -> Prints out the result of the whoami command
   1 -> Prints out the result of the last command
   2 -> Prints out the result of the ls command and a user-specified path
It also prints out statistics of the elapsed time, the Page Faults, and the reclaimed page faults. 

We then added the commands of a, c, e, and p as well as a while loop to print out additional user commands in the initial prompt.  "a" adds a new command created by the user. "c" changes the process working directory to a new directory specified by the user. "e" exits the terminal and terminates the shell. "p" prints the results of pwd. There are two key arrays we use.  One is a double array of type char* where each row is a command and each column (besides column 0) is an argument to that command.  The second key array is the displayNames array of type char*.  Execvp is used to execute all processes initiated by user created commands.  Strtok is used to parse out user created commands and command arguments.

In addition, we added the r command, which is appended to the end of the initial prompt. The r command lists all the active background processes. An array of integers was created, signifying if a command is defined as a background process.  When adding a command that acts as a background task, the ampersand is truncated to avoid errors down the road, but is still fully recorded in the initial prompt.  The EOF condition is modified to wait for background processes to close when triggered.  
	In the parent process, if the command invoked is a background process, then the value of the fork is saved in an array of background process IDS. Background process IDs are marked on a rotating index from 0 to 100. These are coordinated with names and other info.  Arrays are cleard as tasks are finished, and populated as tasks are scheduled.
	At the end of the main while-loop, wait3 is called and saved as a temporary ID.  If the ID is not zero, then all the relvant information of a finished background process is returned.  We record the fault and reclaim information using the resource statistics struct.  
