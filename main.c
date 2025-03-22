#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM "\t\r\n\a"

int tss_cd(char **args);
int tss_help(char **args);
int tss_exit(char **args);

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

int tss_cd(char **args)
{
	if (args[1] == NULL)
	{
		fprintf(stderr, "tss: expected argument to \"cd\"\n");
	}
	else
	{
		if (chdir(args[1]) != 0)
		{
      		perror("tss");
    	}
	}
	return 1;
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

int lsh_exit(char **args)
{
	(void) args;
	return 0;
}

int tss_launch(char **args)
{
	pid_t pid;
	pid_t wpid;
	int status;

	pid = fork();
	wpid = NULL;
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
		} while (!WIFEXITED(status) && !WIFSIGNALED(status));
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
	return lsh_launch(args);
}

char *tss_read_line(void)
{
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char) * bufsize);
	int c;

	if (!buffer)
	{
		fprintf(stderr, "tss: allocation error\n");
		exit(EXIT_FAILURE);
	}

	while (1) {
		c = getchar();

		if (c == EOF || c == '\n')
		{
			buffer[position] = '\0';
			return buffer;
		}
		else 
		{
			buffer[position] = c;
		}
		position++;

		// If we have exceeded the buffer, reallocate.
		if (position >= bufsize)
		{
			bufsize += LSH_RL_BUFSIZE;
			buffer = realloc(buffer, bufsize);
			if (!buffer)
			{
				fprintf(stderr, "tss: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}

char **lsh_split_line(char *line)
{
	int bufsize = LSH_TOK_BUFSIZE, position = 0;
	char **tokens = malloc(bufsize * sizeof(char*));
	char *token;

	if (!tokens)
	{
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token = strtok(line, LSH_TOK_DELIM);
	while (token != NULL)
	{
		tokens[position] = token;
		position++;

		if (position >= bufsize)
		{
			bufsize += LSH_TOK_BUFSIZE;
			tokens = realloc(tokens, bufsize * sizeof(char*));
			if (!tokens)
			{
				fprintf(stderr, "lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
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
		status = tss_execute(agrs);
	} while (status);
}

int main(int argc, char **argv)
{
	tss_loop();
	return (0);
}

