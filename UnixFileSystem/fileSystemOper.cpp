#include "fileSystem.h"

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
int free_bitmap_block_index(FILE *file_ptr, SuperBlock sb);
int free_bitmap_inode_index(FILE *file_ptr, SuperBlock sb);
int calculate_inode_addr(SuperBlock sb, int i_node_index);
int calculate_block_addr(SuperBlock sb, int block_index);
bool same_name_check(FILE *file_ptr, int i_node_id, char *name, int type_of_it);
int find_inode_id(FILE *file_ptr, int i_node_id, char *name, int type_of_it);
int calculate_dir_or_file_in_a_block(FILE *file_ptr, SuperBlock sb, int block_addr);
int calculate_max_dir_or_file_in_a_block(SuperBlock sb);
vector<int> calculate_blocks_index(FILE *file_ptr, SuperBlock sb, int i_node_id);
int find_block_addr_for_adding_file(FILE *file_ptr, SuperBlock sb, int i_node_id, vector<int> blocks_index);
int free_bitmap_index_different_from_given(FILE *file_ptr, SuperBlock sb, int given_index);
int find_direct_block_index(iNode i_node);
void list_print_given_id(FILE *file_ptr, SuperBlock sb, int i_node_id);


// print fonks....
void printSuperBlock(SuperBlock sb);
void print_iNode(FILE *file_ptr, SuperBlock sb);
void print_BitMapBlock(BitMapBlock bmb);
void printSizeOfStructs();
void silinecekFonk(char *file_system);
// print fonks....

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

	silinecekFonk(file_system);

	return 0;
}
void silinecekFonk(char *file_system)
{
	FILE *file_ptr;
	file_ptr = fopen(file_system, "r");

	printSizeOfStructs();

	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	printSuperBlock(sb);
	print_iNode(file_ptr, sb);

	BitMapBlock bmb;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	fread(&bmb, sizeof(bmb), 1, file_ptr);
	print_BitMapBlock(bmb);

	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);
	cout << "bitmap_inode : ";
	int bm_inode;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&bm_inode, sizeof(bm_inode), 1, file_ptr);
		cout << bm_inode << " ";
	}
	cout << endl;

	Directory dir;
	fseek(file_ptr, sb.block_position, SEEK_SET);
	fread(&dir, sizeof(dir), 1, file_ptr);

	cout << "inode : " << dir.i_node_number << endl;
	cout << "file  : " << dir.file_name << endl;

	fclose(file_ptr);
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
	if(path == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : list [path]" << endl;
		return -1;
	}
	char real_path[FileNameLength];
	strcpy(real_path, path);

	vector<char*> tokens = parse_string(path);
	int tokens_size = tokens.size();

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	if(real_path[0] == split[0] && strlen(real_path) == 1)
		list_print_given_id(file_ptr, sb, 1); // root
	
	else if(tokens_size > 0) 
	{
		int parent_id = 1; 
		for(int i = 0; i < tokens_size; ++i)
		{

			bool control = same_name_check(file_ptr, parent_id, tokens[i], 0);
			if(control == false)
			{
				cout << "File System Error!" << endl;
				cout << "There is no path like : " << real_path << endl;
				return -1;
			}
			int current_parent_id;
			current_parent_id = find_inode_id(file_ptr, parent_id, tokens[i], 0);
			parent_id = current_parent_id;
		}
		list_print_given_id(file_ptr, sb, parent_id); // root
	}

	else
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path naming" << endl;
		return -1;
	}

	return 1;
}

