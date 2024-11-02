Project 1 - Simple Shell
Goals
        To understand & correctly use important Unix/POSIX system calls.
        To develop a simple shell application.

User Defined Implementation
        My basic intuition on how to start this project was to follow the REPL design for standard shells. 
        The first implementation I started on was ensuring the $myshell prompt was accurately being ouputted to the user. In order to check this
	 I checked the arguments of the int main(int argc, char* argv) function, to ensure that if the user included the "-n" operator 
  	 to their command that the prompt would not be outputted. You can see this check early on in the myshell.c file.
        Next, we move onto reading commands from the user. To my understanding, this is the portion of the REPL design that takes
         care of the read and parse line steps. To implement the read line instructions, I utilized the fgets command to take in user
         input. This section was a bit interesting due to how to handle string parsing. The original idea I had was to look for spaces,
         though I found this not to be effieceint due to two reasons: one, this would not work for the example "echo foo | grep -o"
         due to the pipe command, and two for cases where there are leading or trailing spaces. Therefore I decided to use the pipe
         character as my new delimeter, and also made a function that moves through the given input from stdin and gets rid of excessive
         spaces, that includes leading, trailing, or extra spaces in between words (see collapse_spaces and trim_spaces).
	 Therefore my tokens were separated by the pipe character, which meant I had to tokenize once again, and this time the delimeter
  	 was a single space in between words. For the mini dealine, we needed to execute the command of the first function call, therefore
    	 when I tokenized the second time (looking for whitespaces) I only passed the first token from the original input.
      	 This portion of the assignment took the longest and the main issues I had were getting rid of unnecessary spaces and figuring out 
	 how to build my argument list correctly based off of the delimeters I was using.
         After the string parsing and argument list was done correctly, I was then able to call the fork system call in order to execute
         the desired function passed by the user. I checked for each return condition of fork(), with pid<0 initiating an error to be
         displayed, if the pid != 0, fork was succesful and we call waitpid() inside of the parent to wait for the return of 0 to be
         returned from the child. This then causes execvp to be called and the resulting output is displayed on the terminal.
         The main issue I had with this assignment was getting my code to compile on bandit and then passing the test cases. Similar to others
         my code ran perfectly on my machine, but when compiing on bandit it would fail. What ultimately helped me was actually figuring out how
         to make a proper Makefile, with the help of a TA, and then using the setvbuff(stdout,NULL,_IONBF,0) command at the beginning of my main
         program.
 POST MINI DEADLINE
         PIPELINE.C FILE
         - The Pipeline.c file handles instances of the | character and correctly handles 2 or more commands passed together using the special 		character. The inital issue I faced with this function was correctly implementing pipe() for more than 2 commands. For the piping logic I 	ended up making a funciton that takes in as inputs,a chacter aray of arguments, which holds all of the commands, and an int variable, 		cmd_count, that represents the number of pipes we need to account for.
        The cmd_count value starts at 0 in myshell.c and is incremented as we encounter the | character when parsing our input from the user. We then 	make file descriptors for every cmd_counts - 1 since the last command does not need a pipe, each pipe also contains two fd one for reading(0) 	and one for writing(1). We then make a fork for each command,where the child process handles redirection logic and execution while the parent 	process handles waiting for the child and closing pipes. Specifically, in the child process if the command wer're referencing isn't the first 	command we use dup2 to redirect input and if the command isn't the last we use dup2 to redirect output, and this logic carries over until we 	reach the last command. All pipe file descriptors are also closed after duplicating to avoidleakage. When it came to the logic for redirection 	I was originally parsing by white space and checking each token to see if it was > or <. This soon took a turn for the worse when I realized 	cat > output.txt is the same as cat>outuput.txt, so the logic had to be revised. I ended up having to iterate over the entire command
        character by character looking out for > or <. When either character was encountered, the subsequent character after the redirection character 	was assigned as the input or output file (depending on > or <). This logic is used in myshell.c but in pipeline.c I looked for the redirection 	characters using tokens and parsing by spaces. Each time we parsed we would look to check if the token was equal > or < and then similarly 	place the subsequent character as the file for input or output. I was having is determining which implementation was better and ran out of 	time, that is why the two file have different logic used. This could be the reason I didn't pass test case 5.
        After correctly piping and determinging if redirection occurs, we have file redirection.

REDIRECTION
        Depending on if we're reading or writing, we open the input or ouput file with special arguments for each case. If we're opening a file we 	want to write to, the arguments contain, the (file itself, O_WRONLY | O_CREAT | O_TRUNC, 0644), the reason we need to do this was mentioned in 	lecture but also included in the man pages for open(), specifically when we're writing to a file. The O_CREAT ensure that we can create a file 	if it doesn't exist, and the O_TRUNC overwrites previous data written to the file, and the 0644 is for permission reasons, 6 allows the owner 	to read and write, 4 lets the group read, and the last 4 allows others not in the group to also read the file. We also have to use dup2 in 	order to correctly redirect STDIN or STDOUT to the respective file we're openeing, which is then immediately followed by close() with the 	respective file to ensure no leakage and for good coding practice. This logic is the same between pipeline.c and myshell.c. One issue I 	encountered with this is that when I originally wrote to a file, the file would include the contents i wrote to it, such as foo if I typed 	echo foo > output.txt, but it would also cotain the next my_shell$ prompt. In order to fix this, I had to save the original STDOUT
        to a saved_stdout variable before redirecting output (or input) in order to restore the STDOUT later and have the prompt correctly outputted 	to the terminal. I just used dup(saved_stdout, STDOUT) in order to preserve the original STDOUT, then after executing commands, I used dup2() 	to restore STDOUT and then closed saved_stdout. This was also done for stdin cases. This was the basic logic used for redirection, and it was 	implemented in both pipeline.c and myshell.c to ensure redirection worked with piping.


AMPERSAND
	The last feature I had to ensure worked correctly was the use of & and handling zombie processes using sigaction and sigchild. When the & 	character was included as the last character in the last command, we had to make sure the parent process did not wait for the child process to 	end and also had to handle child processes (potentially) turning into zombie processes. In order to do this I checked whether the last 		character in the last argument was equal to & and if so a variable amp was set to 1 and the & was removed from the command so it would
        not be considered during execution. In the parent I included an if statement that checked if amp was set to 1 and if so, we would not wait for 	the child process to terminate before executing the next command. In order to handle zombie proccesses, i had to set up a signal handler for 	SIGCHILD, and this part, although somewhat simple looking back on it, was one of the parts of the assignment and it required me asking for 	help from a TA, as well as test case 5. I ended up using the sigaction struct, which we discussed in class, and my signal handler (sigchild)
        to help. The function sigchild, uses waitpid with the special argument WNOHANG passes which allows the root process to adopt the child process 	and terminate it correctly. This implementation was also implemented in pipeline.c for obvious reasons.


SMALL CHANGES
	The only other notable changes I made was adding more comments throughout the code and turning trim_spaces() and collapse_spaces() into their 	own functions which could be referenced by including their header files. I would have done this for the mini deadline but I was having trouble 	with the Makefile at that time.
