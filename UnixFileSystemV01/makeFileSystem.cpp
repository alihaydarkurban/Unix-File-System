#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include "fileSystem.h"

using namespace std;

int init_file_system(char *file_system, int block_size, int num_of_i_nodes);
void init_SuperBlock(FILE *file_ptr, int super_blok_items[]);
void init_iNode(FILE *file_ptr, int num_of_i_nodes);
void init_BitMapBlock(FILE *file_ptr, int last_block);
void init_BitMap_iNode(FILE *file_ptr, int num_of_i_nodes);
void calculation(int *used_size, int *remaining_size, int *amount_of_block, int *i_node_position, 
	int *bitmap_position, int *block_position, int *bitmap_inode_positon, int block_size, int num_of_i_nodes);


int main(int argc, char const *argv[])
{
	int block_size;
	int num_of_i_nodes;
	char file_system[64];

	if(argc != 4)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is ./makeFileSystem [x] [y] [z]" << endl;
		return -1;
	}

	block_size = atoi(argv[1]);
	block_size = block_size * _1KB; // Making block size in type of byte.
	num_of_i_nodes = atoi(argv[2]);
	strcpy(file_system, argv[3]);

	if(block_size <= 0 || num_of_i_nodes <= 0)
	{
		cout << "File System Error!" << endl;
		cout << "Block size and number of i-nodes must be positive integer" << endl;
		return -1;
	}

	init_file_system(file_system, block_size, num_of_i_nodes);
	return 0;
}

int init_file_system(char *file_system, int block_size, int num_of_i_nodes)
{
	FILE *file_ptr;
	file_ptr = fopen(file_system, "w");

	if(file_ptr == NULL)
	{
		cout << "Error occurs while opening \"" << file_system << "\" file" << endl;
		return -1;
	}

	int used_size, remaining_size, amount_of_block, i_node_position, bitmap_position, block_position, bitmap_inode_positon;

	calculation(&used_size, &remaining_size, &amount_of_block, &i_node_position, 
		&bitmap_position, &block_position, &bitmap_inode_positon, block_size, num_of_i_nodes);


	int super_blok_items[SuperBlockItems];
	super_blok_items[0] = block_size; // block_size
	super_blok_items[1] = i_node_position; // i_node_position
	super_blok_items[2] = bitmap_position; // bitmap_position
	super_blok_items[3] = block_position; // block_position
	super_blok_items[4] = amount_of_block; // amount_of_block
	super_blok_items[5] = num_of_i_nodes; // amount_of_i_nodes
	super_blok_items[6] = bitmap_inode_positon; // amount_of_i_nodes

	init_SuperBlock(file_ptr, super_blok_items);

	init_iNode(file_ptr, num_of_i_nodes);

	init_BitMapBlock(file_ptr, amount_of_block); 

	init_BitMap_iNode(file_ptr, num_of_i_nodes);

	cout << sizeof(SuperBlock) << endl;
	cout << sizeof(iNode) << endl;
	cout << sizeof(BitMapBlock) << endl;
	cout << "================================" << endl;
	int i;
	for(i = 0; i < remaining_size; ++i)
		fprintf(file_ptr, "%c", '\0');

	fclose(file_ptr);

	file_ptr = fopen(file_system, "r");

	SuperBlock deneme;

	fread(&deneme, sizeof(deneme), 1, file_ptr);

	cout << deneme.block_size << endl;
	cout << deneme.i_node_position << endl;
	cout << deneme.bitmap_position << endl;
	cout << deneme.bitmap_inode_positon << endl;
	cout << deneme.block_position << endl;
	cout << deneme.amount_of_block << endl;
	cout << deneme.amount_of_i_nodes << endl;


	iNode deneme2;

	for(i = 0; i < num_of_i_nodes; ++i)
		fread(&deneme2, sizeof(deneme2), 1, file_ptr);

	cout << deneme2.size_of_file << endl;
	cout << deneme2.type << endl;

	for(i = 0; i < iNodeBlockSize; ++i)
		cout << deneme2.number_of_block[i] << " ";
	cout << endl;
	   // convert now to string form
	char* dt = ctime(&deneme2.last_modification);
	cout << "The local date and time is: " << dt;
	cout << "file_name" << deneme2.file_name << endl;


	BitMapBlock deneme3;
	fread(&deneme3, sizeof(deneme3), 1, file_ptr);

	for(i = 0; i < 30; ++i)
	{
		cout << deneme3.max_bitmap[i] << " "; 
	}
	cout << endl;



	fclose(file_ptr);
	return 1;
}

void calculation(int *used_size, int *remaining_size, int *amount_of_block, int *i_node_position, 
	int *bitmap_position, int *block_position, int *bitmap_inode_positon, int block_size, int num_of_i_nodes)
{

	*used_size = sizeof(SuperBlock) + (num_of_i_nodes * sizeof(iNode)) + sizeof(BitMapBlock) + (num_of_i_nodes *sizeof(int));
	*remaining_size = _1MB - *used_size;
	*amount_of_block = *remaining_size / block_size;
	*i_node_position = sizeof(SuperBlock);
	*bitmap_position = *i_node_position + (num_of_i_nodes * sizeof(iNode));
	*bitmap_inode_positon = *bitmap_position + (num_of_i_nodes * sizeof(int));
	*block_position = *bitmap_inode_positon + sizeof(BitMapBlock);

}
void init_SuperBlock(FILE *file_ptr, int super_blok_items[])
{
	SuperBlock sb = { super_blok_items[0], super_blok_items[1], super_blok_items[2],
		super_blok_items[3], super_blok_items[4], super_blok_items[5], super_blok_items[6]
	};

	fwrite(&sb, sizeof(sb), 1, file_ptr);
}

void init_iNode(FILE *file_ptr, int num_of_i_nodes)
{
	int i;
	int minus_1_arr[iNodeBlockSize] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	char fn[FileNameLenght];

	iNode i_node;
	i_node.size_of_file = 0;
	i_node.type = -1;
	for(i = 0; i < iNodeBlockSize; ++i)
		i_node.number_of_block[i] = minus_1_arr[i];
	i_node.last_modification = time(0);
	strcpy(i_node.file_name, fn);

	for(i = 0; i < num_of_i_nodes; ++i)
	{
		i_node.i_node_id = i;
		fwrite(&i_node, sizeof(i_node), 1, file_ptr);
	}
}

void init_BitMapBlock(FILE *file_ptr, int last_block)
{
	int i;

	BitMapBlock bmp;

	for(i = 0; i < last_block; ++i)
		bmp.max_bitmap[i] = 0;

	for(i = last_block; i < _1KB; ++i)
		bmp.max_bitmap[i] = -1;

	fwrite(&bmp, sizeof(bmp), 1, file_ptr);
}

void init_BitMap_iNode(FILE *file_ptr, int num_of_i_nodes)
{
	int i;
	int bitmap_inode[num_of_i_nodes];

	for(i = 0; i < num_of_i_nodes; ++i)
		bitmap_inode[i] = 0;

	fwrite(bitmap_inode, sizeof(bitmap_inode), 1, file_ptr);
}