int mkdir(FILE *file_ptr, char *path_and_dir, char *must_be_null)
{ 
	if(path_and_dir == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : mkdir [path and directory name]" << endl;
		return -1;
	}

	char real_path_and_dir[FileNameLength];
	strcpy(real_path_and_dir, path_and_dir);


	vector<char*> tokens = parse_string(path_and_dir); // Parsing the path
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and directory naming" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	int free_inode_index = free_bitmap_inode_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
	int free_block_index = free_bitmap_block_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1

	if(free_inode_index == -1 || free_block_index == -1)
	{
		cout << "File System Error!" << endl;
		cout << "There is not enough i-node" << endl;
		return -1;
	}

	int parent_id = 1; // starts with root.

	// If tokes size = 1 then this loop will not work.
	// And this means that you will add the root directory.
	for(int i = 0; i < tokens_size - 1; ++i)
	{
		bool control = same_name_check(file_ptr, parent_id, tokens[i], 0);
		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_path_and_dir << endl;
			return -1;
		}
		int current_parent_id;
		current_parent_id = find_inode_id(file_ptr, parent_id, tokens[i], 0);
		parent_id = current_parent_id;
	}

	int parent_inode_position = calculate_inode_addr(sb, parent_id - 1); //it is parent inode addr

	// child_inode_id free_idone_index + 1, i_node_id goes 1,2,3,...n 
	int new_i_node_addr = calculate_inode_addr(sb, free_inode_index);	// This is new free inode index for new dir or file
	int new_block_addr = calculate_block_addr(sb, free_block_index);	// This is new free block index for new dir or file
	vector<int> parent_blocks_index = calculate_blocks_index(file_ptr, sb, parent_id);	// Finds the parent's used blocks index

	cout << "ROOT_BLOCKSIZE : "<< parent_blocks_index.size() << endl;

	if(parent_blocks_index.size() == DirectBlocksNum) // If the direct blocks are full
	{
		cout << "Direct blocks are full!" << endl;
		cout << "There is no single, double and triple blocks yet" << endl;
		return -1;
	}

	// find_block_addr_for_adding_file find the first useable block addr.
	// It returns -1 if the parent inode needs new block.
	int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, parent_id, parent_blocks_index);
	cout << "-->" << usable_block_addr << endl;

	if(usable_block_addr == -1) // parent inode's direct_block needs new block 
	{
		cout << "LAN YENI BLOCK GEREK " << endl;
		// This finds a free index and this free index must be different from free_block_index
		int new_block_index_for_parent = free_bitmap_index_different_from_given(file_ptr, sb, free_block_index); 

		cout << "VERILEN BLOCK ID : " << new_block_index_for_parent << endl;
		if(new_block_index_for_parent == -1)
		{
			cout << "File System Error!" << endl;
			cout << "There is not enough block" << endl;
			return -1;
		}

		fseek(file_ptr, parent_inode_position, SEEK_SET); // Make the file_ptr in parent_inode_position
		iNode root_inode;
		fread(&root_inode, sizeof(root_inode), 1, file_ptr);
		int direct_block_index = find_direct_block_index(root_inode); // It returns the first not used direct_block index
		root_inode.direct_block[direct_block_index] = new_block_index_for_parent;
		usable_block_addr = calculate_block_addr(sb, new_block_index_for_parent); // Calculate the new block addr of new_block_index_for_parent

		cout << "YENI BLOCK ADRES " << usable_block_addr << endl;
		fseek(file_ptr, parent_inode_position, SEEK_SET); 
		fwrite(&root_inode, sizeof(root_inode), 1, file_ptr);

		// Fill the bitmap_block
		fseek(file_ptr, sb.bitmap_position, SEEK_SET); 
		BitMapBlock bmb;
		fread(&bmb, sizeof(bmb), 1, file_ptr);
		bmb.max_bitmap_block[new_block_index_for_parent] = 1;
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fwrite(&bmb, sizeof(bmb), 1, file_ptr);

	}

	bool is_same_name = same_name_check(file_ptr, parent_id, tokens[tokens_size - 1], 0); // 0 means that directory

	if(is_same_name == true)
	{
		cout << "File System Error!" << endl;
		cout << "Same file name could not be possible in the same directory!" << endl;
		return -1;
	}

	int dir_count = calculate_dir_or_file_in_a_block(file_ptr, sb, usable_block_addr);

	// ====================================================
	// new created directory
	fseek(file_ptr, new_i_node_addr, SEEK_SET);
	iNode new_i_node;
	fread(&new_i_node, sizeof(new_i_node), 1, file_ptr);

	new_i_node.i_node_id = free_inode_index + 1;
	new_i_node.parent_inode_id = parent_id; 
	new_i_node.type = 0;
	strcpy(new_i_node.file_name, tokens[tokens_size - 1]);
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
	// add directory structor inside the parent (parent' in last modifiye tarihini degistirmen gerekebilir)
	Directory dir;
	dir.i_node_number = new_i_node.i_node_id; 
	strcpy(dir.file_name, new_i_node.file_name);
	fseek(file_ptr, usable_block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
	fwrite(&dir, sizeof(Directory), 1, file_ptr);
	// ====================================================

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

int free_bitmap_block_index(FILE *file_ptr, SuperBlock sb)
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

int calculate_inode_addr(SuperBlock sb, int i_node_index)
{
	return (sb.i_node_position + (i_node_index * sizeof(iNode)));
}

int calculate_block_addr(SuperBlock sb, int block_index)
{
	return (sb.block_position + (block_index * sb.block_size));
}

bool same_name_check(FILE *file_ptr, int i_node_id, char *name, int type_of_it) 
{
	fseek(file_ptr, 0, SEEK_SET);

	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	int used_inode_count = 0;
	vector<int> index_of_used_inodes;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);

	int bm_inode;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&bm_inode, sizeof(bm_inode), 1, file_ptr);
		if(bm_inode != 0)
		{
			index_of_used_inodes.push_back(i);
			used_inode_count++;
		}
	}
	int used_inode_addr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
		used_inode_addr[i] = calculate_inode_addr(sb, index_of_used_inodes[i]);

	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, used_inode_addr[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id && 
			inode_arr[i].type == type_of_it &&
			strcmp(inode_arr[i].file_name, name) == 0)
			return true;
	}
	return false;
}

