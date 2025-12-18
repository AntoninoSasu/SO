#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

/* programa que temporiza la ejecución de un proceso hijo */

pid_t p_id;

void sig_handler(int a) {
	if (kill(p_id, SIGKILL) == -1) {
		perror("Error on kill()\n");
		exit(-1);
	}
}

int main(int argc, char** argv) {
	if ((p_id = fork()) == -1) {
		perror("Error on creating child process\n");
		return -1;
	} else if (p_id == 0) {
		execvp(argv[1], argv + 1);
		perror("Error on execvp()\n");
		return -1;
	}

	{
		struct sigaction sa;
		sa.sa_handler = sig_handler;
		sa.sa_flags = SA_RESTART;

		if (sigaction(SIGALRM, &sa, NULL) == -1) {
			perror("Error on sigaction()\n");
			return -1;
		}
	}
	
	{
		struct sigaction sa;
		sa.sa_handler = SIG_IGN;

		if (sigaction(SIGALRM, &sa, NULL) == -1) {
			perror("Error on sigaction()\n");
			return -1;
		}
	}

	alarm(5);

	int status;
	if (wait(&status) == -1) {
		perror("Error on wait()");
		return -1;
	}

	if (WIFEXITED(status)) {
		printf("Child process finished normally\n");
	} else if (WIFSIGNALED(status)) {
		printf("Child process terminated via signal\n");
	} else {
		perror("Child process terminated unexpectedly\n");
		return -1;
	}

	return 0;
}