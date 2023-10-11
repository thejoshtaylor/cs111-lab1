#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
	// Keep track of our last output file
	int lastOut = STDIN_FILENO;
	// Create our first pipe
	int pipefd[2];
	pipe(pipefd);

	// Loop through all arguments given to us from the command line
	for (int i = 1; i < argc; i++)
	{
		// Fork the process
		int p = fork();
		
		// Child process
		if (p == 0)
		{
			// Redirect STDIN and STDOUT to our pipes
			dup2(lastOut, STDIN_FILENO);
			if (i != argc - 1)
				dup2(pipefd[1], STDOUT_FILENO);

			// Change the process to the commanded process
			execlp(argv[i], argv[i], NULL);

			// If the command is not found, return an error code
			return 1;
		}
		// Parent process
		else
		{
			// Wait for the child process to end and get the status code
			int status = 0;
			waitpid(p, &status, 0);
			status = WEXITSTATUS(status);

			// If we have a failed response
			if (status != 0)
				return status;
			
			// Close the pipe that the child was using for STDIN
			if (lastOut != STDIN_FILENO)
				close(lastOut);
			// Make the pipe that the child was using for STDOUT the new STDIN for the next process
			lastOut = pipefd[0];

			// Cleanup our old pipe and open a new one
			close(pipefd[1]);
			pipe(pipefd);
		}
	}
	close(pipefd[0]);
	close(pipefd[1]);
	// Only close the lastOut if it was modified
	if (argc > 1)
		close(lastOut);
	else
		// No parameters, successful termination
		return 0;
}
