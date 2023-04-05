# minishell
This is a C program for a minishell (ms$) that goes into an infinite loop waiting for user's commands. Once a command is entered, the program should assemble and execute each command using fork(), exec(), and other system calls as required with the following rules and conditions:

## Rules and Conditions
### Rule 1
The argc of any individual command or program should be >=1 and < =6
#### Examples:
     ms$ ls -1 ~/chapter2 -S -n (valid)
     ms$ cat new.txt (valid)
### Rule 2
The argc of individual commands or programs that are used along with the special characters listed below should be >=1 and <=6
#### Examples:
     ms$ ls -1 ~/chapter2 -S -n | wc -w (the first command has 
     argc=5 and the second command has argc=2 which are used along with the special | character)
## Special Characters
The program should handle the following special characters (in accordance to Rule 2 and the additional rules listed below)

### Piping:
Up to 5 piping operations should be supported
#### Example: 
     ms$ cat ex1.c|grep std|wc| wc -w
### Redirection:
>, <, >> redirection is supported
Example: ms$ ls -1 >>dislist.txt
### Conditional Execution:
Up to 5 conditional execution operators should be supported and could possibly be a combination of && and ||
#### Examples:
     ms$ ex1 && ex2 && ex3 && ex4 && ex5
     ms$ c1 && c2 || c3 && c4
### Background Processing:
#### Example: 
     ms$ ex1 & (should run ex1 in the background)
### Sequential Execution:
Up to 5 commands can be executed sequentially
Example: ms$ cat e1.txt; cat e2.txt ; ls ; date
## Usage
- To use this program, compile the minishell.c file using the C compiler of your choice
- Run the compiled program on the command line
- The program will go into an infinite loop waiting for user's commands
- Enter a command with the appropriate arguments and special characters as described above
