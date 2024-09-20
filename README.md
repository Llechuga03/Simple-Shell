# Simple-Shell
"The goal of this project is to implement a basic shell which is able to execute commands, redirect the standard input/output (stdin/stdout) of commands to files, pipe the output of commands to other commands, and carry out commands in the background."


User Defined Implementation
	⁃	My basic intuition on how to start this project was to follow the REPL design for standard shells. We were provided a general outline of how the loop procedure would operate, and were give two functions to declare ourselves: type_prompt() and read_command(command,parameters).
	⁃	The first implementation I started on was the type_prompt() function which simply outputs “my-shell$” as a prompt, which is then followed by whatever commands the user wishes to be called. So far i’ve had no issues with this part of the project, it seems very straightforward.
	⁃	Next, we move onto the read_command(command,parameters). To my understanding, this is the portion of the REPL design that takes care of the read and parse line steps. To implement the read line instructions, I utilized the fgets command to take in user input. This section was a bit interesting due to how to handle string parsing. The original idea I had was to look for spaces, though I found this not to be effieceint due to two reasons: one, this would not work for the example “echo foo | grep -o …” due to the pipe command, and two for cases where there are leading or trailing spaces. Therefore I decided to use the pipe character as my new delimeter, and also made a function that moves through the given input from stdin and gets rid of excessive spaces, that includes leading, trailing, or extra spaces in between words. Therefore my tokens were separated by the pipe character, which meant I had to tokenize once again, and this time the delimeter was a single space in between words. For the mini dealine, we’re only required to execute the command of the first function call, therefore when I tokenized the second time (looking for whitespaces) I only passed the first token from the original input. This portion of the assignment took the longest and the main issues I had were getting rid of unnecessary spaces and figuring out how to build my argument list correctly based off of the delimeters I was using.
	⁃	After the string parsing and argument list was done correctly, I was then able to call the fork system call in order to execute the desired function passed by the user. I checked for each return condition of fork(), with pid<0 initiating an error to be displayed, if the pid != 0, fork was succesful and we call waitpid() inside of the parent to wait for the return of 0 to be returned from the child. This then causes execvp to be called and the resulting output is displayed on the terminal.
   - The main issues I had when writing this code was making sure that flushing out my buffer correctly after printing out a statement with a new line. This was mainly done due to submission guidelines, but it is very good coding practice in general. Instead of using fflush() after every printf() statement, you can instead use the setvbuf() function once at the beginning of your int main() code; check out the man page for this to understand why it works properly ;)
