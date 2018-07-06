#include<stdlib.h>
#include<stdio.h>

#define PAGE_SIZE 12

typedef struct freeBlock
{
	char *address;
	int Size;
	freeBlock *next;
};

typedef struct allBlocks
{
	char *initial_pointer;
	int freeBytes;
	char *pointing_Address;
	freeBlock *freeS_Head;
};

allBlocks **all_1KBs = (allBlocks**)malloc(sizeof(allBlocks*) * PAGE_SIZE);
int total_Blocks = -1;

void allocate_For1KB()
{
	total_Blocks++;
	all_1KBs[total_Blocks] = (allBlocks*)malloc(sizeof(allBlocks));
	all_1KBs[total_Blocks]->freeBytes = 1024;
	all_1KBs[total_Blocks]->freeS_Head = NULL;
	all_1KBs[total_Blocks]->initial_pointer = (char*)malloc(1024);
	printf("\nInitial address of block %d is %d\n", total_Blocks, all_1KBs[total_Blocks]->initial_pointer);
	all_1KBs[total_Blocks]->pointing_Address = all_1KBs[total_Blocks]->initial_pointer;
}

int Blockno_ptr(void *ptr)
{
	for (int i = 0; i <= total_Blocks; i++)
	{
		if ((void*)all_1KBs[i]->initial_pointer <= (void*)ptr && ((void*)(((char*)(all_1KBs[i]->initial_pointer) + 1024)) > (void*)ptr))
			return i;
	}
	return -1;
}

void print_Free(freeBlock *head)
{
	while (head != NULL)
	{
		printf("%d\t%d\n", head->address, head->Size);
		head = head->next;
	}
}

void Merge_free(freeBlock *head)
{
	freeBlock *prev = NULL;
	freeBlock *temp = head;
	while (temp->next != NULL)
	{
		if (temp->address + temp->Size == temp->next->address)
		{
			temp->Size += temp->next->Size;
			temp->next = temp->next->next;
		}
		else
			temp = temp->next;
	}
}

void *rajesh_malloc(int size)
{
	int index = 0;
	if (total_Blocks == -1)
		allocate_For1KB();
	while (total_Blocks != PAGE_SIZE)
	{
		if (all_1KBs[index]->freeBytes < size + 2)//increment index if that block cannot satisfy your need
		{
			index++;
			continue;
		}
		else
		{
			if (all_1KBs[index]->freeS_Head == NULL)
			{
				*(short*)all_1KBs[index]->pointing_Address = size;
				all_1KBs[index]->pointing_Address = all_1KBs[index]->pointing_Address + size + 2;
				all_1KBs[index]->freeBytes -= (size + 2);
				//printf("%d\n", all_1KBs[index]->initial_pointer);
				return (void*)(all_1KBs[index]->pointing_Address - size);
			}
			else
			{
				freeBlock *temp = all_1KBs[index]->freeS_Head;
				freeBlock *prev = NULL;
				//Traverse through free and return the address if it can satisfy your NEED
				while (temp != NULL)
				{
					if (temp->address + temp->Size == all_1KBs[index]->pointing_Address&&temp->next == NULL)
					{
						all_1KBs[index]->pointing_Address = temp->address;
						temp = NULL;
						break;
					}
					if (temp->Size >= size)
					{
						*((short*)(temp->address)) = size;
						all_1KBs[index]->freeBytes -= (size + 2);
						void *res = temp->address + 2;
						if (temp->Size == size)
							prev->address = temp->address;
						else
						{
							temp->address = temp->address + size;
							temp->Size -= size;
						}
						return res;
					}
					prev = temp;
					temp = temp->next;
				}
				if (temp == NULL)//if free list fails then you can allocate from pointing address IF POSSIBLE else move to next BLOCK
				{
					if ((1024 - (all_1KBs[index]->pointing_Address - all_1KBs[index]->initial_pointer)) >= size + 2)
					{
						all_1KBs[index]->pointing_Address = all_1KBs[index]->pointing_Address + size + 2;
						*(short*)all_1KBs[index]->pointing_Address = size;
						all_1KBs[index]->freeBytes -= (size + 2);
						return (void*)(all_1KBs[index]->pointing_Address - size);
					}
					else
						index++;
				}
			}
		}
		if (index > total_Blocks)//for new block
			allocate_For1KB();
	}
	return NULL;
}

void rajesh_free(void *ptr)
{
	printf("\nfreed address :%d\n", ptr);
	freeBlock *newfree_Block = (freeBlock*)malloc(sizeof(freeBlock));
	newfree_Block->address = ((char*)ptr - 2);
	newfree_Block->Size = *(short*)((char*)ptr - 2) + 2;
	newfree_Block->next = NULL;
	int index = Blockno_ptr(ptr);
	if (index == -1)
		return;
	freeBlock *temp = all_1KBs[index]->freeS_Head;
	freeBlock *prev = NULL;
	while (temp != NULL)
	{
		if (temp->address > ptr)
		{
			if (prev != NULL)
			{
				newfree_Block->next = prev->next;
				prev->next = newfree_Block;
			}
			else
			{
				all_1KBs[index]->freeS_Head = newfree_Block;
			}
			break;
		}
		prev = temp;
		temp = temp->next;
	}
	if (temp == NULL)
	{
		if (prev != NULL)
		{
			prev->next = newfree_Block;
		}
		else
			all_1KBs[index]->freeS_Head = newfree_Block;
	}
	all_1KBs[index]->freeBytes += newfree_Block->Size;
	//else
	//{
	//	freeBlock *t = temp;
	//	prev->next = newfree_Block;//THINK OF THESE LINES FORGET WHY I WROTE THEM
	//	newfree_Block->next = t;
	//}
	printf("\nBefore Merging\n");
	print_Free(all_1KBs[index]->freeS_Head);
	Merge_free(all_1KBs[index]->freeS_Head);
	printf("\nAfter Merging\n");
	print_Free(all_1KBs[index]->freeS_Head);
}
