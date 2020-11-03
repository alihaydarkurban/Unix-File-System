#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

using namespace std;

#define OP_SIZE 10
#define OP_STR_SIZE 10

const char operations [][OP_STR_SIZE] = {
	"list", 
	"mkdir", 
	"rmdir", 
	"dumpe2fs", 
	"write", 
	"read", 
	"del", 
	"ln", 
	"lnsym", 
	"fsck"
};

int is_regular_operation(const char *op);
void list(const char *file_system, const char *path);
void mkdir(const char *file_system, const char *path_and_dir);
void rmdir(const char *file_system, const char *path_and_dir);
void dumpe2fs(const char *file_system);
void write(const char *file_system, const char *path, const char *file);
void read(const char *file_system, const char *path, const char *file);
void del(const char *file_system, const char *file);
void ln(const char *file_system, const char *source, const char *dest);
void lnsym(const char *file_system, const char *source, const char *dest);
void fsck(const char *file_system);

int main(int argc, char const *argv[])
{
	int is_operation;

	if(argc < 3 || argc > 5)
	{
		printf("File System Error!\n");
		printf("Runnable format is ./fileSystemOper [x] [y] [z optional] [q optional]\n");
		return -1;
	}

	// mySystem.data acilabiliyormu vs kontrolleri

	is_operation = is_regular_operation(argv[2]);

	switch(is_operation)
	{
		case 0 : list(argv[1], argv[3]); break;
		case 1 : mkdir(argv[1], argv[3]); break;
		case 2 : rmdir(argv[1], argv[3]); break;
		case 3 : dumpe2fs(argv[1]); break;
		case 4 : write(argv[1], argv[3], argv[4]); break;
		case 5 : read(argv[1], argv[3], argv[4]); break;
		case 6 : del(argv[1], argv[3]); break;
		case 7 : ln(argv[1], argv[3], argv[4]); break;
		case 8 : lnsym(argv[1], argv[3], argv[4]); break;
		case 9 : fsck(argv[1]); break;	
		default:
		{
			cout << "File System Error!" << endl;
			cout << "\"" << argv[2] << "\" is not a runnable operation" << endl;
		}

	}

	return 0;
}


int is_regular_operation(const char * op)
{
	// -1 means that op is not a regular operation
	int i;
	int result = 0;

	for(i = 0; i < OP_SIZE; ++i)
	{
		if(strcmp(op, operations[i]) == 0)
			return i;
	}
	return -1;
}


void list(const char *file_system, const char *path){ }
void mkdir(const char *file_system, const char *path_and_dir){ }
void rmdir(const char *file_system, const char *path_and_dir){ }
void dumpe2fs(const char *file_system){ }
void write(const char *file_system, const char *path, const char *file){ }
void read(const char *file_system, const char *path, const char *file){ }
void del(const char *file_system, const char *file){ }
void ln(const char *file_system, const char *source, const char *dest){ }
void lnsym(const char *file_system, const char *source, const char *dest){ }
void fsck(const char *file_system){ }