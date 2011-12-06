//============================================================================
// Name        : statsh.cpp
// Author      : Ryan Konkul
// Version     : CS385
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>

using namespace std;

const int READ_END = 0;
const int WRITE_END = 1;
const int STD_OUT = 1;//file descriptors for stdin and out
const int STD_IN = 0;

//Class to store stats of a command
class Command_stats {
    public:
    char* name;
    double user_time;
    double system_time;
    pid_t pid;
    long maxrss; long ixrss; long idrss; long isrss; long minflt;
    long majflt; long nswap; long inblock; long oublock; long msgsnd;
    long msgrcv; long nsignals; long nvcsw; long nivcsw;
    Command_stats(char* name_, pid_t pid_, double user_time_, double system_time_) {
        name = name_; pid = pid_; user_time = user_time_;
        system_time = system_time_;
        maxrss=0;ixrss=0;idrss=0;isrss=0;minflt=0;majflt=0;nswap=0;inblock=0;
        oublock=0;msgsnd=0;msgrcv=0;nsignals=0;nvcsw=0;nivcsw=0;
    }
    Command_stats(char* name_) {
        name = name_; pid = -1; user_time = -1; system_time = -1; maxrss = 0;
        maxrss=0;ixrss=0;idrss=0;isrss=0;minflt=0;majflt=0;nswap=0;inblock=0;
                oublock=0;msgsnd=0;msgrcv=0;nsignals=0;nvcsw=0;nivcsw=0;
    }
    Command_stats(char* name_, pid_t pid_) {
        name = name_; pid = pid_; user_time = -1; system_time = -1; maxrss = 0;
        maxrss=0;ixrss=0;idrss=0;isrss=0;minflt=0;majflt=0;nswap=0;inblock=0;
                oublock=0;msgsnd=0;msgrcv=0;nsignals=0;nvcsw=0;nivcsw=0;
    }
};

//Makes a vector holding multiple commands divided by pipe symbol.
//The commands are unparsed from their args at this point.
vector<char*> parse_line(string s) {
    vector<char*> cmds;
    if(s.size() > 0) {
        char *runner;
        runner = new char [s.size()+1];
        strcpy(runner, s.c_str());
        char *cmds_;
        cmds_ = strtok(runner, "|");
        while (cmds_ != NULL) {
            cmds.push_back(cmds_);
            cmds_ = strtok(NULL, "|");
        }
    }
    return cmds;
}

//Splits a command up from its args
//Returns array of pointers to each arg
char** parse_cmd(char* cmds, char delim) {
    char* temp = cmds;
    char** args;
    args = new char*[1024];
    int idx = 0;
    char* p_del = new char[1];
    p_del[0] = delim;
    args[idx] = strtok(temp, p_del);
    while(temp != NULL) {
        idx++;
        temp = strtok(NULL, p_del);
        args[idx] = temp;
    }
    delete [] p_del;
    return args;
}

//Parses a line into a vector of pointers to arguments
vector<char**> parse_input(string s) {
    vector<char*> cmds;
    cmds = parse_line(s);
    vector<char**> result;
    for(unsigned int i=0; i<cmds.size(); i++) {
        result.push_back(parse_cmd(cmds[i], ' '));
    }
    return result;
}

//Locates a child based on given pid
int find_child(vector<Command_stats> command_stats, pid_t pid_) {
    for(unsigned int i=0; i<command_stats.size(); i++) {
        if(command_stats[i].pid == pid_) {
            return i;
        }
    }
    return -1;
}

//Prints out the commands
void print_cmds(const vector<char**> cmds_args) {
    cout << "Printing commands\n";
    for(unsigned int i=0; i<cmds_args.size(); i++) {
        for(char ** printer = cmds_args[i]; *printer; printer++) {
            cout << *printer << " ";
        }
        cout << "\n";
    }
    cout << "\n";
}

