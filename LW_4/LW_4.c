#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "md5.h"
#include "header.h"
#include "file.h"

#define str_make(str) str_make_(str)
#define str_make_(str) #str

#define START_CAPACITY 4u
#define MD5_SIZE 16u

size_t fgetc_range(File *, long, size_t, char *);
void fput_str(const char *, File **, long, const char *);
long fstr(File *, long, const char *);
long fstr_prefix(File *, long, const char *);
long fstr_suffix(File *, long, const char *);
void fchecksum(File *, size_t, void *);
void finfo(File *, long *, long *);

void menu_print(void);
void help_print(const char *, int);

#define PAGE_COUNT 4u

int main(int argc, const char *argv[])
{
	if (argc > 1)
	{
		if (!strcmp(argv[1], "-h"))
		{
			help_print(argv[0], EXIT_SUCCESS);
		}
		else if (!strncmp(argv[1], "-file:", strlen("-file:")))
		{
			if (!strncmp(argv[2], "-command:", strlen("-command:")))
			{
				const char * command = argv[2] + strlen("-command:");
				int status;

				File *stream = file_open(argv[1] + strlen("-file:"));
				err_handler(stream, NULL, "file_open()");

				if (!strncmp(command, "fgetc_range", strlen("fgetc_range")))
				{
					if (argc != 5)
					{
						status = EXIT_FAILURE;
					}
					else
					{
						long begin = strtol(argv[3], NULL, 10);
						long end = strtol(argv[4], NULL, 10);
						const size_t length = end - begin;

						char * const buffer = malloc(length * sizeof(char));
						err_handler(buffer, NULL, "malloc()");

						size_t result = fgetc_range(stream, begin, length, buffer);
						printf("%zu byte was successfully reading\n", result);
						for (size_t i = 0u; i < result; ++i)
						{
							putchar(buffer[i]);
						}
						putchar('\n');

						free(buffer);

						status = EXIT_SUCCESS;
					}
				}
				else if (!strncmp(command, "fput_str", strlen("fput_str")))
				{
					if (argc != 5)
					{
						status = EXIT_FAILURE;
					}
					else
					{
						long line_idx = strtol(argv[3], NULL, 10);
						fput_str(argv[1] + strlen("-file:"), &stream, line_idx, argv[4]);
						status = EXIT_SUCCESS;
					}
				}
				else if (!strncmp(command, "fstr", strlen("fstr")))
				{
					if (argc != 6)
					{
						status = EXIT_FAILURE;
					}
					else
					{
						long offset = strtol(argv[3], NULL, 10);
						int whence = (int) strtol(argv[5], NULL, 10);
						long result;
						switch (whence)
						{
							case 0:
							{
								result = fstr(stream, offset, argv[4]);
								break;
							}
							case 1:
							{
								result = fstr_prefix(stream, offset, argv[4]);
								break;
							}
							case 2:
							{
								result = fstr_suffix(stream, offset, argv[4]);
								break;
							}
							default:
							{
								puts("Incorrect whence");
								break;
							}
						}
						if (whence > -1 && whence < 3)
						{
							printf("string \"%s\" located in position %ld\n", argv[4], result);
							status = EXIT_SUCCESS;
						}
						else
						{
							status = EXIT_FAILURE;
						}
					}
				}
				else if (!strncmp(command, "fchecksum", strlen("fchecksum")))
				{
					char buffer[MD5_SIZE];
					fchecksum(stream, 512, buffer);
					for (size_t i = 0u; i < MD5_SIZE; ++i)
					{
						printf("%hhx", (unsigned char) buffer[i]);
					}
					putchar('\n');
					status = EXIT_SUCCESS;
				}
				else if (!strncmp(command, "finfo", strlen("finfo")))
				{
					long char_count, line_count;
					finfo(stream, &char_count, &line_count);
					printf("%ld characters and %ld lines in file\n", char_count, line_count);
					status = EXIT_SUCCESS;
				}
				else
				{
					status = EXIT_FAILURE;
				}

				int return_value = file_close(stream);
				err_handler(return_value, EOF, "file_close()");

				if (status == EXIT_FAILURE)
				{
					help_print(argv[0], EXIT_FAILURE);
				}
				else
				{
					exit(EXIT_SUCCESS);
				}
			}
			else
			{
				help_print(argv[0], EXIT_FAILURE);
			}
		}
		else
		{
			help_print(argv[0], EXIT_FAILURE);
		}
	}

	char name[FILENAME_MAX];
	puts("Enter the name of file:");
	scanf("%"str_make(FILENAME_MAX)"[^\n]", name);
	File *stream = file_open(name);
	err_handler(stream, NULL, "file_open()");

	int op_code = 0;
	while (op_code != -1)
	{
		menu_print();
		scanf("%d", &op_code);
		getchar();

		switch (op_code)
		{
			case 1:
			{
				puts("[begin; end)");
				long begin, end;
				puts("begining position:");
				scanf("%ld", &begin);
				puts("ending position:");
				scanf("%ld", &end);
				if (end < begin)
				{
					puts("Wrong input");
					break;
				}
				else if (end - begin)
				{
					const size_t length = end - begin;
					char * const buffer = malloc(length * sizeof(char));
					if (buffer == NULL)
					{
						puts("Too big range, insufficient memory");
					}

					size_t result = fgetc_range(stream, begin, length, buffer);
					printf("%zu byte was successfully reading\n", result);
					for (size_t i = 0u; i < result; ++i)
					{
						putchar(buffer[i]);
					}

					free(buffer);
				}
				putchar('\n');

				break;
			}
			case 2:
			{
				int ch = 0;
				size_t count = 0u, capacity = START_CAPACITY;
				char *buffer = malloc(capacity * sizeof(char));
				if (buffer == NULL)
				{
					puts("Insufficient memory");
					break;
				}

				puts("Enter the pattern:");
				while (ch != '\n' && ch != EOF)
				{
					buffer[count] = ch = getchar();
					if (ch == '\n')
					{
						buffer[count] = '\0';
						break;
					}
					++count;
					if (count >= capacity)
					{
						capacity *= 2u;
						char *result = realloc(buffer, capacity * sizeof(char));
						if (result == NULL)
						{
							puts("Insufficient memory");
							free(buffer);
							ch = EOF;
						}
						buffer = result;
					}
				}

				if (ch == EOF)
				{
					break;
				}

				puts("Enter the offset in file:");
				long offset;
				scanf("%ld", &offset);

				puts("Enter the whence:");
				puts("2 search by suffix");
				puts("1 search by prefix");
				puts("0 for default search");
				int whence;
				scanf("%d", &whence);

				long result;
				switch (whence)
				{
					case 0:
					{
						result = fstr(stream, offset, buffer);
						break;
					}
					case 1:
					{
						result = fstr_prefix(stream, offset, buffer);
						break;
					}
					case 2:
					{
						result = fstr_suffix(stream, offset, buffer);
						break;
					}
					default:
					{
						puts("Incorrect whence, please try again");
						break;
					}
				}

				if (whence < 3 && whence > -1)
				{
					if (result == -1l)
					{
						puts("No such string");
					}
					else
					{
						printf("string \"%s\" located in position %ld\n", buffer, result);
					}
					puts("Success");
				}

				free(buffer);

				break;
			}
			case 3:
			{
				int ch = 0;
				size_t count = 0u, capacity = START_CAPACITY;
				char *buffer = malloc(capacity * sizeof(char));
				if (buffer == NULL)
				{
					puts("Insufficient memory");
					break;
				}

				puts("Enter string:");
				while (ch != '\n' && ch != EOF)
				{
					buffer[count] = ch = getchar();
					if (ch == '\n')
					{
						buffer[count] = '\0';
						break;
					}
					++count;
					if (count >= capacity)
					{
						capacity *= 2u;
						char *result = realloc(buffer, capacity * sizeof(char));
						if (result == NULL)
						{
							puts("Insufficient memory");
							free(buffer);
							ch = EOF;
						}
						buffer = result;
					}
				}

				if (ch == EOF)
				{
					break;
				}

				puts("Enter the line index (begin from 1)");
				long offset;
				scanf("%ld", &offset);

				fput_str(name, &stream, offset, buffer);

				free(buffer);

				puts("Success");

				break;
			}
			case 4:
			{
				size_t capacity;
				puts("Enter the capacity of the calculational buffer:");
				scanf("%zu", &capacity);

				char buffer[MD5_SIZE];
				fchecksum(stream, capacity, buffer);
				for (size_t i = 0u; i < MD5_SIZE; ++i)
				{
					printf("%hhx", (unsigned char) buffer[i]);
				}
				putchar('\n');
				putchar('\n');
				puts("Success");

				break;
			}
			case 5:
			{
				long char_count, line_count;
				finfo(stream, &char_count, &line_count);
				printf("%ld characters and %ld lines in file\n", char_count, line_count);
				puts("Success");

				break;
			}
			case 6:
			{
				size_t buffer_size;
				puts("Enter the size of buffer in byte:");
				scanf("%zu", &buffer_size);
				if (!buffer_size)
				{
					puts("Wrong size");

					break;
				}

				file_setbuf(stream, buffer_size);
				puts("Success");

				break;
			}
			case -1:
			{
				puts("Exiting");
				break;
			}
			case 0:
			{
				int return_value = file_close(stream);
				err_handler(return_value, EOF, "file_close()");

				puts("Enter the name of new file:");
				scanf("%"str_make(FILENAME_MAX)"[^\n]", name);
				stream = file_open(name);
				err_handler(stream, NULL, "file_open()");
				puts("Success");

				break;
			}
			default:
			{
				puts("Wrong operation code, please try again");
				break;
			}
		}

		putchar('\n');
	}

	int return_value = file_close(stream);
	err_handler(return_value, EOF, "file_close()");

	puts("Done");

	return 0;
}

