#include <stdio.h>
#include <windows.h>

#define NAME_SIZE 512
#define BUFF_SIZE 65536u

int main(void)
{
	HANDLE stream;
	char name[NAME_SIZE], buffer[BUFF_SIZE];
	DWORD count;

	scanf_s("%[^\n]", name, NAME_SIZE);
	getchar();
	SetCurrentDirectory(name);

	GetCurrentDirectory(NAME_SIZE, name);
	printf("Current dir:\n%s\n\n", name);

	scanf_s("%s", name, NAME_SIZE);
	stream = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError())
	{
		printf("Error\n");
		exit(EXIT_FAILURE);
	}
	scanf_s("%zu", &count);
	ReadFile(stream, buffer, count, &count, NULL);
	CloseHandle(stream);

	scanf_s("%s", name, NAME_SIZE);
	stream = CreateFile(name, GENERIC_READ | GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (GetLastError())
	{
		printf("Error\n");
		exit(EXIT_FAILURE);
	}
	WriteFile(stream, buffer, count, &count, NULL);
	CloseHandle(stream);

	return 0;
}