//waits for children and gives their statistics
void wait_for_children(vector<Command_stats> &command_stats, vector<Command_stats> &history) {
    int status; //status of child
    struct rusage child_info; //info from child
    pid_t pid2;
    unsigned int j = 0;
    while(j < command_stats.size()) {//loop until every child counted
        for(unsigned int i=0; i<command_stats.size(); i++) {//until they return
            pid2 = wait4(i, &status, 0, &child_info);//block until child comes back
            if(pid2 > 0) {
                j++;//count the child
                if(WEXITSTATUS(status) == 0) {
                struct timeval user_time_ = child_info.ru_utime;
                struct timeval system_time_ = child_info.ru_stime;
                int idx = find_child(command_stats, pid2);
                double usr = user_time_.tv_usec / 1000000.0;
                double sys = system_time_.tv_usec / 1000000.0;
                usr += user_time_.tv_sec;
                sys += system_time_.tv_sec;
                command_stats[idx].user_time = usr;
                command_stats[idx].system_time = sys;
                cout << "Statistics for " << command_stats[idx].name << endl
                << "Pid: " << pid2 << endl
                << "User time: " << command_stats[idx].user_time << "\n"
                << "System time: " << command_stats[idx].system_time << endl;
                command_stats[idx].maxrss = child_info.ru_maxrss;
                command_stats[idx].ixrss = child_info.ru_ixrss;
                command_stats[idx].idrss = child_info.ru_idrss;
                command_stats[idx].isrss = child_info.ru_isrss;
                command_stats[idx].minflt = child_info.ru_minflt;
                command_stats[idx].majflt = child_info.ru_majflt;
                command_stats[idx].nswap = child_info.ru_nswap;
                command_stats[idx].inblock = child_info.ru_inblock;
                command_stats[idx].oublock = child_info.ru_oublock;
                command_stats[idx].msgsnd = child_info.ru_msgsnd;
                command_stats[idx].msgrcv = child_info.ru_msgrcv;
                command_stats[idx].nsignals = child_info.ru_nsignals;
                command_stats[idx].nvcsw = child_info.ru_nvcsw;
                command_stats[idx].nivcsw = child_info.ru_nivcsw;
                history.push_back(command_stats[idx]);
                }
            }
        }
    }
    command_stats.clear();
}

//Initializes pipe before forking. Modifies pipefd, in, out, next
void prepare_pipe(int* pipefd, int &in, int &out, int &next, int total_commands, int cur_cmd) {
	if (pipe(pipefd) == -1) {
        cerr << "Pipe failed";
        return;
    }
    if(cur_cmd == 0 && cur_cmd != total_commands) {//first command
        in = -1; //first in is stdin
        out = pipefd[WRITE_END];
        next = pipefd[READ_END];
	}
	else if(cur_cmd == 0 && cur_cmd == total_commands) {//one command no pipe
		in = out = next = -1;
	}
    else if(cur_cmd == total_commands) { //last command
	    in = next;
	    out = next = -1;
	}
	else { //middle command
        in = next;
        out = pipefd[WRITE_END];
        next = pipefd[READ_END];
    }
}

//Redirects a process' stdin and stdout to given file descriptors
//only if they are not -1
void child_redirect_in_out(int &in, int &out) {
    if(in != -1) {
	    if(dup2(in, STD_IN) < 0)
		    cerr << "dup2 in failed " << in << endl;
	   close(in);
	}
	if(out != -1) {
	 	if(dup2(out, STD_OUT) < 0)
	        cerr << "dup2 out failed " << out << endl;
		close(out);
	}
}

//Modifies last command to remove the "> outfile.txt" and
//opens the file and modifies out with file descriptor
bool parse_file_output(char**& last_cmd, int& out) {
    string cmd;
    for(char ** printer = last_cmd; *printer; printer++) {
        cmd.append(*printer); cmd.append(" ");
    } //reconstruct the command and reparse it
    if(cmd.find_last_of(">") != string::npos) { //if detect file output
    	int end = cmd.find_last_of(">");
        string file = cmd.substr(end+2, cmd.size()-end-3);//get the filename out
        out = open(file.c_str(), O_WRONLY);
        if(out < 0) {
        	cerr << "Could not open file " << file << endl;
        }
    	string new_cmd = cmd.substr(0, end-1);
    	char *a=new char[new_cmd.size()+1];
        a[new_cmd.size()]=0;
        memcpy(a,new_cmd.c_str(),new_cmd.size());
    	last_cmd = parse_cmd(a, ' ');
    	return true;
    }
    return false;
}

//Modifies first command to remove the "infile.txt <" and
//opens the file and modifies out with file descriptor
bool parse_file_input(char**& first_cmd, int& in) {
    string cmd;
    for(char ** printer = first_cmd; *printer; printer++) {
        cmd.append(*printer); cmd.append(" ");
    } //reconstruct the command and reparse it
    if(cmd.find_first_of("<") != string::npos) {
    	int end = cmd.find_first_of("<");
        string file = cmd.substr(end+2, cmd.size()-end-3);//get the filename out
        in = open(file.c_str(), O_RDONLY);
        if(in < 0) {
        	cerr << "Could not open file " << file << endl;
        }
    	string new_cmd = cmd.substr(0, end-1);
    	char *a=new char[new_cmd.size()+1];
        a[new_cmd.size()]=0;
        memcpy(a,new_cmd.c_str(),new_cmd.size());
    	first_cmd = parse_cmd(a, ' ');
    	return true;
    }
    return false;
}

