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
bool same_name_check(FILE *file_ptr, int i_node_id, char *name, int type_of_it, int inode_addresses[], int used_inode_count);
bool find_parent_inode_id(FILE *file_ptr, int i_node_id, char *name, int type_of_it, int *inode_id_ptr, int inode_addresses[], int used_inode_count);
int calculate_dir_or_file_in_a_block(FILE *file_ptr, SuperBlock sb, int block_addr);
int calculate_max_dir_or_file_in_a_block(SuperBlock sb);
vector<int> calculate_blocks_index(FILE *file_ptr, SuperBlock sb, int i_node_id);
int find_block_addr_for_adding_file(FILE *file_ptr, SuperBlock sb, int i_node_id, vector<int> blocks_index);
int free_bitmap_index_different_from_given(FILE *file_ptr, SuperBlock sb, int given_index);
int find_direct_block_index(iNode i_node);
void list_print_given_id(FILE *file_ptr, SuperBlock sb, int i_node_id, int inode_addresses[], int used_inode_count);
int find_used_inode_addr(FILE *file_ptr, SuperBlock sb, int used_inode_addr[]);
void set_last_modification(FILE *file_ptr, SuperBlock sb, int i_node_id, time_t time);
bool is_it_a_parent_inode(FILE *file_ptr, SuperBlock sb, int i_node_id, int inode_addresses[], int used_inode_count); 
int calculate_free_inode_count(FILE *file_ptr, SuperBlock sb);
int calculate_free_block_count(FILE *file_ptr, SuperBlock sb);
void calculate_director_and_file_count(FILE *file_ptr, SuperBlock sb, int* dir_countP, int *file_countP, int inode_addresses[], int used_inode_count); 
int first_place_to_add_directory_structor(FILE *file_ptr, SuperBlock sb, int block_addr);
void clean_the_removed_directory_space(FILE *file_ptr, SuperBlock sb, vector<int> other_parent_id_blocks_index, Directory removed_dir, int id);
int find_place_of_directory(FILE *file_ptr, SuperBlock sb, int block_addr, Directory dir, bool *first_time);
void fill_array_with_zeros(int arr[], int size);
void set_size_for_directory(FILE *file_ptr, SuperBlock sb, int i_node_id, int used_block_count);
void free_bitmap_blocks_indexes_different_from_given(FILE *file_ptr, SuperBlock sb, int block_index[], int size);
void clean_block(FILE *file_ptr, SuperBlock sb, vector<int> block_index);
bool is_all_free(FILE *file_ptr, SuperBlock sb, int block_addr);
void set_all_linked_size_and_time(FILE *file_ptr, SuperBlock sb, int inode_addresses[], int used_inode_count, char *lnsym_path, int file_size, bool time_control);

int main(int argc, char *argv[])
{
	int operation_index;
	char file_system[MAX_PATH_SIZE];

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
	if(path == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : list [path]" << endl;
		return -1;
	}
	char real_path[MAX_PATH_SIZE];
	strcpy(real_path, path);

	vector<char*> tokens = parse_string(path);
	int tokens_size = tokens.size();

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	if(real_path[0] == split[0] && strlen(real_path) == 1)
		list_print_given_id(file_ptr, sb, 1, inode_addresses, used_inode_count); // root
	
	else if(tokens_size > 0) 
	{
		int parent_id = 1; 
		for(int i = 0; i < tokens_size; ++i)
		{
			int current_parent_id;
			bool control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count);
			if(control == false)
			{
				cout << "File System Error!" << endl;
				cout << "There is no path like : " << real_path << endl;
				return -1;
			}
			parent_id = current_parent_id;
		}
		list_print_given_id(file_ptr, sb, parent_id, inode_addresses, used_inode_count); // other parent
	}

	else
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path naming : " <<  real_path << endl;
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

	char real_path_and_dir[MAX_PATH_SIZE];
	strcpy(real_path_and_dir, path_and_dir);

	vector<char*> tokens = parse_string(path_and_dir); // Parsing the path
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and directory naming : " << real_path_and_dir << endl;
		return -1;
	}

	char ch[FileNameLength];
	strcpy(ch, tokens[tokens_size - 1]); 
	if(ch[0] == '-')
	{
		cout << "File System Error!" << endl;
		cout << "It can not be a file name : \"" << ch << "\"" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	int parent_id = 1; // starts with root.

	// If tokes size = 1 then this loop will not work.
	// And this means that you will add the root directory.
	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	for(int i = 0; i < tokens_size - 1; ++i)
	{
		int current_parent_id;
		bool control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count);
		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_path_and_dir << endl;
			return -1;
		}
		parent_id = current_parent_id;
	}

	int free_inode_index = free_bitmap_inode_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
	int free_block_index = free_bitmap_block_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1

	if(free_inode_index == -1 || free_block_index == -1)
	{
		cout << "File System Error!" << endl;
		cout << "There is not enough i-node" << endl;
		return -1;
	}

	int parent_inode_position = calculate_inode_addr(sb, parent_id - 1); //it is parent inode addr

	// child_inode_id free_idone_index + 1, i_node_id goes 1,2,3,...n 
	int new_i_node_addr = calculate_inode_addr(sb, free_inode_index);	// This is new free inode index for new dir or file
	int new_block_addr = calculate_block_addr(sb, free_block_index);	// This is new free block index for new dir or file
	vector<int> parent_blocks_index = calculate_blocks_index(file_ptr, sb, parent_id);	// Finds the parent's used blocks index

	int used_block_count = parent_blocks_index.size();
	int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, parent_id, parent_blocks_index);

	if(parent_blocks_index.size() == DirectBlocksNum && usable_block_addr == -1) // If the direct blocks are full
	{
		cout << "Direct blocks are full!" << endl;
		cout << "There is no single, double and triple blocks yet" << endl;
		return -1;
	}

	bool is_same_name = same_name_check(file_ptr, parent_id, tokens[tokens_size - 1], 0, inode_addresses, used_inode_count); // 0 means that directory

	if(is_same_name == true)
	{
		cout << "File System Error!" << endl;
		cout << "Same directory name could not be possible in the same directory!" << endl;
		return -1;
	}

	if(strlen(tokens[tokens_size - 1]) >= FileNameLength)
	{
		cout << "File System Error!" << endl;
		cout << "A directory name can have at most " << FileNameLength << " characters"<< endl;
		return -1;
	}
	// find_block_addr_for_adding_file find the first useable block addr.
	// It returns -1 if the parent inode needs new block.
	// int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, parent_id, parent_blocks_index);
	if(usable_block_addr == -1) // parent inode's direct_block needs new block 
	{
		// This finds a free index and this free index must be different from free_block_index
		int new_block_index_for_parent = free_bitmap_index_different_from_given(file_ptr, sb, free_block_index); 

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

	int dir_count = first_place_to_add_directory_structor(file_ptr, sb, usable_block_addr);

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
	new_i_node.size_of_file = sb.block_size;
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
	// add directory structor inside the parent
	Directory dir;
	dir.i_node_number = new_i_node.i_node_id; 
	strcpy(dir.file_name, new_i_node.file_name);
	fseek(file_ptr, usable_block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
	fwrite(&dir, sizeof(Directory), 1, file_ptr);
	set_last_modification(file_ptr, sb, parent_id, time(0));
	set_size_for_directory(file_ptr, sb, parent_id, used_block_count);
	// ====================================================

	return 1;
}

int rmdir(FILE *file_ptr, char *path_and_dir, char *must_be_null)
{ 
	if(path_and_dir == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : rmdir [path and directory name]" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	char real_path_and_dir[MAX_PATH_SIZE];
	strcpy(real_path_and_dir, path_and_dir);

	vector<char*> tokens = parse_string(path_and_dir); // Parsing the path
	int tokens_size = tokens.size();

	if(real_path_and_dir[0] == split[0] && strlen(real_path_and_dir) == 1) // root can not be deleted!!
	{
		cout << "File System Error!" << endl;
		cout << "Root directory can not be deleted" << endl;
		return -1;
	}

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and directory naming : " << real_path_and_dir << endl;
		return -1;
	}

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	int other_parent_id = 1; 
	int parent_id = 1; 
	for(int i = 0; i < tokens_size; ++i)
	{
		int current_parent_id;
		bool control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count);
		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_path_and_dir << endl;
			return -1;
		}
		parent_id = current_parent_id;
		if(i == (tokens_size - 2))
		{
			other_parent_id = parent_id;
		}
	}
	bool is_parent = is_it_a_parent_inode(file_ptr, sb, parent_id, inode_addresses, used_inode_count);

	if(is_parent == true)
	{
		cout << "File System Error!" << endl;
		cout << "The \""<< tokens[tokens_size - 1] << "\" is not empty" << endl;
		return -1;
	}

	int parent_inode_position = calculate_inode_addr(sb, parent_id - 1);

	fseek(file_ptr, parent_inode_position, SEEK_SET);
	iNode inode;
	fread(&inode, sizeof(iNode), 1, file_ptr);
	int deleted_index_in_direct_block = inode.direct_block[0];
	for(int i = 0; i < DirectBlocksNum; ++i)
		inode.direct_block[i] = -1;
	strcpy(inode.file_name, "-"); // Removed inode file name.

	fseek(file_ptr, parent_inode_position, SEEK_SET);
	fwrite(&inode, sizeof(iNode), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(BitMapBlock), 1, file_ptr);
	bmb.max_bitmap_block[deleted_index_in_direct_block] = 0;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	fwrite(&bmb, sizeof(BitMapBlock), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_inode_positon + (sizeof(int) * (parent_id - 1)), SEEK_SET);
	int empty = 0;
	fwrite(&empty, sizeof(int), 1, file_ptr);

	// ========================================
	// other_parent_id operations
	vector<int> other_parent_id_blocks_index = calculate_blocks_index(file_ptr, sb, other_parent_id);
	Directory removed_dir;
	removed_dir.i_node_number = parent_id; // /ali/haydar --> haydar's id
	strcpy(removed_dir.file_name, tokens[tokens_size - 1]); // /ali/haydar --> haydar

	clean_the_removed_directory_space(file_ptr, sb, other_parent_id_blocks_index, removed_dir, other_parent_id);
	set_last_modification(file_ptr, sb, other_parent_id, time(0));
	// ========================================
	return 1;
}

