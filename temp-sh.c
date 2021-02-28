#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include"linkedlist.h"
#include<signal.h>

static char ** history;
static int no_of_command = 0;


void change_white (){ 
	printf("\033[0m"); 
}
void change_blue(){
	printf("\e[0;34m");
}
void change_red (){ 
	printf("\e[1;31m"); 
}
void change_green (){ 
	printf("\e[0;32m"); 
}
void change_yellow(){
	printf("\e[1;33m");
}


int sh_cd(char** args);
int sh_history(char** args);
int sh_exit(char **args);
int sh_bg(char **args);
int sh_bglist(char **args);
int sh_kill(char **args);
int sh_help();


char* builtin_str[] = {
	"cd",
	"exit",
	"bg",
	"bglist",
	"history",
	"help",
	"kill"
};
int (*builtin_func[]) (char**) = {
	&sh_cd,
	&sh_exit,
	&sh_bg,
	&sh_bglist,
	&sh_history,
	&sh_help,
	&sh_kill
};

#define SH_TOKEN_BUFSIZE 64
#define SH_TOK_DELIM " \t\r\n"
#define SH_HISTORY_BUFFER 101



int sh_cd(char** args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "sh: expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0)
		{
			perror("Invalid Call");
		}
	}
	return 1;
}

int sh_exit(char **args)
{
	return 0;
}

process* headProcess = NULL;
int sh_bg(char **args)
{
//args -- bg echo "hello"
	++args;
//args -- echo "hello"
	char *firstCmd = args[0]; //echo
	int childpid = fork();
	if (childpid >= 0)
	{
		if (childpid == 0)
		{
			if (execvp(firstCmd, args) < 0)
			{
				perror("Error on execvp\n");
				exit(0);
			}

		}
		else {
			if (headProcess == NULL)
			{
				headProcess = create_list(childpid, firstCmd);
			}
			else {
				add_to_list(childpid, firstCmd, true);
			}
		}
	}
	else {
		perror("fork() error");
	}
	return 1;
}
int sh_bglist(char **args)
{
	print_list();
	return 1;
}
int sh_kill(char **args)
{
	// kill 1575
	char* pidCmd = args[1];
	if (!pidCmd) //kill
	{
		printf("Please specify a pid\n");
	}
	else
	{
		int pid = atoi(pidCmd);
		process* target = search_in_list(pid, &headProcess);
		if (target != NULL)
		{
			if (kill(target->pid, SIGTERM) >= 0)
			{
				delete_from_list(pid);
			}
			else {
				perror("Could not kill pid specified\n");
			}
		}
		else {
			printf("Specify a pid which is present in the list.\nType \"bglist\" to see active processes\n");
		}
	}
	return 1;
}

int sh_history(char** args) {
	change_red();
	printf("Command History:\n");
	change_yellow();
	int i;
	if (args[1] == NULL) {
		i = 0;
	}
	else
	{
		i = no_of_command - atoi(args[1]);
		if (i < 0)
			i = 0;
	}
	for (int j = i; j < no_of_command; j++)
		printf("%d:%s", j + 1, history[j]);
	change_white();
	return 1;
}


int sh_help(){
	
	change_red();
    
    printf("\nWelcome to TEMP-SH, you can use the following builtin commands:\n");
    
    change_white();
    
    int n=sizeof(builtin_str)/sizeof(char *);
    
    for(int i=0;i<n;i++){
        printf("%d. %s\n" ,i+1,builtin_str[i]);
    }
    
    change_red();
    
    printf("\nStandard linux commands are supported.");
    printf("\nSingle Pipeline supported .Please put a space before and after | while using pipeline\n");
	
	change_white();
    
    return 1;
}

void broadcastTermination(int pid, int status) {
	if (WIFEXITED(status)) {
		printf("exited, status=%d\n", WEXITSTATUS(status));
	}
	else if (WIFSIGNALED(status)) {
		printf("killed by signal %d\n", WTERMSIG(status));
	}
	else if (WIFSTOPPED(status))
	{
		printf("stopped by signal %d\n", WSTOPSIG(status));
	}
	else if (WIFCONTINUED(status))
	{
		printf("continued\n");
	}
	delete_from_list(pid);
}

static void signalHandler(int sig) {
	int pid;
	int status;
	pid = waitpid(-1, &status, WNOHANG);
	broadcastTermination(pid, status);
}

// ------------------------------------------------------------------------------------------------------------

//INPUT CODE

// -----------------------------------------------------------------------------------------------------------

char *sh_read_line()
{
	char*line = NULL;
	ssize_t bufsize = 0;
	if (getline(&line, &bufsize, stdin) == -1)
	{
		if (feof(stdin))
			exit(EXIT_SUCCESS);
		else
		{
			perror("acm-sh: getline\n");
			exit(EXIT_FAILURE);
		}
	}
	return line;
}
char** sh_split_line(char* line)
{
	int bufsize = SH_TOKEN_BUFSIZE, position = 0;
	char ** tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!token)
	{
		fprintf(stderr, "acm-sh: allocation error\n");
		exit(EXIT_FAILURE);
	}

	//strtok :- Library Function
	token = strtok(line, SH_TOK_DELIM);
	while (token != NULL)
	{
		tokens[position] = token;
		token = strtok(NULL, SH_TOK_DELIM);

		position++;
	}
	tokens[position] = NULL;
	return tokens;
}


