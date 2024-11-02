#include "sigchild.h"
#include <unistd.h>
#include <sys/wait.h>
#include <signal.h>

void sigchild(int val)
{
    while(waitpid(-1,NULL,WNOHANG) > 0); /*wait until we collect the pid of the zombie process*/
}
