/*	
	Author: Rishabh Pahwa (110091645)
	Title: COMP8567: Advanced Systems Programming - Assignment 3
	Submitted to: Dr. Prashanth Ranga
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <regex.h>
#include <sys/wait.h>
#define MAX_CMDS 5
#define MAX_INPUT 1024
#define MAX_ARGS 6
#define DEBUG_MODE 0

int isCharEscaped(const char *str, int index);
char *trimWhiteSpace(char *s);
char *formatCommandForShell(const char *command);
int validateCommandArguments(char *command);
int validateCommandsSequence(char **commands, int num_commands);
int tokenizeString(char *str, char **tokens);
int execute(char *cmd, int should_fork);
int execute_pipe(char **commands, int num_commands);
void parseArguments(char **commands, int len);

int main()
{
	while (1)
	{
		char *command_buffer = malloc(4096);
		char *input = malloc(4096);
		char *formatted_input;
		char *tokens[150];

		fflush(stdout);
		printf("ms$ ");
		fgets(command_buffer, MAX_INPUT, stdin);

		if (DEBUG_MODE)
		{
			printf("\nInput: %s\n", command_buffer);
		}

		// Remove leading or trailing spaces
		input = trimWhiteSpace(command_buffer);

		// exit program if user enters "exit"
		if (strcmp(input, "exit") == 0)
		{
			printf("Exiting minishell...\n");
			break;
		}

		if (DEBUG_MODE)
		{
			printf("trimWhiteSpacemed Input: %s\n", input);
		}

		// Skip empty value
		if (strlen(input) == 0)
		{
			continue;
		}

		formatted_input = formatCommandForShell(input);

		if (DEBUG_MODE)
		{
			printf("Formatted input: %s\n", formatted_input);
		}

		int num_tokens = tokenizeString(formatted_input, tokens);

		if (DEBUG_MODE)
		{
			printf("\nNumber of tokens: %d\n", num_tokens);
		}

		if (DEBUG_MODE)
		{
			printf("\n");
			for (int i = 0; i < num_tokens; i++)
			{
				printf("Token %d: %s\n", i, tokens[i]);
			}
		}

		if (validateCommandsSequence(tokens, num_tokens))
			parseArguments(tokens, num_tokens);
	}

	return 0;
}

/*
if(DEBUG_MODE){
	printf("\n\n");
}

*/

// Check if a character is escaped by a backslash
int isCharEscaped(const char *str, int index)
{
	if (index > 0 && str[index - 1] == '\\')
	{
		// Check if there is an even number of backslashes before this one
		int count = 0;
		while (index > 0 && str[index - 1] == '\\')
		{
			count++;
			index--;
		}

		return count % 2 != 0;
	}

	return 0;
}

char *trimWhiteSpace(char *s)
{
	int i;

	// skip leading white spaces
	while (isspace(*s))
	{
		s++;
	}

	// skip trailing white spaces
	for (i = strlen(s) - 1;
		(isspace(s[i])); i--);
	s[i + 1] = '\0';
	return s;
}

// Format a command string to add spaces around special characters
char *formatCommandForShell(const char *command)
{
	const char *specials = "<>|&;";
	const char *double_specials = ">|&";
	size_t len = strlen(command);
	char *formatted = malloc(len *3 + 1);	// Allocate enough space for worst-case scenario
	size_t i = 0, j = 0;
	while (i < len)
	{
		if (strchr(specials, command[i]) != NULL && !isCharEscaped(command, i))
		{
			// Add spaces around special character
			if (strchr(double_specials, command[i]) != NULL && i + 1 < len && command[i] == command[i + 1])
			{
				formatted[j++] = ' ';
				formatted[j++] = command[i++];
				formatted[j++] = command[i];
				formatted[j++] = ' ';
				formatted[j++] = ' ';
			}
			else
			{
				formatted[j++] = ' ';
				formatted[j++] = command[i];
				formatted[j++] = ' ';
			}
		}
		else
		{
			// Copy regular character
			formatted[j++] = command[i];
		}

		i++;
	}

	formatted[j] = '\0';
	return formatted;
}

int validateCommandArguments(char *command)
{
	char *cmd = strdup(command);
	cmd = trimWhiteSpace(cmd);

	char *pch = strtok(cmd, " ");
	int args = 0;
	while (pch != NULL)
	{
		pch = strtok(NULL, " ");
		args++;
	}

	return args <= MAX_ARGS && args > 0;
}

