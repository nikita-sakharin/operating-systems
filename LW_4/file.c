#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "file.h"

#define max(A, B) ((A) > (B) ? (A) : (B))
#define min(A, B) ((A) < (B) ? (A) : (B))

#define err_handler(curr_value, wrong_value, func_name_str)\
	if ((curr_value) == (wrong_value))\
	{\
		perror(func_name_str);\
		exit(EXIT_FAILURE);\
	}\

struct file
{
	off_t file_size;
	off_t position;
	void *map;
	size_t offset_in_map;
	size_t sizeof_map;
	int file_description;
	bool map_read_or_write;
	bool error;
	bool eof;
};

#define MAP_DEFAULT_SIZE 2u
#define READ false
#define WRITE true

File *file_open(const char * restrict name)
{
	File *result = malloc(sizeof(File));
	if (result == NULL)
	{
		return NULL;
	}

	result->file_description = open(name, O_RDWR | O_CREAT | O_APPEND, S_IRWXO | S_IRWXG | S_IRWXU);
	if (result->file_description == -1)
	{
		free(result);
		return NULL;
	}

	struct stat file_state;
	int return_value = fstat(result->file_description, &file_state);
	if (return_value == -1)
	{
		close(result->file_description);
		free(result);
		return NULL;
	}
	result->file_size = file_state.st_size;
	result->position = (off_t) 0;
	result->map = NULL;
	result->offset_in_map = 0u;
	result->sizeof_map = MAP_DEFAULT_SIZE * sysconf(_SC_PAGE_SIZE);
	result->map_read_or_write = READ;
	result->error = false;
	result->eof = false;

	return result;
}

int file_close(File *stream)
{
	int result = 0, temp;

	if (stream->map != NULL)
	{
		if (stream->map_read_or_write == WRITE && stream->offset_in_map)
		{
			temp = ftruncate(stream->file_description, stream->position + stream->offset_in_map - stream->position % stream->sizeof_map);
			if (temp == -1)
			{
				result = EOF;
			}
		}

		temp = munmap(stream->map, stream->sizeof_map);
		if (temp == -1)
		{
			result = EOF;
		}
	}

	temp = close(stream->file_description);
	if (temp == -1)
	{
		result = EOF;
	}

	free(stream);

	return result;
}

int file_error(File * restrict stream)
{
	return (int) stream->error;
}

void file_clearerr(File * restrict stream)
{
	stream->error = false;
}

int file_eof(File * restrict stream)
{
	return (int) stream->eof;
}

size_t file_read(void * restrict buffer, size_t size, size_t count, File * restrict stream)
{
	size_t i;
	for (i = 0u; i < count * size; ++i)
	{
		int ch = file_getchar(stream);
		if (file_eof(stream) || file_error(stream))
		{
			break;
		}
		((char *) buffer)[i] = ch;
	}

	return i;
}
size_t file_write(const void * restrict buffer, size_t size, size_t count, File * restrict stream)
{
	size_t i;
	for (i = 0u; i < count * size; ++i)
	{
		file_putchar(((char *) buffer)[i], stream);
		if (file_error(stream))
		{
			break;
		}
	}

	return i;
}

int file_getchar(File * restrict stream)
{
	if (stream->map != NULL && ((stream->map_read_or_write == READ && stream->offset_in_map >= stream->sizeof_map) || stream->map_read_or_write == WRITE))
	{
		if (stream->map_read_or_write == READ)
		{
			off_t offset = lseek(stream->file_description, stream->offset_in_map, SEEK_CUR);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return EOF;
			}
			stream->position = offset;
		}
		else
		{
			if (stream->offset_in_map)
			{
				int result = ftruncate(stream->file_description, stream->position + stream->offset_in_map - stream->position % stream->sizeof_map);
				if (result == -1)
				{
					stream->error = true;
					return EOF;
				}
				stream->file_size = stream->position + stream->offset_in_map - stream->position % stream->sizeof_map;
			}
			off_t offset = lseek(stream->file_description, (off_t) 0, SEEK_SET);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return EOF;
			}
			stream->position = offset;
		}

		int return_value = munmap(stream->map, stream->sizeof_map);
		if (return_value == -1)
		{
			stream->error = true;
			return EOF;
		}
		stream->map = NULL;
		stream->offset_in_map = 0u;
	}
	if ((off_t) (stream->position + stream->offset_in_map - stream->position % stream->sizeof_map) >= stream->file_size)
	{
		stream->eof = true;
		return EOF;
	}
	if (stream->map == NULL)
	{
		void *map = mmap(NULL, stream->sizeof_map, PROT_READ | PROT_WRITE, MAP_SHARED, stream->file_description, stream->position / sysconf(_SC_PAGE_SIZE) * sysconf(_SC_PAGE_SIZE));
		if (map == MAP_FAILED)
		{
			stream->error = true;
			return EOF;
		}
		stream->map = map;
		stream->offset_in_map = stream->position % stream->sizeof_map;
		stream->map_read_or_write = READ;
	}

	int result = ((char *) stream->map)[stream->offset_in_map];

	++stream->offset_in_map;

	return result;
}

