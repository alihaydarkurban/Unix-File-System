#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "fileSystem.h"

using namespace std;

int is_regular_operation(char *op);
int list(FILE *file_ptr, char *path, char *must_be_null);
int mkdir(FILE *file_ptr, char *path_and_dir, char *must_be_null);
int rmdir(FILE *file_ptr, char *path_and_dir, char *must_be_null);
int dumpe2fs(FILE *file_ptr, char *must_be_null1, char *must_be_null2);
int write(FILE *file_ptr, char *path, char *file);
int read(FILE *file_ptr, char *path, char *file);
int del(FILE *file_ptr, char *path_and_file, char *must_be_null);
int ln(FILE *file_ptr, char *source, char *dest);
int lnsym(FILE *file_ptr, char *source, char *dest);
int fsck(FILE *file_ptr, char *must_be_null1, char *must_be_null2);

vector<char*> parse_string(char *str);
int free_bitmap_index(FILE *file_ptr, SuperBlock sb);
int free_bitmap_inode_index(FILE *file_ptr, SuperBlock sb);
int calculate_free_inode_addr(SuperBlock sb, int i_node_index);
int calculate_free_block_addr(SuperBlock sb, int block_index);
int calculate_max_dir_in_a_block(SuperBlock sb);
vector<int> calculate_blocks_index(FILE *file_ptr, SuperBlock sb, int i_node_id);
int find_block_addr_for_adding_file(FILE *file_ptr, SuperBlock sb, int i_node_id, vector<int> blocks_index);
int free_bitmap_index_different_from_given(FILE *file_ptr, SuperBlock sb, int given_index);
int find_direct_block_index(iNode i_node);

void printSuperBlock(SuperBlock sb);
void print_iNode(iNode i_node);
void print_BitMapBlock(BitMapBlock bmb);
void printSizeOfStructs();


int main(int argc, char *argv[])
{
	int operation_index;
	char file_system[FileNameLength];

	if(argc < 3 || argc > 5)
	{
		printf("File System Error!\n");
		printf("Runnable format is ./fileSystemOper [x] [y] [z optional] [q optional]\n");
		return -1;
	}

	strcpy(file_system, argv[1]);
	FILE *file_ptr;
	file_ptr = fopen(file_system, "r+"); // Opening system file for both reading and writing

	if(file_ptr == NULL)
	{
		cout << "Error occurs while opening \"" << file_system << "\" file" << endl;
		return -1;
	}

	operation_index = is_regular_operation(argv[2]);

	switch(operation_index)
	{
		case 0 : list(file_ptr, argv[3], argv[4]); break;
		case 1 : mkdir(file_ptr, argv[3], argv[4]); break;
		case 2 : rmdir(file_ptr, argv[3], argv[4]); break;
		case 3 : dumpe2fs(file_ptr, argv[3], argv[4]); break;
		case 4 : write(file_ptr, argv[3], argv[4]); break;
		case 5 : read(file_ptr, argv[3], argv[4]); break;
		case 6 : del(file_ptr, argv[3], argv[4]); break;
		case 7 : ln(file_ptr, argv[3], argv[4]); break;
		case 8 : lnsym(file_ptr, argv[3], argv[4]); break;
		case 9 : fsck(file_ptr, argv[3], argv[4]); break;	
		default:
		{
			cout << "File System Error!" << endl;
			cout << "\"" << argv[2] << "\" is not a runnable operation" << endl;
		}

	}
	fclose(file_ptr);


	file_ptr = fopen(file_system, "r");

	printSizeOfStructs();

	SuperBlock deneme;
	fread(&deneme, sizeof(deneme), 1, file_ptr);
	printSuperBlock(deneme);

	fseek(file_ptr, sizeof(SuperBlock) + sizeof(iNode), SEEK_SET);

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


	Directory deneme5;
	fseek(file_ptr, deneme.block_position + sizeof(Directory) , SEEK_SET);
	fread(&deneme5, sizeof(deneme5), 1, file_ptr);

	cout << "i : " << deneme5.i_node_number << endl;
	cout << "file : " << deneme5.file_name << endl;


	fclose(file_ptr);


	return 0;
}