//Detects an & at end of command
//Returns true if command is to be a background process
bool parse_background_cmd(char**& cmd_) {
    string cmd;
    for(char ** printer = cmd_; *printer; printer++) {
        cmd.append(*printer); cmd.append(" ");
    } //reconstruct the command and reparse it
    if(cmd.find_first_of("&") != string::npos) {
    	int at = cmd.find_first_of("&");
    	string new_cmd = cmd.substr(0, at);
    	char *a=new char[new_cmd.size()+1];
        a[new_cmd.size()]=0;
        memcpy(a,new_cmd.c_str(),new_cmd.size());
    	cmd_ = parse_cmd(a, ' ');
    	return true;
    }
    return false;
}

//Prints stats of all commands entered so far
void print_stats(vector<Command_stats>& stats, bool extra)  {
	if(extra) {
		cout << "full stats of entered commands:\n";
		for(unsigned int i=0; i<stats.size(); i++) {
			cout << stats[i].pid << ": " << stats[i].name
		    << "\nUser time: " << stats[i].user_time
			<< "\nSystem time: " << stats[i].system_time
			<< "\nIntegral max resident set size: " << stats[i].maxrss
			<< "\nIntegral shared text memory size: " << stats[i].ixrss
			<< "\nIntegral unshared data size: " << stats[i].idrss
			<< "\nIntegral unshared stack size: " << stats[i].isrss
			<< "\nPage reclaims: " << stats[i].minflt
			<< "\nPage faults: " << stats[i].majflt
			<< "\nSwaps: " << stats[i].nswap
			<< "\nBlock input operations: " << stats[i].inblock
			<< "\nBlock output operations: " << stats[i].oublock
			<< "\nIPC messages sent: " << stats[i].msgsnd
			<< "\nIPC messages received: " << stats[i].msgrcv
			<< "\nSignals received: " << stats[i].nsignals
			<< "\nVoluntary context switches: " << stats[i].nvcsw
			<< "\nInvoluntary context switches: " << stats[i].nivcsw;
			cout << "\n\n";
		}
		cout << endl;
	}
	else {
	    cout << "stats of entered commands\n";
        for(unsigned int i=0; i<stats.size(); i++) {
            cout << stats[i].pid << ": " << stats[i].name
         	  << "\nUser time: " << stats[i].user_time
           	  << "\nSystem time: " << stats[i].system_time << "\n\n";
        }
	}
}

//Waits for background processes. If not completed, does not wait
void wait_background_processes(vector<Command_stats>& background_cmds, vector<Command_stats>& history) {
    int status; //status of child
    struct rusage child_info; //info from child
    pid_t pid2;
    int size = background_cmds.size();
    if(size != 0)
        cout << "\nWaiting on " << size << " background processes\n";
    for(int i=0; i<size; i++) {
        pid2 = wait4(-1, &status, WNOHANG, &child_info);//wait without blocking
        int idx = find_child(background_cmds, pid2);
        if(pid2 > 0) {
        	cout << "Background process " << background_cmds[idx].name << " returned" << endl;
        	if(WEXITSTATUS(status) == 0) { //check return value of child process
        	    struct timeval user_time_ = child_info.ru_utime;
        	    struct timeval system_time_ = child_info.ru_stime;
        	    double usr = user_time_.tv_usec / 1000000.0;
        	    double sys = system_time_.tv_usec / 1000000.0;
        	    usr += user_time_.tv_sec;
        	    sys += system_time_.tv_sec;
        	    background_cmds[idx].user_time = usr;
        	    background_cmds[idx].system_time = sys;
                cout << "Statistics for " << background_cmds[idx].name << endl
                 << "Pid: " << pid2 << endl
                 << "User time: " << background_cmds[idx].user_time << "\n"
                 << "System time: " << background_cmds[idx].system_time << endl;
                background_cmds[idx].maxrss = child_info.ru_maxrss;
                background_cmds[idx].ixrss = child_info.ru_ixrss;
                background_cmds[idx].idrss = child_info.ru_idrss;
                background_cmds[idx].isrss = child_info.ru_isrss;
                background_cmds[idx].minflt = child_info.ru_minflt;
                background_cmds[idx].majflt = child_info.ru_majflt;
                background_cmds[idx].nswap = child_info.ru_nswap;
                background_cmds[idx].inblock = child_info.ru_inblock;
                background_cmds[idx].oublock = child_info.ru_oublock;
                background_cmds[idx].msgsnd = child_info.ru_msgsnd;
                background_cmds[idx].msgrcv = child_info.ru_msgrcv;
                background_cmds[idx].nsignals = child_info.ru_nsignals;
                background_cmds[idx].nvcsw = child_info.ru_nvcsw;
                background_cmds[idx].nivcsw = child_info.ru_nivcsw;
                history.push_back(background_cmds[idx]);
        	}
        	if(idx >=0) //erase bg process regardless of return value
        	    background_cmds.erase(background_cmds.begin() + idx );
        }
    }
}