// Break down String into Two if Pipeline Present 
int isPiped(char* str, char** strpiped)
{
	int i;
	for (i = 0; i < 2; i++) {
		strpiped[i] = strsep(&str, "|");
		if (strpiped[i] == NULL)
			break;
	}

	if (strpiped[1] == NULL)
		return 0;
	else {
		return 1;
	}
}

void str_to_token(char* str, char** args)
{
	int i;

	for (i = 0; i < SH_TOKEN_BUFSIZE; i++) {
		args[i] = strsep(&str, " \t\r\n");

		if (args[i] == NULL)
			break;
		if (strlen(args[i]) == 0)
			i--;
	}
}

int processString(char* str, char** args, char** parsedpipe)
{

	char* strpiped[2];
	int piped = 0;

	piped = isPiped(str, strpiped);

	if (piped) {
		str_to_token(strpiped[0], args);
		str_to_token(strpiped[1], parsedpipe);

	} else {

		str_to_token(str, args);
	}
	return piped;
}

// ------------------------------------------------------------------------------------------------------------------

//                                                  COMMAND EXECUTION CODE

// -------------------------------------------------------------------------------------------------------------------
int sh_launch(char **args)
{
	pid_t pid;
	int status;

	pid = fork();
	if (pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			printf("Invalid Command\n");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		perror("acm-sh");
	}
	else
	{
		do
		{
			waitpid(pid, &status, WUNTRACED);
		}
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int sh_execute(char ** args)
{
	int i;
	if (args[0] == NULL)
	{
		return 1;
	}
	 int n=sizeof(builtin_str)/sizeof(char *);
	for (int i = 0; i < n; i++)
	{
		if (strcmp(args[0], builtin_str[i]) == 0)
			return (*builtin_func[i])(args);
	}

	return sh_launch(args);

}

//----------------------------------------------------------------------------------------------------------------------------
//                                          PIPELINED COMMAND EXECUTION FUNCTION
//----------------------------------------------------------------------------------------------------------------------------


int execArgsPiped(char** args, char** parsedpipe)
{
	// 0 is read end, 1 is write end
	int pipefd[2];
	pid_t p1, p2;
	int status1, status2;
	if (pipe(pipefd) < 0)
	{
		printf("Error in pipe initialization\n");
		return 1;
	}

	p1 = fork();
	if (p1 < 0) {
		perror("acm-sh");
		return 1;
	}

	if (p1 == 0) {

		dup2(pipefd[1], STDOUT_FILENO);
		close(pipefd[0]);

		if (execvp(args[0], args) < 0)
		{
			printf("Invalid Command\n");
		}
		exit(EXIT_FAILURE);
	}
	else
	{
		p2 = fork();
		if (p2 < 0)
		{
			printf("Error in fork\n");
			return 1;
		}

		if (p2 == 0)
		{
			dup2(pipefd[0], STDIN_FILENO);
			close(pipefd[1]);
			if (execvp(parsedpipe[0], parsedpipe) < 0)
			{
				printf("Invalid Command\n");
			}
			exit(EXIT_FAILURE);
		}
		else {
			// parent executing, waiting for two children
			do
			{
				waitpid(p1, &status1, WUNTRACED);
			}
			while (!WIFEXITED(status1) && !WIFSIGNALED(status1));
			do
			{
				waitpid(p2 + 2, &status2, WUNTRACED);
			}
			while (!WIFEXITED(status2) && !WIFSIGNALED(status2));
		}
	}
	return 1;
}


//---------------------------------------------------------------------------------------------------
//                                                    MAIN FUNCTION
//---------------------------------------------------------------------------------------------------
int main(int argc, char **argv)
{
	char *line;
	char *args[SH_TOKEN_BUFSIZE];
	char* argsPiped[SH_TOKEN_BUFSIZE];
	int status;
	history = (char **)malloc(sizeof(char *)*SH_HISTORY_BUFFER);
	

	signal(SIGCHLD, signalHandler);
	do {
		change_green();
		printf("TEMP-SH > "); 
		change_white();
		line = sh_read_line();
		history[no_of_command] = (char *)malloc(sizeof(*line));
		strcpy(history[no_of_command], line);
		
		//piped_command = 0 -> not piped
		//piped_command = 1 -> pipe exists
		int piped_command = processString(line, args, argsPiped);
		
		if (piped_command == 0)
		{
			status = sh_execute(args);
		}
		else if (piped_command == 1)
		{
			status = execArgsPiped(args, argsPiped);
		}

		//Keeping a check at history
		no_of_command++;
		if (no_of_command >= SH_HISTORY_BUFFER)
		{
			for (int i = 0; i < SH_HISTORY_BUFFER - 1; i++) 
			{
				history[i] = history[i + 1];
			}
			no_of_command--;
		}
	}
	while (status);
	return 0;
}