int is_regular_operation(char * op)
{
	// -1 means that op is not a regular operation
	for(int i = 0; i < OP_SIZE; ++i)
	{
		if(strcmp(op, operations[i]) == 0)
			return i;
	}
	return -1;
}

int list(FILE *file_ptr, char *path, char *must_be_null)
{
	cout << "list" << endl;
	
	if(path == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : list [path]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Path : " << path << endl;


	return 1;
}

int mkdir(FILE *file_ptr, char *path_and_dir, char *must_be_null)
{ 
	// cout << "mkdir" << endl;
	if(path_and_dir == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : mkdir [path and directory name]" << endl;
		return -1;
	}
	// cout << "Path and directory name : " << path_and_dir << endl;

	vector<char*> tokens = parse_string(path_and_dir);
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct naming" << endl;
		return -1;
	}
	else
	{
		SuperBlock sb;
		fseek(file_ptr, 0, SEEK_SET);
		fread(&sb, sizeof(sb), 1, file_ptr);

		int free_inode_index = free_bitmap_inode_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
		int free_block_index = free_bitmap_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1

		if(free_inode_index == -1 || free_block_index == -1)
		{
			cout << "File System Error!" << endl;
			cout << "There is not enough i-node" << endl;
			return -1;
		}

		if(tokens_size == 1)
		{
			// child_inode_id free_idone_index + 1, i_node_id goes 1,2,3,...n 
			// parent_inode_id root = 1 (parent_inode_id)
			int new_i_node_addr = calculate_free_inode_addr(sb, free_inode_index);	// This is new free inode index for new dir or file
			int new_block_addr = calculate_free_block_addr(sb, free_block_index);	// This is new free block index for new dir or file
			vector<int> root_blocks_index = calculate_blocks_index(file_ptr, sb, 1);	// Finds the root's used block index

			if(root_blocks_index.size() == DirectBlocksNum)
			{
				cout << "Direct blocks are full!" << endl;
				cout << "There is no single, double and triple blocks yet" << endl;
				return -1;
			}

			// Icinde bosluk olan blocun baslangic addresini donderir.
			int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, 1, root_blocks_index);
			cout << "-->" << usable_block_addr << endl;

			if(usable_block_addr == -1) // root direct_block needs new block 
			{
				cout << "LAN YENI BLOCK GEREK " << endl;
				int new_index_for_root_block = free_bitmap_index_different_from_given(file_ptr, sb, free_block_index); // This is for current inode extra block 

				cout << "VERILEN BLOCK ID : " << new_index_for_root_block << endl;
				if(new_index_for_root_block == -1)
				{
					cout << "File System Error!" << endl;
					cout << "There is not enough block" << endl;
					return -1;
				}
				// burada root un -1 olan yerini bul ve ona new_index_for_root_block ver 
				// ve onun için addr hesapla usable_block_addr ata.
				// ardından için current_dirc count bul ve o adrese ekleme yapıcaksın en aşağıda

				fseek(file_ptr, sb.i_node_position, SEEK_SET); // root
				iNode root_inode;
				fread(&root_inode, sizeof(root_inode), 1, file_ptr);
				int direct_block_index = find_direct_block_index(root_inode); // ilk -1 gordugu index i return eder
				root_inode.direct_block[direct_block_index] = new_index_for_root_block; // yeni olusan block indexi atar
				usable_block_addr = calculate_free_block_addr(sb, new_index_for_root_block); // yeni block index icin adres olusturur

				cout << "YENI BLOCK ADRES " << usable_block_addr << endl;
				fseek(file_ptr, sb.i_node_position, SEEK_SET); 
				fwrite(&root_inode, sizeof(root_inode), 1, file_ptr);

				fseek(file_ptr, sb.bitmap_position, SEEK_SET); 
				BitMapBlock bmb;
				fread(&bmb, sizeof(bmb), 1, file_ptr);
				bmb.max_bitmap_block[new_index_for_root_block] = 1;
				fseek(file_ptr, sb.bitmap_position, SEEK_SET);
				fwrite(&bmb, sizeof(bmb), 1, file_ptr);

			}
//// Sadece ekleme yapacağı block için şuanlık
			fseek(file_ptr, usable_block_addr, SEEK_SET); // directory ekleneme yapılacak block
			int max_dir_in_block = calculate_max_dir_in_a_block(sb);
			Directory dir_arr[max_dir_in_block];
			int dir_count = 0;

			for(int i = 0; i < max_dir_in_block; ++i)
			{
				fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);
				if(dir_arr[i].file_name[0] == '\0')
					break;
				dir_count++;
			}
			for(int i = 0; i < dir_count; ++i)
			{
				if(strcmp(dir_arr[i].file_name, tokens[0]) == 0)
				{
					cout << "File System Error!" << endl;
					cout << "Same file name could not be possible in the same directory!" << endl;
					return -1;
				}
			}
/////
			// ====================================================
			// new created directory
			fseek(file_ptr, new_i_node_addr, SEEK_SET);
			iNode new_i_node;
			fread(&new_i_node, sizeof(new_i_node), 1, file_ptr);

			new_i_node.i_node_id = free_inode_index + 1;
			new_i_node.parent_inode_id = 1; // parent is root
			new_i_node.type = 0;
			strcpy(new_i_node.file_name, tokens[0]);
			new_i_node.last_modification = time(0);
			new_i_node.direct_block[0] = free_block_index;
			fseek(file_ptr, new_i_node_addr, SEEK_SET);
			fwrite(&new_i_node, sizeof(new_i_node), 1, file_ptr);

			fseek(file_ptr, sb.bitmap_position, SEEK_SET);
			BitMapBlock bmb;
			fread(&bmb, sizeof(bmb), 1, file_ptr);
			bmb.max_bitmap_block[free_block_index] = 1;
			fseek(file_ptr, sb.bitmap_position, SEEK_SET);
			fwrite(&bmb, sizeof(bmb), 1, file_ptr);

			fseek(file_ptr, sb.bitmap_inode_positon + (sizeof(int) * free_inode_index), SEEK_SET);
			int not_empty = 1;
			fwrite(&not_empty, sizeof(int), 1, file_ptr);
			// ====================================================

			// ====================================================
			// add directory structor inside the root
			Directory dir;

			dir.i_node_number = new_i_node.i_node_id; 
			strcpy(dir.file_name, new_i_node.file_name);
			fseek(file_ptr, usable_block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
			fwrite(&dir, sizeof(Directory), 1, file_ptr);
			// ====================================================
		}

		else
		{
			fseek(file_ptr, sb.i_node_position, SEEK_SET);





		}
	}
	return 1;
}

