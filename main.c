#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

int tss_cd(char **args);
int tss_help(char **args);
int tss_exit(char **args);

// 1. fix permission denied when running code
// 2. fix wpid issues in tss_launch function

char *builtin_str[] = {
	"cd",
	"help",
	"exit"
};
  
int (*builtin_func[]) (char **) = {
	&tss_cd,
	&tss_help,
	&tss_exit
};
  
int tss_num_builtins() {
	return sizeof(builtin_str) / sizeof(char *);
}

int tss_launch(char **args)
{
	pid_t pid;
	pid_t wpid;
	int status;

	pid = fork();
	wpid = waitpid(pid, &status, WUNTRACED);;
	if (pid == 0)
	{
		if (execvp(args[0], args) == -1)
		{
			perror("tss");
		}
		exit(EXIT_FAILURE);
	}
	else if (pid < 0)
	{
		perror("tss");
	}
	else
	{
		do
		{
			wpid = waitpid(pid, &status, WUNTRACED);
		} 
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int tss_execute(char **args)
{
	int i;

	if (args[0] == NULL)
	{
		return 1;
	}
	for (i = 0; i < tss_num_builtins(); i++)
	{
		if (strcmp(args[0], builtin_str[i]) == 0)
		{
			return (*builtin_func[i])(args);
		}
	}
	return tss_launch(args);
}

int tss_cd(char **args)
{
    if (args[1] == NULL)
    {
        fprintf(stderr, "tss: expected argument to \"cd\"\n");
        return 1;
    }

    // Try to manually update PWD using setenv
    if (setenv("PWD", args[1], 1) != 0)
    {
        perror("tss");
        return 1;
    }
	printf("PWD set to: %s\n", args[1]);

	// After successful directory change, print the current working directory (optional)
	char *cwd = getcwd(NULL, 0); // Get the current directory
	if (cwd != NULL)
	{
		printf("Current directory changed to: %s\n", cwd);
		free(cwd);
	}
	else
	{
		perror("tss: getcwd");
		return 1;
	}
		
	// Relaunch the shell in the new directory (if you want to restart the shell process)
	// You can relaunch by executing the shell again here, like this:
	char *argv[] = {args[0], NULL}; // Set the shell's arguments to relaunch
	execvp(argv[0], argv); // Relaunch the shell or program in the new directory

		return 0;  // This line will never be reached unless execvp fails
}

int tss_help(char **args)
{
	(void) args;
	int i;
	printf("Tej Shah's Shell TSS\n");
	printf("Type program names and arguments, and hit enter.\n");
	printf("The following are built in:\n");

	for (i = 0; i < tss_num_builtins(); i++)
	{
		printf("  %s\n", builtin_str[i]);
	}

	printf("Use the man command for information on other programs.\n");
	return 1;
}

int tss_exit(char **args)
{
	(void) args;
	return 0;
}

char *tss_read_line(void)
{
  char *line = NULL;
  size_t bufsize = 0; // have getline allocate a buffer for us

  if (getline(&line, &bufsize, stdin) == -1){
    if (feof(stdin)) {
      exit(EXIT_SUCCESS);  // We recieved an EOF
    } else  {
      perror("readline");
      exit(EXIT_FAILURE);
    }
  }

  return line;
}

#define INITIAL_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n"

char **tss_split_line(char *line) {
    int bufsize = INITIAL_BUFSIZE;
    int position = 0;
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    if (tokens == NULL) {
        fprintf(stderr, "lsh: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        // Ensure space for next token + NULL terminator
        if (position >= bufsize - 1) {
            bufsize += INITIAL_BUFSIZE;
            char **temp = realloc(tokens, bufsize * sizeof(char*));
            if (!temp) {
                free(tokens);
                fprintf(stderr, "lsh: allocation error\n");
                exit(EXIT_FAILURE);
            }
            tokens = temp;
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

void tss_loop(void)
{
	char *line;
	char **args;
	int status;

	do
	{
		printf("> ");
		line = tss_read_line();
		args = tss_split_line(line);
		status = tss_execute(args);

		free(line);
		free(args);
	} while (status);
}

int main(int argc, char **argv)
{
	(void) argc;
	(void) argv;

	tss_loop();
	return (EXIT_SUCCESS);
}

