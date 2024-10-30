Quash Shell Project Report
Overview
The objective of this project was to develop a custom command-line shell called Quash that mimics the behavior of common UNIX shells such as bash. Quash supports several functionalities, including built-in commands, external command execution with process management, background processes, signal handling, process timeouts, and basic I/O redirection. This project helped us gain practical experience with process forking, signal handling, and interacting with system calls in UNIX.

Design Choices
Built-in Commands
Quash implements several built-in commands: cd, pwd, echo, env, setenv, and exit. Each built-in command is implemented as a separate function, which is called if the shell identifies the command as built-in. The cd command changes the working directory using chdir(), while pwd retrieves the directory path using getcwd().

For echo, the shell loops through all arguments after the echo command, printing each one. The env command displays all environment variables or a specified variable, accessed through the environ array. The setenv command uses setenv() to add or update an environment variable, allowing overwrite. By having separate functions for each command, Quash is modular and allows for easy extension if additional built-in commands need to be added in the future.

Process Management and Forking
For external commands, Quash uses fork() to create a child process. Within the child, execvp() is called to execute the external command. If execvp() fails, an error message is displayed, and the child process exits. The parent process, which represents Quash itself, waits for the child process to finish before returning to the prompt (unless the command is run in the background).

By implementing process forking, we allowed Quash to handle commands in parallel, enabling the user to run background processes while still interacting with the shell.

Background Processes
Quash supports background processes by identifying if a command ends with &. If present, & is removed from the command, and the shell forks a child process without waiting for it to finish. This lets the shell immediately return to the prompt while the background process continues executing. This feature was relatively straightforward to implement by adding a check for & and modifying the shell to only wait if the command isn’t a background process.

Signal Handling
A major requirement for Quash was handling SIGINT (Ctrl+C) to avoid terminating the shell itself when interrupting a running process. We accomplished this by setting a custom signal handler using signal(SIGINT, handler), where the handler function catches and ignores the signal. When a user presses Ctrl+C, only the currently running command is interrupted, while Quash remains active and returns to the prompt. Signal handling is critical to provide a robust shell experience and to prevent unintended shell terminations.

Timeout for Long-Running Processes
Quash includes a timeout mechanism for long-running foreground processes. If a process runs for longer than 10 seconds, the shell terminates it using kill(). This feature is implemented by setting an alarm with alarm(10) before calling execvp() in the child process. If the process completes before 10 seconds, alarm(0) disables the timer. If the process exceeds 10 seconds, the alarm signal triggers, and the handler calls kill() to terminate the process. This feature helps manage process resources effectively.

I/O Redirection
I/O redirection is implemented by checking for > (output redirection) and < (input redirection) symbols in the command. If > is found, Quash opens the specified file with write permissions and redirects STDOUT to this file using dup2(). If < is found, the specified file is opened with read permissions, and STDIN is redirected to this file. By adding I/O redirection, Quash can now support a variety of command formats, enhancing its versatility.

Code Documentation
Main Function: Initializes signal handlers and starts the shell prompt loop.
execute_command: Parses the input command, handles built-in commands, and forks a child process for external commands.
change_directory: Implements the cd command by changing the current directory.
print_working_directory: Implements the pwd command to display the current directory.
echo: Implements the echo command, supporting environment variable substitution.
set_environment_variable: Sets or updates environment variables.
print_environment: Prints all environment variables or a specified variable.
sigint_handler: Signal handler for SIGINT, preventing the shell from terminating on Ctrl+C.
alarm_handler: Signal handler for process timeouts, terminating processes running over 10 seconds.
I/O Redirection Handling: Detects and handles > and < for output and input redirection.
Conclusion
The Quash shell project provided practical experience with UNIX system programming concepts, including process management, signaling, and I/O redirection. By implementing these features, we built a functional shell that can handle user commands, manage background and foreground processes, and interact with the file system. This project solidified our understanding of C programming in a UNIX environment and helped us develop essential system programming skills.