Ryan Konkul
rkonku2
Homework 1: statsh

To compile and run the program enter

make -f statshMake statsh
./statsh

The optional enhancements I implemented were:
-Implemented cd and mkdir (pwd is not a shell specific syscall)
-Allow listing of environment variables. Entered as
envvar
-stats command allows for a -f flag to display full statistics about the executed processes. Entered as
stats -f

I was able to implement all of the steps and a few optional enhancements. I have not had the chance to do extensive bug testing, but most simple commands work fine.

My program opens with getting a line from the user using getline(). This allows a very large input size to be read in. There is a limit to the size of the arguments. I allocated 1024 bytes for each argument within each piped command. After getting a string from the user, I send it off to parse_input() which handles creating a vector<char**> which is a list of commands. Each command is divided by a pipe symbol. parse_input() uses parse_line() to create a vector<char*> and then calls parse_cmd() on each commands to split it up from its arguments. parse_cmd() uses strtok to split up the arguments.

Once the arguments are split, I first check for an & to see if the children should be background processes. If there is one, I remove it and enter a loop which creates pipes for the children. I check the first and last commands for file io redirection after preparing the pipe. If a < or > is found, a file descriptor is opened on the file and the functions return the command arguments without the > file.txt part. 

Before getting a line from the user, the program checks if any background processes need to be waited for. After executing the commands, the stats are temporarily stored in either cur_command_stats or background_processes depending on the command. After they are waited for, final stats are stored in history. If a process could not be executed or returned an error code, it is not stored in history. 

As the program exits, time elapsed is calculated and displayed. 

