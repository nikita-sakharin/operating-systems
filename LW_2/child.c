#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "header.h"

#define BUFFER_SIZE 80u

int main(int argc, const char *argv[])
{
	if (argc < 2)
	{
		fprintf(stderr, "argc = %d\n", argc);
		exit(EXIT_FAILURE);
	}

	int result;

	int out_stream = creat(argv[1u], S_IRWXO | S_IRWXG | S_IRWXU);
	err_handler(out_stream, -1, "open()");

	for (ssize_t local_result = 0; local_result > -1;)
	{
		IntType temp;
		local_result = read(STDIN_FILENO, &temp, sizeof(IntType));
		if (!local_result)
		{
			break;
		}
		err_handler(local_result != sizeof(IntType), 1, "read()");

		char buffer[BUFFER_SIZE];
		result = snprintf(buffer, BUFFER_SIZE, "%"PRI_IntType"\n", temp);
		local_result = write(out_stream, buffer, result);
		err_handler(local_result != result, 1, "write()");
	}

	result = close(out_stream);
	err_handler(result, EOF, "fclose()");

	return 0;
}