int file_putchar(int character, File * restrict stream)
{
	if (stream->map != NULL && (stream->map_read_or_write == READ || (stream->map_read_or_write == WRITE && stream->offset_in_map >= stream->sizeof_map)))
	{
		if (stream->map_read_or_write == READ)
		{
			off_t offset = lseek(stream->file_description, (off_t) 0, SEEK_END);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return EOF;
			}
			stream->position = offset;
			stream->eof = false;
		}
		else
		{
			if (stream->offset_in_map >= stream->sizeof_map)
			{
				int result = ftruncate(stream->file_description, stream->position + stream->offset_in_map - stream->position % stream->sizeof_map);
				if (result == -1)
				{
					stream->error = true;
					return EOF;
				}
				stream->file_size = stream->position + stream->offset_in_map - stream->position % stream->sizeof_map;
			}
			off_t offset = lseek(stream->file_description, (off_t) 0, SEEK_END);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return EOF;
			}
			stream->position = offset;
		}

		int result = munmap(stream->map, stream->sizeof_map);
		if (result == -1)
		{
			stream->error = true;
			return EOF;
		}
		stream->map = NULL;
		stream->offset_in_map = 0u;
	}
	if (stream->map == NULL)
	{
		void *map = mmap(NULL, stream->sizeof_map, PROT_READ | PROT_WRITE, MAP_SHARED, stream->file_description, stream->position / sysconf(_SC_PAGE_SIZE) * sysconf(_SC_PAGE_SIZE));
		if (map == MAP_FAILED)
		{
			stream->error = true;
			return EOF;
		}
		stream->map = map;
		stream->offset_in_map = stream->position % stream->sizeof_map;
		stream->map_read_or_write = WRITE;
		int result = ftruncate(stream->file_description, stream->file_size + stream->sizeof_map);
		if (result == -1)
		{
			stream->error = true;
			return EOF;
		}
	}

	((char *) stream->map)[stream->offset_in_map] = (char) character;

	++stream->offset_in_map;

	return (int) (char) character;
}

int file_seek(File * restrict stream, long offset, int whence)
{
	if (stream->map != NULL)
	{
		if (stream->map_read_or_write == READ)
		{
			off_t offset = lseek(stream->file_description, stream->offset_in_map, SEEK_CUR);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return EOF;
			}

			stream->eof = false;
		}
		else
		{
			if (stream->offset_in_map)
			{
				int result = ftruncate(stream->file_description, stream->position + stream->offset_in_map - stream->position % stream->sizeof_map);
				if (result == -1)
				{
					stream->error = true;
					return EOF;
				}
				stream->file_size = stream->position + stream->offset_in_map - stream->position % stream->sizeof_map;
				off_t offset = lseek(stream->file_description, stream->offset_in_map, SEEK_CUR);
				if (offset == (off_t) -1)
				{
					stream->error = true;
					return EOF;
				}
			}
		}

		int return_value = munmap(stream->map, stream->sizeof_map);
		if (return_value == -1)
		{
			stream->error = true;
			return EOF;
		}
		stream->map = NULL;
		stream->offset_in_map = 0u;
	}

	off_t result = lseek(stream->file_description, offset, whence);
	if (result == (off_t) -1)
	{
		stream->error = true;
		return EOF;
	}
	stream->position = result;
	stream->error = false;
	stream->eof = false;

	return 0;
}

long file_tell(const File * restrict stream)
{
	return stream->position + stream->offset_in_map - stream->position % stream->sizeof_map;
}

void file_setbuf(File * restrict stream, size_t sizeof_map)
{
	if (stream->map != NULL)
	{
		if (stream->map_read_or_write == READ)
		{
			off_t offset = lseek(stream->file_description, stream->offset_in_map, SEEK_CUR);
			if (offset == (off_t) -1)
			{
				stream->error = true;
				return;
			}
			stream->position = offset;
		}
		else
		{
			if (stream->offset_in_map >= stream->sizeof_map)
			{
				stream->file_size = stream->position + stream->offset_in_map - stream->position % stream->sizeof_map;
				int result = ftruncate(stream->file_description, stream->file_size);
				if (result == -1)
				{
					stream->error = true;
					return;
				}
				off_t offset = lseek(stream->file_description, stream->offset_in_map, SEEK_CUR);
				if (offset == (off_t) -1)
				{
					stream->error = true;
					return;
				}
				stream->position = offset;
			}
		}

		int result = munmap(stream->map, stream->sizeof_map);
		if (result == -1)
		{
			stream->error = true;
			return;
		}
		stream->map = NULL;
		stream->offset_in_map = 0u;
	}

	stream->sizeof_map = (sizeof_map + sysconf(_SC_PAGE_SIZE) - (sizeof_map != 0)) / sysconf(_SC_PAGE_SIZE) * sysconf(_SC_PAGE_SIZE);
}