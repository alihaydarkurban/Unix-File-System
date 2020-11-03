#ifndef FileSystem_H
#define FileSystem_H
#include <ctime>

#define SuperBlockItems 7
#define iNodeBlockSize 10
#define _1KB 1024 // 1 kilobyte = 1024 bytes
#define _1MB 1048576 // _1KB * _1KB
#define FileNameLenght 128

struct Directory
{
	int i_node_number;
	char file_name[FileNameLenght];
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
	int size_of_file; 
	int type; // 0 means that directory, 1 means that file, -1 means that not initialized
	int number_of_block[iNodeBlockSize]; // -1 means that not initialize
	time_t last_modification;
	char file_name[FileNameLenght];
	
};

struct BitMapBlock
{
	int max_bitmap[_1KB]; // 0 means that empty, 1 means that full -1 means that end
};


#endif