int dumpe2fs(FILE *file_ptr, char *must_be_null1, char *must_be_null2)
{ 
	if(must_be_null1 != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : dumpe2fs" << endl;
		return -1;
	}
	
	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(SuperBlock), 1, file_ptr);

	int used_inode_addr[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, used_inode_addr);

	int total_block_count = sb.amount_of_block;
	int total_inode_count = sb.amount_of_i_nodes;
	int block_size = sb.block_size;
	int free_inode_count = calculate_free_inode_count(file_ptr, sb);
	int free_block_count = calculate_free_block_count(file_ptr, sb);	
	int dir_count = 0, file_count = 0;
	calculate_director_and_file_count(file_ptr, sb, &dir_count, &file_count, used_inode_addr, used_inode_count);

	cout << "Block Count : " << total_block_count << endl;
	cout << "I-node Count : " << total_inode_count << endl;
	cout << "Free Block Count : " << free_block_count << endl;
	cout << "Free I-node Count : " << free_inode_count << endl;
	cout << "Directory Count : " << dir_count << endl;
	cout << "File Count : " << file_count << endl;
	cout << "Block Size : " << block_size << " bytes" << endl;

	iNode inode_arr[used_inode_count];

	cout << "Information About Used I-nodes" << endl;
	cout << "Parent I-node Id | I-node Id | File Name | Block Index -> Block Address" << endl;

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, used_inode_addr[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);
		cout << inode_arr[i].parent_inode_id << " | ";
		cout << inode_arr[i].i_node_id << " | ";
		cout << "\""<< inode_arr[i].file_name << "\" | ";		
		int count = 0;
		int index_of_used_block[DirectBlocksNum];

		for(int j = 0; j < DirectBlocksNum; ++j)
		{
			if(inode_arr[i].direct_block[j] != -1)
			{
				index_of_used_block[count] = inode_arr[i].direct_block[j]; 
				count++;
			}
		}
		for(int j = 0; j < count; ++j)
			cout << index_of_used_block[j] << " -> " << calculate_block_addr(sb, index_of_used_block[j]) << ". ";
		
		cout << endl;
	}
	return 1;
}