int validateCommandsSequence(char **commands, int num_commands)
{
	if (num_commands % 2 == 0)
	{
		printf("improper command. please check your input\n");
		return 0;
	}

	if (num_commands > 2 *MAX_CMDS - 1)
	{
		printf("Maximum 5 commands supported\n");
		return 0;
	}

	for (int i = 0; i < num_commands; i++)
	{
		if (DEBUG_MODE)
		{
			printf("validating command: %s\n", commands[i]);
		}

		if (i % 2) continue;
		int cmd_valid = validateCommandArguments(commands[i]);
		if (!cmd_valid)
		{
			printf("Invalid command\n");
			return 0;
		}
	}

	return 1;
}

int tokenizeString(char *str, char **tokens)
{
	int num_tokens = 0;
	int len = strlen(str);
	int i;
	int start_index = 0;

	for (i = 0; i < len; i++)
	{
		if (strncmp(str + i, "&&", 2) == 0)
		{
			tokens[num_tokens] = malloc((i - start_index + 1) *sizeof(char));
			strncpy(tokens[num_tokens++], str + start_index, i - start_index);
			tokens[num_tokens++] = "&&";
			i++;
			start_index = i + 2;
		}
		else if (strncmp(str + i, "||", 2) == 0)
		{
			tokens[num_tokens] = malloc((i - start_index + 1) *sizeof(char));
			strncpy(tokens[num_tokens++], str + start_index, i - start_index);
			tokens[num_tokens++] = "||";
			i++;
			start_index = i + 2;
		}
		else if (str[i] == '|')
		{
			tokens[num_tokens] = malloc((i - start_index + 1) *sizeof(char));
			strncpy(tokens[num_tokens++], str + start_index, i - start_index);
			tokens[num_tokens++] = "|";
			start_index = i + 1;
		}
		else if (str[i] == ';')
		{
			tokens[num_tokens] = malloc((i - start_index + 1) *sizeof(char));
			strncpy(tokens[num_tokens++], str + start_index, i - start_index);
			tokens[num_tokens++] = ";";
			start_index = i + 1;
		}
	}

	// Copy the last token
	tokens[num_tokens] = malloc((len - start_index + 1) *sizeof(char));
	strncpy(tokens[num_tokens++], str + start_index, len - start_index);

	return num_tokens;
}

int execute(char *cmd, int should_fork)
{
	char *command = strdup(cmd);
	command = trimWhiteSpace(command);
	char *arguments[10];
	char *token;
	char *delim = " ";
	int background = 0;
	char *input_file = NULL;
	char *output_file = NULL;
	int output_append = 0;

	// Remove newline character from end of command
	if (command[strlen(command) - 1] == '&')
	{
		background = 1;
		command[strlen(command) - 1] = '\0';
		command = trimWhiteSpace(command);
	}

	// Check if user entered "cd"
	if ((strcmp(command, "cd") == 0) || (strstr(command, "cd ~") == command))
	{
		// Change working directory to home directory
		chdir(getenv("HOME"));
		return 0;

		// Change working directory to specified directory
	}
	else if (strstr(command, "cd ") == command)
	{
		// Skip "cd " at the beginning of the command
		char *dir = command + 3;
		dir = trimWhiteSpace(dir);
		chdir(dir);
		return 0;
	}

	// Check for input/output redirection
	int i = 0;
	char *next_token;
	token = strtok_r(command, delim, &next_token);
	while (token != NULL)
	{
		if (strcmp(token, "<") == 0)
		{
			// Input redirection
			input_file = strtok_r(NULL, delim, &next_token);
		}
		else if (strcmp(token, ">") == 0)
		{
			// Output redirection
			output_file = strtok_r(NULL, delim, &next_token);
			output_append = 0;
		}
		else if (strcmp(token, ">>") == 0)
		{
			// Append output redirection
			output_file = strtok_r(NULL, delim, &next_token);
			output_append = 1;
		}
		else
		{
			arguments[i++] = token;
		}

		token = strtok_r(NULL, delim, &next_token);
	}

	// last char to null
	arguments[i] = NULL;

	// Fork process
	int pid = 0;
	if (should_fork)
	{
		pid = fork();
	}

	if (pid < 0)
	{
		fprintf(stderr, "Error: fork failed\n");
		exit(1);
	}
	else if (pid == 0)
	{
		// Child process
		int fd_in, fd_out;
		if (input_file)
		{
			fd_in = open(input_file, O_RDONLY);
			if (fd_in < 0)
			{
				fprintf(stderr, "Error: could not open input file\n");
				exit(1);
			}

			dup2(fd_in, STDIN_FILENO);
			close(fd_in);
		}

		if (output_file)
		{
			if (output_append)
			{
				fd_out = open(output_file, O_WRONLY | O_CREAT | O_APPEND, 0666);
			}
			else
			{
				fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
			}

			if (fd_out < 0)
			{
				fprintf(stderr, "Error: could not open output file\n");
				exit(1);
			}

			dup2(fd_out, STDOUT_FILENO);
			close(fd_out);
		}

		if (execvp(arguments[0], arguments) < 0)
		{
			// execvp(arguments[0], arguments);
			fprintf(stderr, "Error: exec failed\n");
			exit(1);
		}
	}
	else
	{
		// Parent process
		if (!background)
		{
			// Wait for child process to finish
			int stat;
			wait(&stat);
			return stat;
		}
		else
		{
			printf("Background process started with PID: %d\n", pid);
			return 0;
		}
	}

	if (should_fork == 0)
	{
		if (!background)
		{
			// Wait for child process to finish
			int stat;
			wait(&stat);
			return stat;
		}
		else
		{
			printf("Background process started with PID: %d\n", pid);
			return 0;
		}
	}

	return 0;
}

