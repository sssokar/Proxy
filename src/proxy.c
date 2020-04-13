#include <proxy.h>

t_info_process	info_process = {0};

// ---------------------- //
// -- [START] SECURITY -- //
// ---------------------- //
static void	handler_signal(int signal)
{
	switch(signal)
	{
		case SIGTERM:
			// -- Delete Rootkit -- //
			// -------------------- //
			// -------------------- //
			// -------------------- //

			fprintf(stderr, "*** ALERT SECURITY ***\n");
			// -- Delete info process -- //
			//for(uint8_t i = 0; info_process.args_files[i]; i++)
			//	remove(info_process.args_files[i]);

			kill(info_process.pid, SIGKILL);
			break;
	}
}
// -------------------- //
// -- [END] SECURITY -- //
// -------------------- //



int	main(int __attribute__((unused))argc, char ** __attribute__((unused))argv)
{

	sleep(15);

	signal(SIGTERM, handler_signal);

	if (argv[1] == NULL) {
		fprintf(stderr, "Bad parameter\t\t[NOK]");
		exit(EXIT_FAILURE);
	}

	char	*key = argv[1];

	info_process.pid = getpid();

	char	*str = calloc(sizeof(char), 1024);
	if (str == NULL)
		exit(1);

	sprintf(str, "/proc/%d/maps", info_process.pid);

	const char *cmd_cat[] 	= { "/bin/cat", str, key, 0 };
  	const char *cmd_awk[] 	= { "/usr/bin/awk", "{print $6}", key, 0 };
  	t_cmds cmd[] = { {cmd_cat}, {cmd_awk}};

	info_process.args_files = execute_cmds(2, cmd);
	while(1);
	return (0);
}
