#include <proxy.h>

/*!
 * \fn	int spawn_proc (int, int, t_cmds *)
 * \brief [...]
 */

static int spawn_proc (int in, int out, t_cmds *cmd)
{
	pid_t pid;

  	if ((pid = fork()) == 0) {
		if (in != 0){
			dup2 (in, 0);
			close (in);
		}
		if (out != 1) {
			dup2 (out, 1);
			close (out);
		}
		exit(execve (cmd->argv [0], (char * const *)cmd->argv, NULL));
	}

	int	state = -1;
	if (waitpid(pid, &state, WUNTRACED) == -1)
		return (-1);
	if (WIFEXITED(state))
		return (1);
	return -1;
}

char	**execute_cmds(uint16_t n, t_cmds *cmd)
{
  	int		fd[2];
  
  	/* The first process should get its input from the original file descriptor 0.  */
  	int in = 0;

  	/* Note the loop bound, we spawn here all, but the last stage of the pipeline.  */
	uint16_t i = 0;
  	for (i = 0; i < n; ++i) {
		if (pipe(fd) == -1)
			return NULL;

		/* f [1] is the write end of the pipe, we carry `in` from the prev iteration.  */
		if (spawn_proc (in, fd [1], cmd + i) == -1)
			return NULL;

		/* No need for the write end of the pipe, the child will write here.  */
		close (fd [1]);

		/* Keep the read end of the pipe, the next child will read from there.  */
		in = fd [0];
	}

  	/* Last stage of the pipeline - set stdin be the read end of the previous pipe
     	and output to the original file descriptor 1. */
  	if (in != 0)
		  dup2 (in, 0);

	char	*ptr = mmap(NULL, getpagesize()*2, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	if (ptr == MAP_FAILED)
		return NULL;

	while (read(0, ptr, getpagesize()*2))
		;
	return split(ptr, '\n');
}

/*!
 * \fn long execute_simple_cmds(t_cmds *)
 * \brief
 */
long	execute_simple_cmds(t_cmds *cmd)
{
	int pid, state;

  	if ((pid = fork()) == 0)
		exit(execve (cmd->argv [0], (char * const *)cmd->argv, NULL));
	if (waitpid(pid, &state, WUNTRACED) == -1)
		return (-1);
	return (pid);
}