int rmdir(FILE *file_ptr, char *path_and_dir, char *must_be_null)
{ 
	cout << "rmdir" << endl;
	
	if(path_and_dir == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : rmdir [path and directory name]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Path and directory name : " << path_and_dir << endl;


	return 1;
}

int dumpe2fs(FILE *file_ptr, char *must_be_null1, char *must_be_null2)
{ 
	cout << "dumpe2fs" << endl;

	if(must_be_null1 != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : dumpe2fs" << endl;
		return -1;
	}
	cout << "It is OK to run" << endl;


	return 1;
}

int write(FILE *file_ptr, char *path, char *file)
{ 
	cout << "write" << endl;
	
	if(path == NULL || file == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : write [path] [file]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Path name : " << path << endl;
	cout << "File name : " << file << endl;


	return 1;
}

int read(FILE *file_ptr, char *path, char *file)
{ 
	cout << "read" << endl;
	
	if(path == NULL || file == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : read [path] [file]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "path name : " << path << endl;
	cout << "file name : " << file << endl;


	return 1;
}

int del(FILE *file_ptr, char *path_and_file, char *must_be_null)
{ 
	cout << "del" << endl;
	
	if(path_and_file == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : del [path and file name]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Path and file name : " << path_and_file << endl;


	return 1;
}

int ln(FILE *file_ptr, char *source, char *dest)
{
	cout << "ln" << endl;
	
	if(source == NULL || dest == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : ln [source path and file] [destitation path and file]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Source path and file name : " << source << endl;
	cout << "Destitation path and file: " << dest << endl;


	return 1;
}

int lnsym(FILE *file_ptr, char *source, char *dest)
{ 
	cout << "lnsym" << endl;
	
	if(source == NULL || dest == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : lnsym [source path and file] [destitation path and file]" << endl;
		return -1;
	}

	cout << "It is OK to run" << endl;
	cout << "Source path and file name : " << source << endl;
	cout << "Destitation path and file: " << dest << endl;


	return 1;
}

int fsck(FILE *file_ptr, char *must_be_null1, char *must_be_null2)
{ 
	cout << "fsck" << endl;

	if(must_be_null1 != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : fsck" << endl;
		return -1;
	}
	cout << "It is OK to run" << endl;


	return 1;
}

vector<char*> parse_string(char *str)
{
	vector<char*> tokens;
	int seperator_count = 0;

	if(str[0] != split[0])
		return tokens;

	strcat(str, "-");

	char *each_word = strtok(str, split);

	while(each_word != NULL) 
	{ 
		tokens.push_back(each_word);
		each_word = strtok(NULL, split);
	} 
	
	if(strcmp(tokens[tokens.size() - 1], "-") == 0)
	{
		vector<char*> error_vec;
		return error_vec;
	}
	else
	{
		char *temp;
		temp = tokens[tokens.size() - 1];
		temp[strlen(temp) - 1] = '\0';
		tokens[tokens.size() - 1] = temp;
	}
	return tokens;
}

int free_bitmap_index(FILE *file_ptr, SuperBlock sb)
{
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);

	for(int i = 0; sb.amount_of_block; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0)
			return i;
	}

	return -1;
}

int free_bitmap_inode_index(FILE *file_ptr, SuperBlock sb)
{
	int free_inode;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&free_inode, sizeof(free_inode), 1, file_ptr);
		if(free_inode == 0)
			return i;
	}
	return -1;
}

int free_bitmap_index_different_from_given(FILE *file_ptr, SuperBlock sb, int given_index)
{
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);

	for(int i = 0; sb.amount_of_block; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0 && i != given_index)
			return i;
	}

	return -1;
}