int find_inode_id(FILE *file_ptr, int i_node_id, char *name, int type_of_it)
{
	fseek(file_ptr, 0, SEEK_SET);

	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	int used_inode_count = 0;
	vector<int> index_of_used_inodes;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);

	int bm_inode;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&bm_inode, sizeof(bm_inode), 1, file_ptr);
		if(bm_inode != 0)
		{
			index_of_used_inodes.push_back(i);
			used_inode_count++;
		}
	}
	int used_inode_addr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
		used_inode_addr[i] = calculate_inode_addr(sb, index_of_used_inodes[i]);

	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, used_inode_addr[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id && 
			inode_arr[i].type == type_of_it &&
			strcmp(inode_arr[i].file_name, name) == 0)
			return  inode_arr[i].i_node_id;
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

int calculate_dir_or_file_in_a_block(FILE *file_ptr, SuperBlock sb, int block_addr)
{
	fseek(file_ptr, block_addr, SEEK_SET); // Makes the file_ptr the begining of addable block

	int max_dir_in_block = calculate_max_dir_or_file_in_a_block(sb); // This returns the max number of having directory or file in a block 
	
	Directory dir_arr[max_dir_in_block];
	int dir_count = 0;

	for(int i = 0; i < max_dir_in_block; ++i)
	{
		fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);
		if(dir_arr[i].file_name[0] == '\0')
			break;
		dir_count++;
	}
	return dir_count;
}

int calculate_max_dir_or_file_in_a_block(SuperBlock sb)
{
	return (sb.block_size / sizeof(Directory));
}

vector<int> calculate_blocks_index(FILE *file_ptr, SuperBlock sb, int i_node_id)
{
	int i_node_adrr = calculate_inode_addr(sb, i_node_id - 1); // seraching inode addr; i_node id 1, 2, 3 .. n
	fseek(file_ptr, i_node_adrr, SEEK_SET);

	vector<int> blocks_index;
	iNode i_node;
	fread(&i_node, sizeof(iNode), 1, file_ptr);

	for(int i = 0; (i_node.direct_block[i] != -1) && (i < DirectBlocksNum); ++i)	
		blocks_index.push_back(i_node.direct_block[i]);

	return blocks_index;
}

int find_block_addr_for_adding_file(FILE *file_ptr, SuperBlock sb, int i_node_id, vector<int> blocks_index)
{
	int max_dir_count = calculate_max_dir_or_file_in_a_block(sb); // It means that a block holds at most this number of dir and file.

	for(int i = 0; i < blocks_index.size(); ++i)
	{
		int block_addr = calculate_block_addr(sb, blocks_index[i]); // This is block addr of given block index

		int dir_count = calculate_dir_or_file_in_a_block(file_ptr, sb, block_addr);

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

void list_print_given_id(FILE *file_ptr, SuperBlock sb, int i_node_id)
{
	int used_inode_count = 0;
	vector<int> index_of_used_inodes;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);

	int bm_inode;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&bm_inode, sizeof(bm_inode), 1, file_ptr);
		if(bm_inode != 0)
		{
			index_of_used_inodes.push_back(i);
			used_inode_count++;
		}
	}
	int used_inode_addr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
		used_inode_addr[i] = calculate_inode_addr(sb, index_of_used_inodes[i]);

	iNode inode_arr[used_inode_count];

	cout << "Total : ALI" << endl;
	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, used_inode_addr[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id && inode_arr[i].i_node_id != i_node_id)
		{
			if(inode_arr[i].type == 0) // directory
				cout << "Directory   cse312@ubuntu ";
			else
				cout < "Regular_File cse312@ubuntu ";

			printf("%7d ", inode_arr[i].size_of_file);
			char* date = ctime(&inode_arr[i].last_modification);
			date[strlen(date) - 6] = '\0';
			cout << date << " ";
			cout << inode_arr[i].file_name << endl;
		}
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

void print_iNode(FILE *file_ptr, SuperBlock sb)
{

	int used_inode_count = 0;
	vector<int> index_of_used_inodes;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);

	int bm_inode;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&bm_inode, sizeof(bm_inode), 1, file_ptr);
		if(bm_inode != 0)
		{
			index_of_used_inodes.push_back(i);
			used_inode_count++;
		}
	}
	int used_inode_addr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
		used_inode_addr[i] = calculate_inode_addr(sb, index_of_used_inodes[i]);

	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, used_inode_addr[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		cout << "================================" << endl;
		cout << "i_node_id : " << inode_arr[i].i_node_id << endl;
		cout << "parent_inode_id : " << inode_arr[i].parent_inode_id << endl;
		cout << "size_of_file : " << inode_arr[i].size_of_file << endl;
		cout << "type : " << inode_arr[i].type << endl;

		cout << "direct_block : ";
		for(int j = 0; j < DirectBlocksNum; ++j)
			cout << inode_arr[i].direct_block[j] << " ";
		cout << endl;

		char* date = ctime(&inode_arr[i].last_modification);
		cout << "last_modification : " << date;
		cout << "file_name : " << inode_arr[i].file_name << endl;
		cout << "================================" << endl;	
	}
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


