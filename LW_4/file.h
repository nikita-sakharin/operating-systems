#ifndef _FILE_H_
#define _FILE_H_

typedef struct file File;

#define FILE_SEEK_SET 0
#define FILE_SEEK_CUR 1
#define FILE_SEEK_END 2

File *file_open(const char *);
int file_close(File *);

int file_error(File *);
void file_clearerr(File *);
int file_eof(File *);

size_t file_read(void *, size_t, size_t, File *);
size_t file_write(const void *, size_t, size_t, File *);

int file_getchar(File *);
int file_putchar(int, File *);

int file_seek(File *, long, int);
long file_tell(const File *);

void file_setbuf(File *, size_t);

#endif