int execute_pipe(char **commands, int num_commands)
{
	int i;
	int pid = fork();
	if (!pid)
	{
		for (i = 0; i < num_commands - 1; i += 2)
		{
			int pd[2];
			pipe(pd);

			if (!fork())
			{
			 	// set output to current pipe
				if ((dup2(pd[1], STDOUT_FILENO)) == -1)
				{
					perror("dup2");
					exit(EXIT_FAILURE);
				}

				execute(commands[i], 0);
			}

			// set input to previous pipe
			if ((dup2(pd[0], STDIN_FILENO)) == -1)
			{
				perror("dup2");
				exit(EXIT_FAILURE);
			}

			close(pd[1]);
		}

		// execute last command
		execute(commands[i], 0);
	}
	else if (pid > 0)
	{
		char *command = strdup(commands[num_commands - 1]);
		command = trimWhiteSpace(command);
		if (command[strlen(command) - 1] != '&')
		{
			waitpid(pid, NULL, 0);
		}
		else
		{
			printf("Background process started with PID: %d\n", pid);
		}
	}
}

void parseArguments(char **commands, int len)
{
	int i = 0;
	int result = 0;
	int fd[2];
	int previous_result = 1;

	while (i < len)
	{
		if (DEBUG_MODE)
		{
			printf("Processing Token %d: %s\n", i, commands[i]);
		}

		// Handle conditional execution for "||"
		if (strcmp(commands[i], "||") == 0)
		{
			if (previous_result == 0)
			{
			 	// skip the next command
				i++;
				while (i < len && strcmp(commands[i], "&&") != 0 && strcmp(commands[i], "|") != 0 && strcmp(commands[i], ";") != 0)
				{
					i++;
				}

				if (i > 0 && i < len && strcmp(commands[i], "|") == 0) i += 2;
			}
			else
			{
				i++;
			}

			// Handle conditional execution for "&&" 
		}
		else if (strcmp(commands[i], "&&") == 0)
		{
			if (previous_result != 0)
			{
			 	// skip the next command
				i++;
				while (i < len && strcmp(commands[i], "||") != 0 && strcmp(commands[i], "|") != 0 && strcmp(commands[i], ";") != 0)
				{
					i++;
				}

				if (i > 0 && i < len && strcmp(commands[i], "|") == 0) i += 2;
			}
			else
			{
				i++;
			}

			// Handle piping character
		}
		else if (strcmp(commands[i], "|") == 0)
		{
			i++;

			// Handle sequential character
		}
		else if (strcmp(commands[i], ";") == 0)
		{
			i++;
		}

		// Handle piping execution
		else if (i < len - 1 && strcmp(commands[i + 1], "|") == 0)
		{
			if (DEBUG_MODE)
			{
				printf("Executing piped comand: %s\n", commands[i]);
			}

			printf("\n");
			int start = i;
			//combine all piping commands and append input redirection
			while (i < len - 1 && strcmp(commands[i + 1], "|") == 0)
				i += 2;
			execute_pipe(commands + start, i - start + 1);
			i++;
		}

		// Handle regular command
		else
		{
			if (DEBUG_MODE)
			{
				printf("Executing regular command: %s\n", commands[i]);
			}

			printf("\n");
			result = execute(commands[i], 1);
			previous_result = result;
			if (DEBUG_MODE)
			{
				printf("Result: %d\n", result);
			}

			i++;
		}
	}
}