int write(FILE *file_ptr, char *path, char *file)
{ 
	if(path == NULL || file == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : write [path] [file]" << endl;
		return -1;
	}

	char real_path_and_file[MAX_PATH_SIZE];
	char linux_file[MAX_PATH_SIZE];
	strcpy(real_path_and_file, path);
	strcpy(linux_file, file);

	vector<char*> tokens = parse_string(path); // Parsing the path
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and file naming : " << real_path_and_file << endl;
		return -1;
	}

	char ch[FileNameLength];
	strcpy(ch, tokens[tokens_size - 1]); 
	if(ch[0] == '-')
	{
		cout << "File System Error!" << endl;
		cout << "It can not be a file name : \"" << ch << "\"" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	int parent_id = 1; // starts with root.

	// If tokes size = 1 then this loop will not work.
	// And this means that you will add the root directory.
	for(int i = 0; i < tokens_size - 1; ++i)
	{
		int current_parent_id;
		bool control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count);
		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_path_and_file << endl;
			return -1;
		}
		parent_id = current_parent_id;
	}

	int free_inode_index = free_bitmap_inode_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
	int free_block_index = free_bitmap_block_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1

	if(free_inode_index == -1 || free_block_index == -1)
	{
		cout << "File System Error!" << endl;
		cout << "There is not enough i-node" << endl;
		return -1;
	}
	int current_parent_id;
	bool is_same_name = find_parent_inode_id(file_ptr, parent_id, tokens[tokens_size - 1], 1, &current_parent_id, inode_addresses, used_inode_count); // 1 means that file
	
	if(strlen(tokens[tokens_size - 1]) >= FileNameLength)
	{
		cout << "File System Error!" << endl;
		cout << "A file name can have at most " << FileNameLength << " characters"<< endl;
		return -1;
	}

	if(is_same_name == true)
	{
		parent_id = current_parent_id;
		int check_the_file_addr = calculate_inode_addr(sb, parent_id - 1);
		fseek(file_ptr, check_the_file_addr, SEEK_SET);
		iNode check_the_file;
		fread(&check_the_file, sizeof(iNode), 1, file_ptr);

		bool likend_size_control = false;
		FILE *new_file_ptr;
		new_file_ptr = fopen(linux_file, "r");
		if(new_file_ptr == NULL)
		{
			cout << "Error occurs while opening \"" << linux_file << "\" file" << endl;
			return -1;
		}

		if(strcmp(check_the_file.lnsym_path, "-") != 0) // lnsym file
		{
			char new_lnsym_path[MAX_PATH_SIZE];
			strcpy(new_lnsym_path, check_the_file.lnsym_path);

			vector<char*> lnsym_tokens = parse_string(new_lnsym_path); // Parsing the path
			int lnsym_tokens_size = lnsym_tokens.size();
			int lnsym_parent_id = 1; 

			for(int i = 0; i < lnsym_tokens_size; ++i)
			{
				int current_parent_id;
				bool control;

				if(i == lnsym_tokens_size - 1)
					control = find_parent_inode_id(file_ptr, lnsym_parent_id, lnsym_tokens[i], 1, &current_parent_id, inode_addresses, used_inode_count); // file check
				else
					control = find_parent_inode_id(file_ptr, lnsym_parent_id, lnsym_tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count); // directory check

				if(control == false)
				{
					cout << "File System Error!" << endl;
					cout << "Symbolick linked source file was deleted"<< endl;
					return -1;
				}

				lnsym_parent_id = current_parent_id;
			}
			parent_id = lnsym_parent_id;	
			likend_size_control = true;
		}

		fseek(new_file_ptr, 0, SEEK_END);
		int reading_size = ftell(new_file_ptr); // INCLUDING NULL (END OF FILE)
		int needed_block_count = (reading_size / sb.block_size) + 1;
		if(needed_block_count > DirectBlocksNum)
		{
			cout << "There is no single, double and triple blocks yet" << endl;
			cout << "At most number of " << DirectBlocksNum << " block will be used" << endl;
			needed_block_count = DirectBlocksNum;
			reading_size = (needed_block_count * sb.block_size);
		}

		vector<int> block_index = calculate_blocks_index(file_ptr, sb, parent_id);
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		BitMapBlock bmb;
		fread(&bmb, sizeof(BitMapBlock), 1, file_ptr);
		for(int i = 0; i < block_index.size(); ++i)
			bmb.max_bitmap_block[block_index[i]] = 0;
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fwrite(&bmb, sizeof(BitMapBlock), 1, file_ptr);
		clean_block(file_ptr, sb, block_index);

		int new_block_index[needed_block_count];
		for(int i = 0; i < needed_block_count; ++i)
			new_block_index[i] = -1;

		// returns the usable block indexes (new_block_index)
		free_bitmap_blocks_indexes_different_from_given(file_ptr, sb, new_block_index, needed_block_count);

		if(new_block_index[0] == -1)
		{
			cout << "File System Error!" << endl;
			cout << "There is not enough block" << endl;
			return -1;
		}

		int index = 0;
		fseek(new_file_ptr, 0, SEEK_SET);

		int block_addr = calculate_block_addr(sb, new_block_index[0]);
		fseek(file_ptr, block_addr, SEEK_SET);

		for(int i = 0; i < reading_size; ++i)
		{
			char ch_in_reading_file = fgetc(new_file_ptr);

			if(i % sb.block_size == 0 && i != 0)
			{
				index++;
				block_addr = calculate_block_addr(sb, new_block_index[index]);
				fseek(file_ptr, block_addr, SEEK_SET);
			}
			fwrite(&ch_in_reading_file, sizeof(char), 1, file_ptr);
		}
		char null_ch = '\0';
		fwrite(&null_ch, sizeof(char), 1, file_ptr);
		fclose(new_file_ptr);

		//inode
		iNode new_i_node;
		int addr = calculate_inode_addr(sb, parent_id - 1);
		fseek(file_ptr, addr, SEEK_SET);
		fread(&new_i_node, sizeof(new_i_node), 1, file_ptr);
		new_i_node.last_modification = time(0);
		for(int i = 0; i < DirectBlocksNum; ++i)
			new_i_node.direct_block[i] = -1;

		for(int i = 0; i < needed_block_count; ++i)
			new_i_node.direct_block[i] = new_block_index[i];	

		new_i_node.size_of_file = reading_size;
		fseek(file_ptr, addr, SEEK_SET);
		fwrite(&new_i_node, sizeof(new_i_node), 1, file_ptr);

		// bitmap block
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fread(&bmb, sizeof(bmb), 1, file_ptr);
		for(int i = 0; i < needed_block_count; ++i)
			bmb.max_bitmap_block[new_block_index[i]] = 1;
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fwrite(&bmb, sizeof(bmb), 1, file_ptr);
		
		if(likend_size_control)
			set_all_linked_size_and_time(file_ptr, sb, inode_addresses, used_inode_count, check_the_file.lnsym_path, reading_size, true); // for linked
		else
			set_all_linked_size_and_time(file_ptr, sb, inode_addresses, used_inode_count, real_path_and_file, reading_size, false); // not linked (but it could be parent of link)

	}

	else if(is_same_name == false)
	{
		int parent_inode_position = calculate_inode_addr(sb, parent_id - 1); //it is parent inode addr

		// child_inode_id free_idone_index + 1, i_node_id goes 1,2,3,...n 
		int new_i_node_addr = calculate_inode_addr(sb, free_inode_index);	// This is new free inode index for new dir or file
		vector<int> parent_blocks_index = calculate_blocks_index(file_ptr, sb, parent_id);	// Finds the parent's used blocks index

		int used_block_count = parent_blocks_index.size();
		int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, parent_id, parent_blocks_index);

		if(parent_blocks_index.size() == DirectBlocksNum && usable_block_addr == -1) // If the direct blocks are full
		{
			cout << "Direct blocks are full!" << endl;
			cout << "There is no single, double and triple blocks yet" << endl;
			return -1;
		}

		if(usable_block_addr == -1) // parent inode's direct_block needs new block 
		{
			// This finds a free index and this free index must be different from free_block_index
			int new_block_index_for_parent = free_bitmap_index_different_from_given(file_ptr, sb, free_block_index); 

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

		int dir_count = first_place_to_add_directory_structor(file_ptr, sb, usable_block_addr);

		FILE *new_file_ptr;
		new_file_ptr = fopen(linux_file, "r");
		if(new_file_ptr == NULL)
		{
			cout << "Error occurs while opening \"" << linux_file << "\" file" << endl;
			return -1;
		}

		// ====================================================
		// add directory structor inside the parent
		char my_file[FileNameLength];
		strcpy(my_file, tokens[tokens_size - 1]);
		Directory dir;
		dir.i_node_number = free_inode_index + 1; //(free_inode_index + 1) means that the new file inode id
		strcpy(dir.file_name, my_file);

		fseek(file_ptr, usable_block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
		fwrite(&dir, sizeof(Directory), 1, file_ptr);
		set_last_modification(file_ptr, sb, parent_id, time(0));
		// ====================================================

		// ====================================================
		// copy other file and write it.
		fseek(new_file_ptr, 0, SEEK_END);
		int reading_size = ftell(new_file_ptr); // INCLUDING NULL (END OF FILE)
		int needed_block_count = (reading_size / sb.block_size) + 1;

		if(needed_block_count > DirectBlocksNum)
		{
			cout << "There is no single, double and triple blocks yet" << endl;
			cout << "At most number of " << DirectBlocksNum << " block will be used" << endl;
			needed_block_count = DirectBlocksNum;
			reading_size = (needed_block_count * sb.block_size);
		}

		int new_block_index[needed_block_count];
		for(int i = 0; i < needed_block_count; ++i)
			new_block_index[i] = -1;

		// returns the usable block indexes (new_block_index)
		free_bitmap_blocks_indexes_different_from_given(file_ptr, sb, new_block_index, needed_block_count);
		if(new_block_index[0] == -1)
		{
			cout << "File System Error!" << endl;
			cout << "There is not enough block" << endl;
			return -1;
		}

		int index = 0;
		fseek(new_file_ptr, 0, SEEK_SET);

		int block_addr = calculate_block_addr(sb, new_block_index[0]);
		fseek(file_ptr, block_addr, SEEK_SET);

		for(int i = 0; i < reading_size; ++i)
		{
			char ch_in_reading_file = fgetc(new_file_ptr);

			if(i % sb.block_size == 0 && i != 0)
			{
				index++;
				block_addr = calculate_block_addr(sb, new_block_index[index]);
				fseek(file_ptr, block_addr, SEEK_SET);
			}
			fwrite(&ch_in_reading_file, sizeof(char), 1, file_ptr);
		}
		char null_ch = '\0';
		fwrite(&null_ch, sizeof(char), 1, file_ptr);
		fclose(new_file_ptr);

		//inode
		fseek(file_ptr, new_i_node_addr, SEEK_SET);
		iNode new_i_node;
		fread(&new_i_node, sizeof(new_i_node), 1, file_ptr);

		new_i_node.i_node_id = free_inode_index + 1;
		new_i_node.parent_inode_id = parent_id; 
		new_i_node.type = 1; // file
		strcpy(new_i_node.file_name, my_file);
		new_i_node.last_modification = time(0);

		for(int i = 0; i < needed_block_count; ++i)
			new_i_node.direct_block[i] = new_block_index[i];
		
		new_i_node.size_of_file = reading_size;
		fseek(file_ptr, new_i_node_addr, SEEK_SET);
		fwrite(&new_i_node, sizeof(new_i_node), 1, file_ptr);

		// bitmap block
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		BitMapBlock bmb;
		fread(&bmb, sizeof(bmb), 1, file_ptr);
		for(int i = 0; i < needed_block_count; ++i)
			bmb.max_bitmap_block[new_block_index[i]] = 1;
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fwrite(&bmb, sizeof(bmb), 1, file_ptr);

		// free bitmap
		fseek(file_ptr, sb.bitmap_inode_positon + (sizeof(int) * free_inode_index), SEEK_SET);
		int not_empty = 1;
		fwrite(&not_empty, sizeof(int), 1, file_ptr);
		// ====================================================
	}

	return 1;
}

