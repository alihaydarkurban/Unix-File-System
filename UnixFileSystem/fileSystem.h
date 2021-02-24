#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <ctime>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

#define MAX_PATH_SIZE 192		// Maximum path size
#define SuperBlockItems 7		// SuperBlock items count
#define DirectBlocksNum 10		// direct_block size (it means that the number of direct block does the file system have)	
#define _1KB 1024			 	// 1 kilobyte = 1024 bytes
#define _1MB 1048576 		 	// _1KB * _1KB
#define FileNameLength 16		// Maximum file name size
#define OP_SIZE 10				// It keeps the number of operation does the file system have
#define OP_STR_SIZE 10			// Maximum size for operation char

const char split[2] = "/";
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

struct Directory // 20 bytes					
{
	int i_node_number;				// 4 bytes
	char file_name[FileNameLength]; // 16bytes
};

struct SuperBlock // 28 bytes
{
	int block_size;				// 4 bytes
	int i_node_position;		// 4 bytes
	int bitmap_position;		// 4 bytes
	int block_position;			// 4 bytes
	int amount_of_block;		// 4 bytes
	int amount_of_i_nodes;		// 4 bytes
	int bitmap_inode_positon;	// 4 bytes
};

struct iNode // 268 bytes
{
	int i_node_id;						// 4 bytes
	int parent_inode_id;				// 4 bytes
	int size_of_file; 					// 4 bytes
	int type; 							// 4 bytes		// 0 means that directory, 1 means that file, -1 means that not initialized
	int direct_block[DirectBlocksNum]; 	// 40 bytes		// -1 means that not initialize
	time_t last_modification;			// 8 bytes
	char file_name[FileNameLength];		// 16 bytes
	char lnsym_path[MAX_PATH_SIZE];		// 192 bytes
};

struct BitMapBlock // 4096 bytes
{
	int max_bitmap_block[_1KB]; 	// 4096 bytes 		// 0 means that empty, 1 means that full -1 means that end
};

#endif