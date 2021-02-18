#include "fileSystem.h"

/*
	SuperBlock
	iNode
	BitMapBlock
	BitMap_iNode
	Remaining {Blocks and NotUsable places}
*/

int init_file_system(char *file_system, int block_size, int num_of_i_nodes);
void init_SuperBlock(FILE *file_ptr, int super_blok_items[]);
void init_iNode(FILE *file_ptr, int num_of_i_nodes);
void init_BitMapBlock(FILE *file_ptr, int last_block);
void init_BitMap_iNode(FILE *file_ptr, int num_of_i_nodes);
void init_remaining(FILE *file_ptr, int remaining_size);
void calculation(int *used_size, int *remaining_size, int *amount_of_block, int *i_node_position, 
	int *bitmap_position, int *block_position, int *bitmap_inode_positon, int block_size, int num_of_i_nodes);
int init_root(char *file_system);

void silinecekFonk(char *file_name);
void printSizeOfStructs();
void printSuperBlock(SuperBlock sb);
void print_iNode(iNode i_node);
void print_BitMapBlock(BitMapBlock bmb);

int main(int argc, char const *argv[])
{
	int block_size;
	int num_of_i_nodes;
	char file_system[FileNameLength];

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

	init_remaining(file_ptr, remaining_size);

	fclose(file_ptr);

	init_root(file_system); // Create root directory

	silinecekFonk(file_system); // bunu en son sil

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
	*bitmap_inode_positon = *bitmap_position + sizeof(BitMapBlock);
	*block_position = *bitmap_inode_positon + (num_of_i_nodes * sizeof(int));
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
	char fn[FileNameLength];
	strcpy(fn, "-");
	iNode i_node;
	i_node.size_of_file = 0;
	i_node.type = -1;
	for(i = 0; i < DirectBlocksNum; ++i)
		i_node.direct_block[i] = -1; // not initialize
	i_node.last_modification = time(0);
	strcpy(i_node.file_name, fn);
	strcpy(i_node.lnsym_path, fn); //

	for(i = 0; i < num_of_i_nodes; ++i)
	{
		i_node.i_node_id = i + 1; // [1,2,3.....n]
		fwrite(&i_node, sizeof(i_node), 1, file_ptr);
	}
}

void init_BitMapBlock(FILE *file_ptr, int last_block)
{
	int i;

	BitMapBlock bmb;

	for(i = 0; i < last_block; ++i)
		bmb.max_bitmap_block[i] = 0;

	for(i = last_block; i < _1KB; ++i)
		bmb.max_bitmap_block[i] = -1;

	fwrite(&bmb, sizeof(bmb), 1, file_ptr);
}

void init_BitMap_iNode(FILE *file_ptr, int num_of_i_nodes)
{
	int i;
	int bitmap_inode[num_of_i_nodes];

	for(i = 0; i < num_of_i_nodes; ++i)
		bitmap_inode[i] = 0;

	fwrite(bitmap_inode, sizeof(bitmap_inode), 1, file_ptr);
}

void init_remaining(FILE *file_ptr, int remaining_size)
{
	char empty = '\0';
	for(int j = 0; j < remaining_size; ++j)
		fwrite(&empty, sizeof(char), 1, file_ptr);
}

int init_root(char *file_system)
{
	FILE *file_ptr;
	file_ptr = fopen(file_system, "r+");

	if(file_ptr == NULL)
	{
		cout << "Error occurs while opening \"" << file_system << "\" file" << endl;
		return -1;
	}

	fseek(file_ptr, 0, SEEK_SET);
	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	iNode my_root;
	fread(&my_root, sizeof(my_root), 1, file_ptr);

	my_root.i_node_id = 1;
	my_root.parent_inode_id = 1;
	my_root.type = 0; // directory
	my_root.direct_block[0] = 0; // It means that first block of the system
	my_root.size_of_file = sb.block_size;
	my_root.last_modification = time(0);
	strcpy(my_root.file_name, "/"); // root_name
	fseek(file_ptr, sb.i_node_position, SEEK_SET);
	fwrite(&my_root, sizeof(my_root), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);
	bmb.max_bitmap_block[0] = 1;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	fwrite(&bmb, sizeof(bmb), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);
	int bitmap_inode[sb.amount_of_i_nodes];
	fread(bitmap_inode, sizeof(bitmap_inode), 1, file_ptr);
	bitmap_inode[0] = 1;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);
	fwrite(bitmap_inode, sizeof(bitmap_inode), 1, file_ptr);

	fclose(file_ptr);
	return 1;
}