int read(FILE *file_ptr, char *path, char *file)
{ 
	if(path == NULL || file == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : read [path] [file]" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	char real_path_and_file[MAX_PATH_SIZE];
	strcpy(real_path_and_file, path);
	char linux_file[MAX_PATH_SIZE];
	strcpy(linux_file, file);


	vector<char*> tokens = parse_string(path); // Parsing the path
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and file naming : " << real_path_and_file << endl;
		return -1;
	}

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	int parent_id = 1; 
	for(int i = 0; i < tokens_size; ++i)
	{
		int current_parent_id;
		bool control;

		if(i == tokens_size - 1)
			control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 1, &current_parent_id, inode_addresses, used_inode_count); // file check
		else
			control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count); // directory check

		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no file path like : " << real_path_and_file << endl;
			return -1;
		}
		parent_id = current_parent_id;
	}

	int check_the_file_addr = calculate_inode_addr(sb, parent_id - 1);
	fseek(file_ptr, check_the_file_addr, SEEK_SET);
	iNode check_the_file;
	fread(&check_the_file, sizeof(iNode), 1, file_ptr);

	if(strcmp(check_the_file.lnsym_path, "-") != 0) // lnsym file
	{
		char new_lnsym_path[MAX_PATH_SIZE];
		strcpy(new_lnsym_path, check_the_file.lnsym_path);

		vector<char*> lnsym_tokens = parse_string(new_lnsym_path); // Parsing the path
		int lnsym_tokens_size = lnsym_tokens.size();
		int lnsym_parent_id = 1; 

		for(int i = 0; i < lnsym_tokens_size; ++i)
		{
			int current_parent_id;
			bool control;

			if(i == lnsym_tokens_size - 1)
				control = find_parent_inode_id(file_ptr, lnsym_parent_id, lnsym_tokens[i], 1, &current_parent_id, inode_addresses, used_inode_count); // file check
			else
				control = find_parent_inode_id(file_ptr, lnsym_parent_id, lnsym_tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count); // directory check

			if(control == false)
			{
				cout << "File System Error!" << endl;
				cout << "Symbolick linked source file was deleted"<< endl;
				return -1;
			}

			lnsym_parent_id = current_parent_id;
		}

		parent_id = lnsym_parent_id;
	}

	vector<int> parent_blocks_index = calculate_blocks_index(file_ptr, sb, parent_id);	// Finds the parent's used blocks index
	int block_index_size = parent_blocks_index.size(); 

	FILE *new_file_ptr;
	new_file_ptr = fopen(linux_file, "w");
	if(new_file_ptr == NULL)
	{
		cout << "Error occurs while opening \"" << linux_file << "\" file" << endl;
		return -1;
	}

	int block_addr = calculate_block_addr(sb, parent_blocks_index[0]);
	fseek(file_ptr, block_addr, SEEK_SET);
	int count = 0;
	for(int i = 0; i < (block_index_size * sb.block_size); ++i)
	{
		if(i % sb.block_size == 0 && i != 0)
		{
			count++;
			block_addr = calculate_block_addr(sb, parent_blocks_index[count]);
			fseek(file_ptr, block_addr, SEEK_SET);
		}
		char ch;
		fread(&ch, sizeof(char), 1, file_ptr);

		if(ch != '\0')
			fwrite(&ch, sizeof(char), 1, new_file_ptr);
		else
			break;
	}
	fclose(new_file_ptr);

	return 1;
}

