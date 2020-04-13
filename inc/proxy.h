#ifndef PROXY_H
#define PROXY_H
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include <fcntl.h>

typedef struct 		s_info_process
{
	uint32_t	pid;
	char		**args_files;

}			t_info_process;

typedef struct		s_cmds
{
	const char	**argv;
}			t_cmds;

char	**split(char *, uint8_t);
char	**execute_cmds(uint16_t, t_cmds *);
long	execute_simple_cmds(t_cmds *);
#endif
