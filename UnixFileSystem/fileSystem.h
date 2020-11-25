#ifndef FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include <ctime>
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

using namespace std;

#define MAX_PATH_SIZE 512
#define SuperBlockItems 7
#define DirectBlocksNum 10
#define _1KB 1024 // 1 kilobyte = 1024 bytes
#define _1MB 1048576 // _1KB * _1KB
#define FileNameLength 16
#define OP_SIZE 10
#define OP_STR_SIZE 10

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

struct Directory
{
	int i_node_number;
	char file_name[FileNameLength];
};

struct SuperBlock
{
	int block_size;
	int i_node_position;
	int bitmap_position;
	int block_position;
	int amount_of_block;
	int amount_of_i_nodes;
	int bitmap_inode_positon;
};

struct iNode
{
	int i_node_id;
	int parent_inode_id;
	int size_of_file; 
	int type; // 0 means that directory, 1 means that file, -1 means that not initialized
	int direct_block[DirectBlocksNum]; // -1 means that not initialize
	time_t last_modification;
	char file_name[FileNameLength];
	
};

struct BitMapBlock
{
	int max_bitmap_block[_1KB]; // 0 means that empty, 1 means that full -1 means that end
};

#endif