int del(FILE *file_ptr, char *path_and_file, char *must_be_null)
{ 
	if(path_and_file == NULL || must_be_null != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : del [path and file name]" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	char real_path_and_file[MAX_PATH_SIZE];
	strcpy(real_path_and_file, path_and_file);

	vector<char*> tokens = parse_string(path_and_file); // Parsing the path
	int tokens_size = tokens.size();

	if(tokens_size == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and file naming : " << real_path_and_file << endl;
		return -1;
	}

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	int other_parent_id = 1; 
	int parent_id = 1; 
	for(int i = 0; i < tokens_size; ++i)
	{
		bool control;
		int current_parent_id;
		if(i == tokens_size - 1)
			control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 1, &current_parent_id, inode_addresses, used_inode_count); // file check
		else
			control = find_parent_inode_id(file_ptr, parent_id, tokens[i], 0, &current_parent_id, inode_addresses, used_inode_count); // directory check

		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_path_and_file << endl;
			return -1;
		}
		parent_id = current_parent_id;
		if(i == (tokens_size - 2))
		{
			other_parent_id = parent_id;
		}
	}
	vector<int> block_index = calculate_blocks_index(file_ptr, sb, parent_id);

	int parent_inode_position = calculate_inode_addr(sb, parent_id - 1);
	
	fseek(file_ptr, parent_inode_position, SEEK_SET);
	iNode inode;
	fread(&inode, sizeof(iNode), 1, file_ptr);
	for(int i = 0; i < DirectBlocksNum; ++i)
		inode.direct_block[i] = -1;
	strcpy(inode.file_name, "-"); // Removed inode file name.
	strcpy(inode.lnsym_path, "-");
	
	fseek(file_ptr, parent_inode_position, SEEK_SET);
	fwrite(&inode, sizeof(iNode), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(BitMapBlock), 1, file_ptr);
	for(int i = 0; i < block_index.size(); ++i)
		bmb.max_bitmap_block[block_index[i]] = 0;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	fwrite(&bmb, sizeof(BitMapBlock), 1, file_ptr);

	fseek(file_ptr, sb.bitmap_inode_positon + (sizeof(int) * (parent_id - 1)), SEEK_SET);
	int empty = 0;
	fwrite(&empty, sizeof(int), 1, file_ptr);
	clean_block(file_ptr, sb, block_index);

	// ========================================
	// other_parent_id operations (real parent)
	vector<int> other_parent_id_blocks_index = calculate_blocks_index(file_ptr, sb, other_parent_id);
	Directory removed_dir;
	removed_dir.i_node_number = parent_id;
	strcpy(removed_dir.file_name, tokens[tokens_size - 1]);
	clean_the_removed_directory_space(file_ptr, sb, other_parent_id_blocks_index, removed_dir, other_parent_id);
	set_last_modification(file_ptr, sb, other_parent_id, time(0));
	// ========================================
	return 1;
}

int ln(FILE *file_ptr, char *source, char *dest)
{
	if(source == NULL || dest == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : ln [source path and file] [destitation path and file]" << endl;
		return -1;
	}
	cout << "Source path and file name : " << source << endl;
	cout << "Destitation path and file: " << dest << endl;
	cout << "Hard-Linking is not available!" << endl;

	return 1;
}