int calculate_free_inode_addr(SuperBlock sb, int i_node_index)
{
	return (sb.i_node_position + (i_node_index * sizeof(iNode)));
}

int calculate_free_block_addr(SuperBlock sb, int block_index)
{
	return (sb.block_position + (block_index * sb.block_size));
}

int calculate_max_dir_in_a_block(SuperBlock sb)
{
	return (sb.block_size / sizeof(Directory));
}

vector<int> calculate_blocks_index(FILE *file_ptr, SuperBlock sb, int i_node_id)
{
	int i_node_arr = calculate_free_inode_addr(sb, i_node_id - 1); // seraching inode addr; i_node id 1, 2, 3 .. n
	fseek(file_ptr, i_node_arr, SEEK_SET);

	vector<int> blocks_index;
	iNode i_node;
	fread(&i_node, sizeof(iNode), 1, file_ptr);

	for(int i = 0; (i_node.direct_block[i] != -1) && (i < DirectBlocksNum); ++i)
		blocks_index.push_back(i_node.direct_block[i]);
	
	return blocks_index;
}

int find_block_addr_for_adding_file(FILE *file_ptr, SuperBlock sb, int i_node_id, vector<int> blocks_index)
{
	int max_dir_count = calculate_max_dir_in_a_block(sb); // It means that a block holds at most this number of dir and file.

	for(int i = 0; i < blocks_index.size(); ++i)
	{
		int block_addr = calculate_free_block_addr(sb, blocks_index[i]); // This is block addr of given block index

		fseek(file_ptr, block_addr, SEEK_SET);

		Directory dir_arr[max_dir_count];
		int dir_count = 0;

		for(int j = 0; j < max_dir_count; ++j)
		{
			fread(&dir_arr[j], sizeof(Directory), 1, file_ptr);
			if(dir_arr[j].file_name[0] == '\0')
				break;
			dir_count++;
		}

		if(dir_count < max_dir_count)
		{
			int return_addr = block_addr;
			return return_addr;
		}
	}
	return -1;
}

int find_direct_block_index(iNode i_node)
{
	for(int i = 0; i < DirectBlocksNum; ++i)
	{
		if(i_node.direct_block[i] == -1)
			return i;
	}
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

void printSizeOfStructs()
{
	cout << "================================" << endl;
	cout << "SuperBlock : " << sizeof(SuperBlock) << endl;
	cout << "iNode : " << sizeof(iNode) << endl;
	cout << "BitMapBlock : " << sizeof(BitMapBlock) << endl;
	cout << "================================" << endl;
}