int main(int argc, char ** argv, char ** envp) {
	time_t program_start_time = time(NULL);
    cout << "Author: Ryan Konkul rkonku2\nHomework 1: statsh" << endl;
    string cmdline;//user input
    vector<char**> cmds_args; //list of commands, commands are arrays of args
    vector<Command_stats> cur_command_stats;//holds current command stats
    vector<Command_stats> background_processes; //stats of current bg processes
    vector<Command_stats> history; //history of executed children
    do {
    	//wait for any background processes
    	wait_background_processes(background_processes, history);
    	cout.flush();
        cout << "$";
        getline(cin, cmdline); //get a line from user
        if(cmdline.size() == 0)
            continue;
        cerr << endl; cout << endl;
        cmds_args = parse_input(cmdline); //parse into a list of commands
        bool is_background = false;
        char** cmd = cmds_args[cmds_args.size()-1];
        is_background = parse_background_cmd(cmd);//check for a '&'
        if(is_background)//if found one, remove it and pass arguments on
            cmds_args[cmds_args.size()-1] = cmd;
        string e(cmds_args[0][0]);
        if(e == "exit") {
            break;
        }
        else if(e == "cd") {
			chdir(cmds_args[0][1]);
			continue;
		}
        else if(e == "mkdir") {
			mkdir(cmds_args[0][1], 0777);
			continue;
		}
        else if(e == "stats") {
			try{
				string f = cmds_args[0][1];//may cause exception
				if(f == "-f") {//flag for full stats
					print_stats(history, true);
					continue;
				}
			}
		    catch(...){}
		    print_stats(history, false);
		    continue;
		}
        else if(e == "envvar" || e == "envar") {
            cout << "envvar = \n";
        	int i = 0;
            for(char ** e = envp; *e; e++)
        	cout << "  envvar[" << i++ << "] = \"" << *e << "\"\n";
            continue;
        }
        pid_t pid; //child id
        int pipefd[2]; //prepare pipe array
        int in;// in from previous pipe
        int out; // out to next pipe
        int next; // output in next pipe
        for(unsigned int i=0; i<cmds_args.size(); i++) { //for each cmd, fork()
            prepare_pipe(pipefd, in, out, next, cmds_args.size()-1, i);
            if(i==cmds_args.size()-1) {
                char** last_cmd = cmds_args[cmds_args.size()-1];
	    		if(parse_file_output(last_cmd, out)) { //check for > outfile.txt
		    		cmds_args[cmds_args.size()-1] = last_cmd;
		    	}
            }
            if(i==0) {
                char** first_cmd = cmds_args[0];
             	if(parse_file_input(first_cmd, in)) {//check for < infile.txt
                 	cmds_args[0] = first_cmd;
              	}
            }
            print_cmds(cmds_args);//after stripping the < > & signs print out what is left
            pid = fork();//fork the process
            if(pid < 0)
                cout << "Error: fork() failed" << endl;
            else if(pid == 0) {//child process
		        close(next);
                child_redirect_in_out(in, out);//switch child's stdin and stdout
                int exec_error;
                exec_error = execvp(cmds_args[i][0], cmds_args[i]);
                if(exec_error < 0) {
                    cerr << "Could not execute " << cmds_args[i][0] << endl;
                    return -1; //child should return error
                }
            }
            else {//parent process
                close(in);
                close(out);
                //save the child name and child's pid
                if(is_background) {
                	background_processes.push_back(Command_stats(cmds_args[i][0], pid));
                }
                else {
                    cur_command_stats.push_back(Command_stats(cmds_args[i][0], pid));
                }
            }
        }
        //separate loop, wait for children
        wait_for_children(cur_command_stats, history);
    } while(true);
    time_t program_end_time = time(NULL); //calculate time elapsed
    time_t total_time = difftime(program_end_time, program_start_time);
    cout << "statsh was running for " << total_time << " seconds" << endl;
    return 0;
}
