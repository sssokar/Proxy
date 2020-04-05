#include <proxy.h>

t_info_process	info_process = {0};

void	handler_signal(int signal)
{
	switch(signal)
	{
		case SIGTERM:
			fprintf(stderr, "ALERT SECURITY\n");
			break;
	}
}

int	main(int __attribute__((unused))argc, char ** __attribute__((unused))argv)
{
	signal(SIGTERM, handler_signal);

	info_process.pid = getpid();

	char	*str = calloc(sizeof(char), 1024);
	if (str == NULL)
		exit(1);

	sprintf(str, "/proc/%d/maps", info_process.pid);
	const char *cmd_cat[] 	= { "/bin/cat", str, 0 };
  	const char *cmd_awk[] 	= { "/usr/bin/awk", "{print $6}", 0 };
  	t_cmds cmd[] = { {cmd_cat}, {cmd_awk}};

	info_process.args_files = execute_cmds(2, cmd);
	
	return (0);
}