size_t fgetc_range(File * restrict stream, long offset, size_t count, char * restrict buffer)
{
	int return_value;

	long old_pos = file_tell(stream);
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(stream, offset, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	size_t result = file_read(buffer, sizeof(char), count, stream);
	err_handler(!file_error(stream), 0, "file_read()");

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	return result;
}

void fput_str(const char * restrict name, File ** restrict stream, long line_idx, const char * restrict string)
{
	static char *temp_name = NULL;
	if (temp_name == NULL)
	{
		temp_name = tmpnam(NULL);
		err_handler(temp_name, NULL, "tmpnam()");
	}

	int return_value;

	long old_pos = file_tell(*stream);
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(*stream, 0l, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	long line_counter = 1l, line_begining = -1l, next_line_begining = -1l;
	if (line_idx == 1l)
	{
		line_begining = 0l;
	}
	for (long i = 0l; line_counter <= line_idx; ++i)
	{
		return_value = file_getchar(*stream);
		err_handler(!file_error(*stream), 0, "file_getchar()");
		if (file_eof(*stream))
		{
			break;
		}
		if (return_value == '\n')
		{
			++line_counter;
			if (line_counter == line_idx)
			{
				line_begining = i + 1l;
			}
			else if (line_counter > line_idx)
			{
				next_line_begining = i + 1l;
			}
		}
	}

	const size_t length = strlen(string);

	if (line_begining == -1)
	{
		return_value = file_seek(*stream, 0l, SEEK_END);
		err_handler(!return_value, 0, "file_seek()");

		for (; line_counter < line_idx; ++line_counter)
		{
			return_value = file_putchar('\n', *stream);
			err_handler(return_value, EOF, "file_putchar()");
		}

		file_write(string, sizeof(char), length, *stream);

		return;
	}

	File *temp_file = file_open(temp_name);
	err_handler(temp_file, NULL, "file_open()");

	return_value = file_seek(*stream, 0l, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	for (long i = 0l; i < line_begining; ++i)
	{
		return_value = file_getchar(*stream);
		err_handler(!file_error(*stream), 0, "file_getchar()");

		return_value = file_putchar(return_value, temp_file);
		err_handler(!file_error(temp_file), 0, "file_putchar()");
	}

	long pos;
	if (old_pos >= line_begining && old_pos <next_line_begining)
	{
		pos = file_tell(temp_file);
		err_handler(pos, -1l, "file_tell()");
	}
	file_write(string, sizeof(char), length, temp_file);
	err_handler(!file_error(temp_file), 0, "file_write()");
	if (old_pos > next_line_begining)
	{
		pos = file_tell(temp_file);
		err_handler(pos, -1l, "file_tell()");
		++pos;
	}

	return_value = file_seek(*stream, next_line_begining, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");
	return_value = '\n';
	while (!file_eof(*stream))
	{
		return_value = file_putchar(return_value, temp_file);
		err_handler(!file_error(temp_file), 0, "file_putchar()");

		return_value = file_getchar(*stream);
		err_handler(!file_error(*stream), 0, "file_getchar()");
	}

	return_value = file_close(temp_file);
	err_handler(return_value, EOF, "file_close()");

	return_value = file_close(*stream);
	err_handler(return_value, EOF, "file_close()");

	return_value = remove(name);
	err_handler(!return_value, 0, "remove()");
	return_value = rename(temp_name, name);
	err_handler(!return_value, 0, "rename()");

	*stream = file_open(name);
	err_handler(*stream, NULL, "file_open()");
	if (old_pos < line_begining)
	{
		return_value = file_seek(*stream, old_pos, SEEK_SET);
		err_handler(!return_value, 0, "file_seek()");
	}
	else if (old_pos > next_line_begining)
	{
		return_value = file_seek(*stream, pos + old_pos - next_line_begining, SEEK_SET);
		err_handler(!return_value, 0, "file_seek()");
	}
	else
	{
		return_value = file_seek(*stream, pos, SEEK_SET);
		err_handler(!return_value, 0, "file_seek()");
	}
}

long fstr(File * restrict stream, long offset, const char * restrict pattern)
{
	int return_value;

	const size_t length = strlen(pattern);
	if (!length)
	{
		return -1l;
	}

	long old_pos = file_tell(stream), result = -1l;
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(stream, offset, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	char * const buffer = malloc((length + 1u) * sizeof(char));
	err_handler(buffer, NULL, "malloc()");

	file_read(buffer, sizeof(char), length, stream);
	err_handler(!file_error(stream), 0, "file_getchar()");
	buffer[length] = '\0';

	while (!file_eof(stream))
	{
		return_value = strcmp(pattern, buffer);
		if (!return_value)
		{
			result = file_tell(stream);
			err_handler(result, -1l, "file_tell()");
			result -= length;
			break;
		}

		return_value = file_getchar(stream);
		err_handler(!file_error(stream), 0, "file_getchar()");
		if (file_eof(stream))
		{
			break;
		}

		if (length > 1u)
		{
			memmove(buffer, buffer + 1u, length - 1u);
		}
		buffer[length - 1u] = return_value;
	}

	free(buffer);

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	return result;
}

long fstr_prefix(File * restrict stream, long offset, const char * restrict pattern)
{
	int return_value;

	const size_t length = strlen(pattern);
	if (!length)
	{
		return -1l;
	}

	long old_pos = file_tell(stream), result = -1l;
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(stream, offset, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	char * const buffer = malloc((length + 1u) * sizeof(char));
	err_handler(buffer, NULL, "malloc()");

	file_read(buffer, sizeof(char), length, stream);
	err_handler(!file_error(stream), 0, "file_getchar()");
	buffer[length] = '\0';

	int prev_char = EOF;
	while (!file_eof(stream))
	{
		return_value = strcmp(pattern, buffer);
		if (!return_value && (prev_char == '\n' || prev_char == EOF))
		{
			result = file_tell(stream);
			err_handler(result, -1l, "file_tell()");
			result -= length;
			break;
		}

		return_value = file_getchar(stream);
		err_handler(!file_error(stream), 0, "file_getchar()");
		if (file_eof(stream))
		{
			break;
		}

		prev_char = buffer[0u];
		if (length > 1u)
		{
			memmove(buffer, buffer + 1u, length - 1u);
		}
		buffer[length - 1u] = return_value;
	}

	free(buffer);

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	return result;
}

long fstr_suffix(File * restrict stream, long offset, const char * restrict pattern)
{
	int return_value;

	const size_t length = strlen(pattern);
	if (!length)
	{
		return -1l;
	}

	long old_pos = file_tell(stream), result = -1l;
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(stream, offset, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	char * const buffer = malloc((length + 1u) * sizeof(char));
	err_handler(buffer, NULL, "malloc()");

	file_read(buffer, sizeof(char), length, stream);
	err_handler(!file_error(stream), 0, "file_getchar()");
	buffer[length] = '\0';

	int next_char = EOF;
	while (!file_eof(stream))
	{
		next_char = file_getchar(stream);
		err_handler(!file_error(stream), 0, "file_getchar()");

		return_value = strcmp(pattern, buffer);
		if (!return_value && (next_char == '\n' || next_char == EOF))
		{
			result = file_tell(stream);
			err_handler(result, -1l, "file_tell()");
			result -= length;
			break;
		}

		if (file_eof(stream))
		{
			break;
		}

		if (length > 1u)
		{
			memmove(buffer, buffer + 1u, length - 1u);
		}
		buffer[length - 1u] = next_char;
	}

	free(buffer);

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	return result;
}

void fchecksum(File * restrict stream, size_t capacity, void * restrict md5)
{
	int return_value;

	void * const buffer = malloc(capacity);
	err_handler(buffer, NULL, "malloc()");

	MD5_CTX temp;
	MD5_Init(&temp);

	long old_pos = file_tell(stream);
	err_handler(old_pos, -1l, "file_tell()");

	return_value = file_seek(stream, 0l, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	for (size_t byte_cnt = capacity; byte_cnt && !file_eof(stream);)
	{
		byte_cnt = file_read(buffer, sizeof(char), capacity, stream);
		err_handler(!file_error(stream), 0, "file_read()");

		if (!byte_cnt)
		{
			break;
		}

		MD5_Update(&temp, buffer, byte_cnt);
	}

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	MD5_Final(md5, &temp);

	free(buffer);
}

void finfo(File * restrict stream, long * restrict char_count, long * restrict line_count)
{
	int return_value;

	long old_pos = file_tell(stream);
	err_handler(old_pos, -1l, "file_tell()");
	return_value = file_seek(stream, 0l, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");

	for (long char_counter = 0l, line_counter = 1l; !file_eof(stream); ++char_counter)
	{
		return_value = file_getchar(stream);
		err_handler(!file_error(stream), 0, "file_getchar()");

		if (file_eof(stream))
		{
			*char_count = char_counter;
			*line_count = line_counter;
			break;
		}

		if (return_value == '\n')
		{
			++line_counter;
		}
	}

	return_value = file_seek(stream, old_pos, SEEK_SET);
	err_handler(!return_value, 0, "file_seek()");
}

void menu_print(void)
{
	puts("1    print characters in certain range");
	puts("2    search string in file");
	puts("3    changing string in certain index");
	puts("4    calculate md5 checksum");
	puts("5    info");
	puts("6    set buffer size");
	puts("0    change file");
	puts("-1    for exit");
	putchar('\n');
}

void help_print(const char * restrict name, int status)
{
	if (status)
	{
		puts("Wrong usage");
	}

	printf("%s -file:FILE -command:COMMAND [COMMAND_OPTION]\n", name);
	puts("COMMAND:");
	puts("fgetc_range BEGIN END");
	puts("fput_str LINE_INDEX LINE_NEW_CONTENT");
	puts("fstr OFFSET PATTREN [ 0 = default | 1 = prefix | 2 = suffix ]");
	puts("fchecksum");
	puts("finfo");

	exit(status);
}