void silinecekFonk(char *file_system)
{
	FILE *file_ptr;
	file_ptr = fopen(file_system, "r");

	printSizeOfStructs();

	SuperBlock deneme;
	fread(&deneme, sizeof(deneme), 1, file_ptr);
	printSuperBlock(deneme);

	fseek(file_ptr, sizeof(SuperBlock), SEEK_SET);

	iNode deneme2;
	fread(&deneme2, sizeof(deneme2), 1, file_ptr);
	print_iNode(deneme2);

	fseek(file_ptr, deneme.bitmap_position, SEEK_SET);

	BitMapBlock deneme3;
	fread(&deneme3, sizeof(deneme3), 1, file_ptr);
	print_BitMapBlock(deneme3);

	cout << "bitmap_inode : ";
	for(int i = 0; i < deneme.amount_of_i_nodes; ++i)
	{
		int temp;
		fread(&temp, sizeof(temp), 1, file_ptr);
		cout << temp << " ";
	}
	cout << endl;

	fclose(file_ptr);
}

void printSizeOfStructs()
{
	cout << "================================" << endl;
	cout << "SuperBlock : " << sizeof(SuperBlock) << endl;
	cout << "iNode : " << sizeof(iNode) << endl;
	cout << "BitMapBlock : " << sizeof(BitMapBlock) << endl;
	cout << "================================" << endl;
}

void printSuperBlock(SuperBlock sb)
{
	cout << "================================" << endl;
	cout << "block_size : " << sb.block_size << endl;
	cout << "i_node_position : " << sb.i_node_position << endl;
	cout << "bitmap_position : " << sb.bitmap_position << endl;
	cout << "bitmap_inode_positon : " << sb.bitmap_inode_positon << endl;
	cout << "block_position : " << sb.block_position << endl;
	cout << "amount_of_block : " << sb.amount_of_block << endl;
	cout << "amount_of_i_nodes : " << sb.amount_of_i_nodes << endl;

	cout << "Total Usage : " << sb.block_position + (sb.block_size  * sb.amount_of_block) << endl;
	cout << "NOTUSESABLE : " << _1MB - (sb.block_position + (sb.block_size  * sb.amount_of_block)) << endl;
	cout << "================================" << endl;
}

void print_iNode(iNode i_node)
{
	cout << "================================" << endl;
	cout << "i_node_id : " << i_node.i_node_id << endl;
	cout << "parent_inode_id : " << i_node.parent_inode_id << endl;
	cout << "size_of_file : " << i_node.size_of_file << endl;
	cout << "type : " << i_node.type << endl;

	cout << "direct_block : ";
	for(int i = 0; i < DirectBlocksNum; ++i)
		cout << i_node.direct_block[i] << " ";
	cout << endl;

	char* date = ctime(&i_node.last_modification);
	cout << "last_modification : " << date;
	cout << "file_name : " << i_node.file_name << endl;
	cout << "lnsym_path : " << i_node.lnsym_path << endl;
	cout << "================================" << endl;
}

void print_BitMapBlock(BitMapBlock bmb)
{
	cout << "================================" << endl;
	cout << "max_bitmap_block : ";
	for(int i = 0; bmb.max_bitmap_block[i] != -1; ++i)
		cout << bmb.max_bitmap_block[i] << " "; 
	cout << endl;
	cout << "================================" << endl;
}