int lnsym(FILE *file_ptr, char *source, char *dest)
{ 
	if(source == NULL || dest == NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : lnsym [source path and file] [destitation path and file]" << endl;
		return -1;
	}

	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	char real_source[MAX_PATH_SIZE];
	strcpy(real_source, source);
	char real_dest[MAX_PATH_SIZE];
	strcpy(real_dest, dest);
	char real_dest_temp[MAX_PATH_SIZE];
	strcpy(real_dest_temp, dest);

	vector<char*> tokens_source = parse_string(source); // Parsing the path
	int tokens_size_source = tokens_source.size();

	if(tokens_size_source == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and file naming : " << real_source << endl;
		return -1;
	}

	int inode_addresses[sb.amount_of_i_nodes];
	int used_inode_count = find_used_inode_addr(file_ptr, sb, inode_addresses);

	int source_id = 1; 
	for(int i = 0; i < tokens_size_source; ++i)
	{
		int current_parent_id;
		bool control;

		if(i == tokens_size_source - 1)
			control = find_parent_inode_id(file_ptr, source_id, tokens_source[i], 1, &current_parent_id, inode_addresses, used_inode_count); // file check
		else
			control = find_parent_inode_id(file_ptr, source_id, tokens_source[i], 0, &current_parent_id, inode_addresses, used_inode_count); // directory check

		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no file path like : " << real_source << endl;
			return -1;
		}
		source_id = current_parent_id;
	}

	bool flag_for_adding_block = true;

	int addr = calculate_inode_addr(sb, source_id - 1);
	fseek(file_ptr, addr, SEEK_SET);
	iNode control_inode;
	fread(&control_inode, sizeof(iNode), 1, file_ptr);
	if(strcmp(control_inode.lnsym_path, "-") != 0)
	{
		flag_for_adding_block = false;
		strcpy(real_source, control_inode.lnsym_path);
	}
	/// ================================================
	vector<char*> tokens_dest = parse_string(real_dest); // Parsing the path

	int tokens_size_dest = tokens_dest.size();
	if(tokens_size_dest == 0)
	{
		cout << "File System Error!" << endl;
		cout << "Not correct path and file naming : " << real_dest_temp << endl;
		return -1;
	}

	char ch[FileNameLength];
	strcpy(ch, tokens_dest[tokens_size_dest - 1]); 
	if(ch[0] == '-')
	{
		cout << "File System Error!" << endl;
		cout << "It can not be a file name : \"" << ch << "\"" << endl;
		return -1;
	}

	int free_inode_index = free_bitmap_inode_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
	if(free_inode_index == -1)
	{
		cout << "File System Error!" << endl;
		cout << "There is not enough i-node" << endl;
		return -1;
	}
	int dest_parent_id = 1; // starts with root.

	// If tokes size = 1 then this loop will not work.
	// And this means that you will add the root directory.
	for(int i = 0; i < tokens_size_dest - 1; ++i)
	{
		int current_parent_id;
		bool control = find_parent_inode_id(file_ptr, dest_parent_id, tokens_dest[i], 0, &current_parent_id, inode_addresses, used_inode_count);
		if(control == false)
		{
			cout << "File System Error!" << endl;
			cout << "There is no path like : " << real_dest_temp << endl;
			return -1;
		}
		dest_parent_id = current_parent_id;
	}

	int parent_inode_position_dest = calculate_inode_addr(sb, dest_parent_id - 1); //it is parent inode addr

	// child_inode_id free_idone_index + 1, i_node_id goes 1,2,3,...n 
	int new_i_node_addr = calculate_inode_addr(sb, free_inode_index);	// This is new free inode index for new dir or file
	vector<int> parent_blocks_index = calculate_blocks_index(file_ptr, sb, dest_parent_id);	// Finds the parent's used blocks index

	int used_block_count = parent_blocks_index.size();
	int usable_block_addr = find_block_addr_for_adding_file(file_ptr, sb, dest_parent_id, parent_blocks_index);

	if(parent_blocks_index.size() == DirectBlocksNum && usable_block_addr == -1) // If the direct blocks are full
	{
		cout << "Direct blocks are full!" << endl;
		cout << "There is no single, double and triple blocks yet" << endl;
		return -1;
	}

	bool is_same_name = same_name_check(file_ptr, dest_parent_id, tokens_dest[tokens_size_dest - 1], 1, inode_addresses, used_inode_count); // 1 means that file

	if(is_same_name == true)
	{
		cout << "File System Error!" << endl;
		cout << "--Same file name could not be possible in the same directory!" << endl;
		return -1;
	}

	if(strlen(tokens_dest[tokens_size_dest - 1]) >= FileNameLength)
	{
		cout << "File System Error!" << endl;
		cout << "A file name can have at most " << FileNameLength << " characters"<< endl;
		return -1;
	}

	if(usable_block_addr == -1) // parent inode's direct_block needs new block 
	{
		// This finds a free index and this free index must be different from free_block_index
		int free_block_index = free_bitmap_block_index(file_ptr, sb); // -1 error control // 0,1,2.. n-1
		if(free_block_index == -1)
		{
			cout << "File System Error!" << endl;
			cout << "There is not enough block" << endl;
			return -1;
		}

		fseek(file_ptr, parent_inode_position_dest, SEEK_SET); // Make the file_ptr in parent_inode_position_dest
		iNode root_inode;
		fread(&root_inode, sizeof(root_inode), 1, file_ptr);
		int direct_block_index = find_direct_block_index(root_inode); // It returns the first not used direct_block index
		root_inode.direct_block[direct_block_index] = free_block_index;
		usable_block_addr = calculate_block_addr(sb, free_block_index); // Calculate the new block addr of new_block_index_for_parent

		fseek(file_ptr, parent_inode_position_dest, SEEK_SET); 
		fwrite(&root_inode, sizeof(root_inode), 1, file_ptr);

		// Fill the bitmap_block
		fseek(file_ptr, sb.bitmap_position, SEEK_SET); 
		BitMapBlock bmb;
		fread(&bmb, sizeof(bmb), 1, file_ptr);
		bmb.max_bitmap_block[free_block_index] = 1;
		fseek(file_ptr, sb.bitmap_position, SEEK_SET);
		fwrite(&bmb, sizeof(bmb), 1, file_ptr);

	}

	int dir_count = first_place_to_add_directory_structor(file_ptr, sb, usable_block_addr);
	// ====================================================
	// add directory structor inside the parent
	char my_file[FileNameLength];
	strcpy(my_file, tokens_dest[tokens_size_dest - 1]);
	Directory dir;
	dir.i_node_number = free_inode_index + 1; //(free_inode_index + 1) means that the new file inode id
	strcpy(dir.file_name, my_file);

	fseek(file_ptr, usable_block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
	fwrite(&dir, sizeof(Directory), 1, file_ptr);
	set_last_modification(file_ptr, sb, dest_parent_id, time(0));
	// ====================================================

	// ====================================================
	// copy other file and write it.
	int source_inode_addr = calculate_inode_addr(sb, source_id - 1);
	iNode source_inode;
	fseek(file_ptr, source_inode_addr, SEEK_SET);
	fread(&source_inode, sizeof(iNode), 1, file_ptr);

	//inode
	fseek(file_ptr, new_i_node_addr, SEEK_SET);
	iNode new_i_node;
	fread(&new_i_node, sizeof(new_i_node), 1, file_ptr);

	new_i_node.i_node_id = free_inode_index + 1;
	new_i_node.parent_inode_id = dest_parent_id; 
	new_i_node.size_of_file = source_inode.size_of_file;
	new_i_node.type = 1; // file
	new_i_node.last_modification = time(0);
	strcpy(new_i_node.file_name, my_file);
	strcpy(new_i_node.lnsym_path, real_source);
	fseek(file_ptr, new_i_node_addr, SEEK_SET);
	fwrite(&new_i_node, sizeof(new_i_node), 1, file_ptr);

	// free bitmap
	fseek(file_ptr, sb.bitmap_inode_positon + (sizeof(int) * free_inode_index), SEEK_SET);
	int not_empty = 1;
	fwrite(&not_empty, sizeof(int), 1, file_ptr);
	// ====================================================

	return 1;
}

int fsck(FILE *file_ptr, char *must_be_null1, char *must_be_null2)
{ 
	if(must_be_null1 != NULL)
	{
		cout << "File System Error!" << endl;
		cout << "Runnable format is : fsck" << endl;
		return -1;
	}
	
	SuperBlock sb;
	fseek(file_ptr, 0, SEEK_SET);
	fread(&sb, sizeof(sb), 1, file_ptr);

	int free_block_arr[sb.amount_of_block];
	int used_block_arr[sb.amount_of_block];
	int free_inode_arr[sb.amount_of_i_nodes];
	int used_inode_arr[sb.amount_of_i_nodes];

	fill_array_with_zeros(free_block_arr, sb.amount_of_block);
	fill_array_with_zeros(used_block_arr, sb.amount_of_block);
	fill_array_with_zeros(free_inode_arr, sb.amount_of_i_nodes);
	fill_array_with_zeros(used_inode_arr, sb.amount_of_i_nodes);

	iNode inode_arr[sb.amount_of_i_nodes];
	fseek(file_ptr, sb.i_node_position, SEEK_SET);

	// Scanning all inodes from start of the begining of i-node position in the file
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if((inode_arr[i].file_name[0] != '-')) // means that not empty i node
			used_inode_arr[inode_arr[i].i_node_id - 1] = used_inode_arr[inode_arr[i].i_node_id - 1] + 1; 
		
	}

	// Scanning bitmap_inode for free places
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);
	int is_free;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&is_free, sizeof(is_free), 1, file_ptr);
		if(is_free == 0) // means that empty
			free_inode_arr[i] = free_inode_arr[i] + 1; 
	}

	// Scanning used inodes block index for used block
	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		for(int j = 0; j < DirectBlocksNum; ++j)
		{
			if((inode_arr[i].direct_block[j] != -1)) // means that not empty block
				used_block_arr[inode_arr[i].direct_block[j]] = used_block_arr[inode_arr[i].direct_block[j]] + 1;
		}
	}

	// Scanning bitmap for free block
	BitMapBlock bmb;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	fread(&bmb, sizeof(bmb), 1, file_ptr);

	for(int i = 0; bmb.max_bitmap_block[i] != -1; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0)
			free_block_arr[i] = free_block_arr[i] + 1;
	}
	
	cout << "Used Block Table" << endl;
	for(int i = 0; i < sb.amount_of_block; ++i) // for(int i = 0; i < sb.amount_of_block; ++i) for(int i = 0; i < 40; ++i)
		cout << used_block_arr[i] << " ";

	cout << "\n\nFree Block Table" << endl;
	for(int i = 0; i < sb.amount_of_block; ++i)  // for(int i = 0; i < sb.amount_of_block; ++i) for(int i = 0; i < 40; ++i)
		cout << free_block_arr[i] << " ";

	cout << "\n\nUsed I-node Table" << endl;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i) // for(int i = 0; i < sb.amount_of_i_nodes; ++i) for(int i = 0; i < 40; ++i)
		cout << used_inode_arr[i] << " ";

	cout << "\n\nFree I-node Table" << endl;
	for(int i = 0; i < sb.amount_of_i_nodes; ++i) // for(int i = 0; i < sb.amount_of_i_nodes; ++i) for(int i = 0; i < 40; ++i)
		cout << free_inode_arr[i] << " ";

	cout << endl;

	return 1;
}

