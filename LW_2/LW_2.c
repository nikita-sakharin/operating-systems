#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "header.h"

#define PIPE_MODE_COUNT 2u

#define READ 0u
#define WRITE 1u

typedef struct
{
	pid_t process;
	int pipe_fd[PIPE_MODE_COUNT];
} ProcessPipe;

void str_fscan(FILE *, char *, size_t);

int main(void)
{
	int result;

	const char *child = "child";

	size_t process_count;
	result = scanf("%zu", &process_count);
	err_handler(result, EOF, "scanf()");

	ProcessPipe *process_pipe = malloc(process_count * sizeof(ProcessPipe));
	err_handler(process_pipe, NULL, "malloc()");

	for (size_t i = 0u; i < process_count; ++i)
	{
		result = pipe(process_pipe[i].pipe_fd);
		err_handler(result, -1, "pipe()");
	}

	for (size_t i = 0u; i < process_count; ++i)
	{
		char out_name[FILENAME_MAX];
		str_fscan(stdin, out_name, FILENAME_MAX);

		process_pipe[i].process = fork();
		err_handler(process_pipe[i].process, (pid_t) -1, "fork()");

		if (!process_pipe[i].process)
		{
			result = dup2(process_pipe[i].pipe_fd[READ], STDIN_FILENO);
			err_handler(result, -1, "dup2()");

			for (size_t j = 0u; j < process_count; ++j)
			{
				result = close(process_pipe[j].pipe_fd[READ]);
				err_handler(result, -1, "close()");

				result = close(process_pipe[j].pipe_fd[WRITE]);
				err_handler(result, -1, "close()");
			}

			execl(child, child, out_name, NULL);
		}
	}

	for (size_t i = 0u; i < process_count; ++i)
	{
		result = close(process_pipe[i].pipe_fd[READ]);
		err_handler(result, -1, "close()");
	}

	IntType num_count;
	result = scanf("%"PRI_IntType, &num_count);
	err_handler(result, EOF, "scanf()");

	for (IntType i = 1; i <= num_count; ++i)
	{
		for (size_t j = 0u; j < process_count; ++j)
		{
			ssize_t byte_cnt = write(process_pipe[j].pipe_fd[WRITE], &i, sizeof(IntType));
			err_handler(byte_cnt, (ssize_t) -1, "write()");
		}
	}

	for (size_t i = 0u; i < process_count; ++i)
	{
		result = close(process_pipe[i].pipe_fd[WRITE]);
		err_handler(result, -1, "close()");

		result = waitpid(process_pipe[i].process, NULL, 0);
		err_handler(result, -1, "waitpid()");
	}

	free(process_pipe);

	return 0;
}

#define FORMAT_LENGTH 80u

void str_fscan(FILE *in, char *str, size_t length)
{
	if (!length)
	{
		fprintf(stderr, "str_fscan(): length = %zu\n", length);
		exit(EXIT_FAILURE);
	}

	int result;

	char format[FORMAT_LENGTH];
	result = snprintf(format, FORMAT_LENGTH, "%%%zus", length - 1u);
	err_handler(result < 0, 1, "fscanf()");

	result = fscanf(in, format, str);
	err_handler(result, EOF, "fscanf()");
}