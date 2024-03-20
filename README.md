# myShell | CIS*3110 Assignment | author: Cameron Norrie | Date: Feb 12, 2021

This program is a Bash shell built from scratch in C. It implements basic commands such as ls, cd, pwd, and more. 
Supports background processes and piping. Mimicing the behaviour of a Linux shell but doesn't support full functionality.
Uses forked process to execute commands with parent-child hierarchical structure.
Only compatabile with Linux.

# Functionality:
This program support running bash commands with any number of arguments. e.g ls -a
The program also support input and output redirection as well as piping e.g ls | wc e.g cat ameer.txt > ameer2.txt
The program includes support for built-in commands history, cd
Additonally can echo environment variables from this shell : PATH, HOME,HISTFILE
Does not support export command or chaining redirects or chaining piping.
Background support is limited, commands run in the background but will not print when terminated.

# Compilation
$ make

# Execution
./myShell Then use as you would a bash shell.