vector<char*> parse_string(char *str)
{
	vector<char*> tokens;

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

	for(int i = 0; i < sb.amount_of_block; ++i)
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

bool same_name_check(FILE *file_ptr, int i_node_id, char *name, int type_of_it, int inode_addresses[], int used_inode_count) 
{
	fseek(file_ptr, 0, SEEK_SET);

	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id && 
			inode_arr[i].type == type_of_it &&
			strcmp(inode_arr[i].file_name, name) == 0)
			return true;
	}
	return false;
}

bool find_parent_inode_id(FILE *file_ptr, int i_node_id, char *name, int type_of_it, int *inode_id_ptr, int inode_addresses[], int used_inode_count)
{
	fseek(file_ptr, 0, SEEK_SET);
	SuperBlock sb;
	fread(&sb, sizeof(sb), 1, file_ptr);

	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id && 
			inode_arr[i].type == type_of_it &&
			strcmp(inode_arr[i].file_name, name) == 0)
		{
			*inode_id_ptr = inode_arr[i].i_node_id; // This is return value with pointer
			return true;
		}
	}
	return false;
}

int free_bitmap_index_different_from_given(FILE *file_ptr, SuperBlock sb, int given_index)
{
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);

	for(int i = 0; i < sb.amount_of_block; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0 && i != given_index)
			return i;
	}
	return -1;
}

int calculate_dir_or_file_in_a_block(FILE *file_ptr, SuperBlock sb, int block_addr) // Total directory count of given block addr
{
	fseek(file_ptr, block_addr, SEEK_SET); // Makes the file_ptr the begining of addable block

	int max_dir_in_block = calculate_max_dir_or_file_in_a_block(sb); // This returns the max number of having directory or file in a block 
	
	Directory dir_arr[max_dir_in_block];
	int dir_count = 0;

	for(int i = 0; i < max_dir_in_block; ++i)
	{
		fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);
		if(dir_arr[i].file_name[0] != '\0' && dir_arr[i].file_name[0] != '-') //!!
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

	for(int i = 0; (i < DirectBlocksNum); ++i)
	{
		if(i_node.direct_block[i] != -1)
		{
			blocks_index.push_back(i_node.direct_block[i]);			
		}
	}	
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

int first_place_to_add_directory_structor(FILE *file_ptr, SuperBlock sb, int block_addr) // First empty place for directory
{
	fseek(file_ptr, block_addr, SEEK_SET); // Makes the file_ptr the begining of addable block

	int max_dir_in_block = calculate_max_dir_or_file_in_a_block(sb); // This returns the max number of having directory or file in a block 
	
	Directory dir_arr[max_dir_in_block];
	int dir_count = 0;

	for(int i = 0; i < max_dir_in_block; ++i)
	{
		fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);
		if(dir_arr[i].file_name[0] == '\0' || dir_arr[i].file_name[0] == '-') //!!
			break;
		dir_count++;
	}
	return dir_count;
}

int find_direct_block_index(iNode i_node)
{
	for(int i = 0; i < DirectBlocksNum; ++i)
	{
		if(i_node.direct_block[i] == -1)
			return i;
	}
}

void list_print_given_id(FILE *file_ptr, SuperBlock sb, int i_node_id, int inode_addresses[], int used_inode_count)
{
	int total = 0;
	bool flag;
	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		flag = false;
		if(inode_arr[i].parent_inode_id == i_node_id && inode_arr[i].i_node_id != i_node_id)
		{
			if(inode_arr[i].type == 0) // directory
				cout << "Directory    cse312@ubuntu ";
			else if(inode_arr[i].type == 1 && strcmp(inode_arr[i].lnsym_path, "-") == 0) // file
				cout << "RegularFile  cse312@ubuntu ";
			else if(strcmp(inode_arr[i].lnsym_path, "-") != 0)
			{
				flag = true;
				cout << "LinkedFile   cse312@ubuntu ";
			}

			total = total + inode_arr[i].size_of_file; 
			printf("%7d ", inode_arr[i].size_of_file);
			char* date = ctime(&inode_arr[i].last_modification);
			date[strlen(date) - 6] = '\0';
			cout << date << " ";
			if(!flag)
				cout << inode_arr[i].file_name << endl;
			else
			{
				cout << inode_arr[i].file_name << " -> ";
				cout << inode_arr[i].lnsym_path << endl;
			}
		}
	}
	cout << "Total : " << total  << " bytes" << endl;
}

int find_used_inode_addr(FILE *file_ptr, SuperBlock sb, int used_inode_addr[])
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

	for(int i = 0; i < used_inode_count; ++i)
		used_inode_addr[i] = calculate_inode_addr(sb, index_of_used_inodes[i]);

	return used_inode_count;
}

void set_last_modification(FILE *file_ptr, SuperBlock sb, int i_node_id, time_t time)
{
	iNode inode;

	int inode_addr = calculate_inode_addr(sb, i_node_id - 1);
	fseek(file_ptr, inode_addr, SEEK_SET);
	fread(&inode, sizeof(iNode), 1, file_ptr);
	inode.last_modification = time;
	fseek(file_ptr, inode_addr, SEEK_SET);
	fwrite(&inode, sizeof(iNode), 1, file_ptr);
}

bool is_it_a_parent_inode(FILE *file_ptr, SuperBlock sb, int i_node_id, int inode_addresses[], int used_inode_count)
{
	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].parent_inode_id == i_node_id)
			return true;
	}
	return false;
}

int calculate_free_inode_count(FILE *file_ptr, SuperBlock sb)
{
	int count = 0;
	int free_inode;
	fseek(file_ptr, sb.bitmap_inode_positon, SEEK_SET);

	for(int i = 0; i < sb.amount_of_i_nodes; ++i)
	{
		fread(&free_inode, sizeof(free_inode), 1, file_ptr);
		if(free_inode == 0)
			count++;
	}
	return count;
}

int calculate_free_block_count(FILE *file_ptr, SuperBlock sb)
{
	int count = 0;
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);

	for(int i = 0; i < sb.amount_of_block; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0)
			count++;
	}
	return count;
}

void calculate_director_and_file_count(FILE *file_ptr, SuperBlock sb, int* dir_countP, int *file_countP, int inode_addresses[], int used_inode_count)
{
	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(inode_arr[i].type == 0) // directory
			*dir_countP = *dir_countP + 1;
		else if(inode_arr[i].type == 1)
			*file_countP = *file_countP + 1;
	}
}

void clean_the_removed_directory_space(FILE *file_ptr, SuperBlock sb, vector<int> other_parent_id_blocks_index, Directory removed_dir, int id)
{
	int max_dir_count = calculate_max_dir_or_file_in_a_block(sb); // It means that a block holds at most this number of dir and file.

	int block_addr;
	int dir_count;
	int which_index;
	for(int i = 0; i < other_parent_id_blocks_index.size(); ++i)
	{
		bool first_time = false;
		block_addr = calculate_block_addr(sb, other_parent_id_blocks_index[i]); // This is block addr of given block index
		dir_count = find_place_of_directory(file_ptr, sb, block_addr, removed_dir, &first_time);
		which_index = i;
		if(first_time == true)
			break;
	}

	fseek(file_ptr, block_addr + (dir_count * sizeof(Directory)), SEEK_SET);
	Directory empty;
	empty.i_node_number = 0;
	char name[FileNameLength] = "-";
	strcpy(empty.file_name, name);
	fwrite(&empty, sizeof(Directory), 1, file_ptr);

	if(other_parent_id_blocks_index.size() > 1)
	{
		int addr = calculate_block_addr(sb, other_parent_id_blocks_index[which_index]);

		bool empty = is_all_free(file_ptr, sb, addr);

		if(empty == true)
		{
			int inode_addr = calculate_inode_addr(sb, id -1);
			iNode inode;
			fseek(file_ptr, inode_addr, SEEK_SET);
			fread(&inode, sizeof(iNode), 1, file_ptr);
			inode.direct_block[which_index] = -1;
			fseek(file_ptr, inode_addr, SEEK_SET);
			fwrite(&inode, sizeof(iNode), 1, file_ptr);

			BitMapBlock bmb;
			fseek(file_ptr, sb.bitmap_position, SEEK_SET);
			fread(&bmb, sizeof(bmb), 1, file_ptr);
			bmb.max_bitmap_block[other_parent_id_blocks_index[which_index]] = 0;

			fseek(file_ptr, sb.bitmap_position, SEEK_SET);
			fwrite(&bmb, sizeof(bmb), 1, file_ptr);

		}
	}
}

int find_place_of_directory(FILE *file_ptr, SuperBlock sb, int block_addr, Directory dir, bool *first_time)
{
	fseek(file_ptr, block_addr, SEEK_SET); // Makes the file_ptr the begining of addable block

	int max_dir_in_block = calculate_max_dir_or_file_in_a_block(sb); // This returns the max number of having directory or file in a block 
	
	Directory dir_arr[max_dir_in_block];
	int dir_count = 0;

	for(int i = 0; i < max_dir_in_block; ++i)
	{
		fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);

		if((strcmp(dir_arr[i].file_name, dir.file_name) == 0) && (dir_arr[i].i_node_number == dir.i_node_number))
		{
			*first_time = true;
			break;
		}
		dir_count++;
	}
	return dir_count;
}

void fill_array_with_zeros(int arr[], int size)
{
	for(int i = 0; i < size; ++i)
		arr[i] = 0;
}

void set_size_for_directory(FILE *file_ptr, SuperBlock sb, int i_node_id, int used_block_count)
{
	iNode inode;

	int inode_addr = calculate_inode_addr(sb, i_node_id - 1);
	fseek(file_ptr, inode_addr, SEEK_SET);
	fread(&inode, sizeof(iNode), 1, file_ptr);
	inode.size_of_file = sb.block_size * used_block_count;
	fseek(file_ptr, inode_addr, SEEK_SET);
	fwrite(&inode, sizeof(iNode), 1, file_ptr);
}

void free_bitmap_blocks_indexes_different_from_given(FILE *file_ptr, SuperBlock sb, int block_index[], int size)
{
	fseek(file_ptr, sb.bitmap_position, SEEK_SET);
	BitMapBlock bmb;
	fread(&bmb, sizeof(bmb), 1, file_ptr);
	int index = 0;

	for(int i = 0; i < sb.amount_of_block; ++i)
	{
		if(bmb.max_bitmap_block[i] == 0)
		{
			if(index < size)
			{
				block_index[index] = i;
				index++;
			}
		}
		if(index >= size)
			break;
	}
}

void clean_block(FILE *file_ptr, SuperBlock sb, vector<int> block_index)
{
	for(int i = 0; i < block_index.size(); ++i)
	{	
		int addr = calculate_block_addr(sb, block_index[i]);
		fseek(file_ptr, addr, SEEK_SET);
		char empty = '\0';
		for(int j = 0; j < sb.block_size; ++j)
			fwrite(&empty, sizeof(char), 1, file_ptr);
		
	}
}

bool is_all_free(FILE *file_ptr, SuperBlock sb, int block_addr)
{
	fseek(file_ptr, block_addr, SEEK_SET); // Makes the file_ptr the begining of addable block

	int max_dir_in_block = calculate_max_dir_or_file_in_a_block(sb); // This returns the max number of having directory or file in a block 
	
	Directory dir_arr[max_dir_in_block];

	for(int i = 0; i < max_dir_in_block; ++i)
	{
		fread(&dir_arr[i], sizeof(Directory), 1, file_ptr);
		if((strcmp(dir_arr[i].file_name, "-") != 0))
			return false;
	}
	return true;
}

void set_all_linked_size_and_time(FILE *file_ptr, SuperBlock sb, int inode_addresses[], int used_inode_count, char *lnsym_path, int file_size, bool time_control)
{
	iNode inode_arr[used_inode_count];

	for(int i = 0; i < used_inode_count; ++i)
	{
		fseek(file_ptr, inode_addresses[i], SEEK_SET);
		fread(&inode_arr[i], sizeof(iNode), 1, file_ptr);

		if(strcmp(inode_arr[i].lnsym_path, lnsym_path) == 0)
		{
			inode_arr[i].size_of_file = file_size;
			
			if(time_control)
				inode_arr[i].last_modification = time(0);
			
			fseek(file_ptr, inode_addresses[i], SEEK_SET);
			fwrite(&inode_arr[i], sizeof(iNode), 1, file_ptr);
		}
	}
}