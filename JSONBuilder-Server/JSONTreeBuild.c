/******************************************************************************
* FILENAME      : JSONTreeBuild                                                     
*									      
* DESCRIPTION   : Device server main module, which handles different events    
*                 from different modules.				      	
* 									      
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "jsontree.h"
#include "io.h"
#include "dirent.h" //POSIX wrapper for WIN32 directory APIs 

/*----------------------------------------------------------------------------*/

struct dirTreeNode_t{
	char name[64];
	unsigned type;
	unsigned int numchildren;
	void* children[21];
};

typedef struct dirTreeNode_t dirTreeNode;

static void* garbagecollect[256];

static struct jsontree_object final_tree;

static void* jsongarbagecollect[512];

static char path[2048];

volatile static unsigned int gci = 0;
volatile static unsigned int jsongci = 0;

static int recursivelistdir(const char *name, int indent,dirTreeNode * tempNode) //Recursively list directories and sub-directories
{
    DIR *dir;
    struct dirent *entry;
	dirTreeNode * tempChildNode;
	
	volatile unsigned int tk = 0;
	
	for(tk=0;tk<10;tk++)
	{
		tempNode->children[tk] = NULL;
	}
	
	tk=0;

    if (!(dir = opendir(name)))
        return (-1);

    while ((entry = readdir(dir)) != NULL) {
		
        if (entry->type == _A_SUBDIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
			
			tempChildNode = (dirTreeNode *)malloc(sizeof(dirTreeNode));
		
			memset(tempChildNode,0,sizeof(dirTreeNode));
				
			strcpy(tempChildNode->name,entry->d_name);
		
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
			tempChildNode->type = _A_SUBDIR;
			garbagecollect[gci++] = (void *)(tempChildNode);
			tempNode->children[tk++] = (void *)tempChildNode;
			if(tk > 20)
				break;
            //printf("%*s[%s]\n", indent, "", entry->d_name);
            if(recursivelistdir(path, indent + 2,tempChildNode)<0)
				break;
        } else if ((entry->type == _A_NORMAL) || (entry->type == _A_ARCH)){
			
			tempChildNode = (dirTreeNode *)malloc(sizeof(dirTreeNode));
		
			memset(tempChildNode,0,sizeof(dirTreeNode));
				
			strcpy(tempChildNode->name,entry->d_name);
			
            //printf("%*s- %s\n", indent, "", entry->d_name);			
			tempChildNode->type = _A_ARCH;	

			garbagecollect[gci++] = (void *)(tempChildNode);
			
			tempNode->children[tk++] = (void *)tempChildNode;
			
			if(tk > 20)
				break;
        }
    }
	closedir(dir);
	if(tk>20)
	{
		for(tk=0;tk<gci;tk++)
		{
			if(garbagecollect[tk] != NULL)
			{
				free(garbagecollect[tk]);
				garbagecollect[tk] = NULL;
			}
		}
		return (-1);
	}else{
		tempNode->numchildren = (tk);
		return 0;
	}
}

static struct jsontree_value *
convertToJSONTree(dirTreeNode * tempNode)
{
	volatile unsigned int k = 0; 
	volatile unsigned int j = 0;

	struct jsontree_object* dirobj = (struct jsontree_object*) malloc(sizeof(struct jsontree_object)); 
	memset(dirobj,0,sizeof(struct jsontree_object));
	jsongarbagecollect[jsongci] = (void *)(dirobj);
	jsongci+=1;
	dirobj->type = JSON_TYPE_OBJECT;
	
	//strcpy(path + strlen(path),tempNode->name);
	
	if(tempNode->numchildren){
		dirobj->count = tempNode->numchildren;
		struct jsontree_pair* pairsarray =(struct jsontree_pair*) malloc(tempNode->numchildren * sizeof(struct jsontree_pair)); 
		memset(pairsarray,0,tempNode->numchildren * sizeof(struct jsontree_pair));
		dirobj->pairs = pairsarray;
		jsongarbagecollect[jsongci] = (void *)(pairsarray);
		jsongci+=1;
		for(k=0;k<tempNode->numchildren;k++)
		{
			if(tempNode->children[k] != NULL)
			{
				dirTreeNode *node = (dirTreeNode *)tempNode->children[k];
				if(node->type == _A_ARCH){ //Normal file
					struct jsontree_pair* filepair = (struct jsontree_pair*)(&(pairsarray[j++])); 
					strcpy(filepair->name,"F");
					struct jsontree_string* filestr = (struct jsontree_string*) malloc(sizeof(struct jsontree_string)); 
					memset(filestr,0,sizeof(struct jsontree_string));
					jsongarbagecollect[jsongci] = (void *)(filestr);
					jsongci+=1;
					filestr->type = JSON_TYPE_STRING;
					unsigned int filestrlen = strlen(node->name) + 1;
					filestr->value = (char*)malloc(filestrlen);
					memset(filestr->value,0,filestrlen);
					jsongarbagecollect[jsongci] = (void *)(filestr->value);
					jsongci+=1;
					strcpy(filestr->value,node->name);	
					filepair->value = (struct jsontree_value *)filestr;
				}
			}
		}
		for(k=0;k<tempNode->numchildren;k++)
		{
			if(tempNode->children[k] != NULL)
			{
				dirTreeNode *node = (dirTreeNode *)tempNode->children[k];
				if(node->type == _A_SUBDIR){//Subdirectory
					struct jsontree_pair* subdirpair = (struct jsontree_pair*)(&(pairsarray[j++])); 
					strcpy(subdirpair->name,node->name);
					//strcpy(path + strlen(path),"/");
					subdirpair->value = convertToJSONTree(node);
				}
			}
		}
	}
	return (struct jsontree_value *)(dirobj);
}

void purgeJSONTree(){
	volatile unsigned int j;
	for(j=0;j<jsongci;j++)
	{
		if(jsongarbagecollect[j] != NULL){
			free(jsongarbagecollect[j]);
			jsongarbagecollect[j] = NULL;
		}
	}
}

void* JSONTreeBuild()
{
	dirTreeNode * dirTreeRoot;
	
	memset(garbagecollect,0,256*sizeof(void*));
	
	dirTreeRoot = (dirTreeNode *)malloc(sizeof(dirTreeNode));
	
	memset(dirTreeRoot,0,sizeof(dirTreeNode));
	
	strcpy(dirTreeRoot->name,".");
	
	dirTreeRoot->type = _A_SUBDIR;
	
	garbagecollect[gci++] = (void *)(dirTreeRoot);
	
	if(recursivelistdir(".", 0,dirTreeRoot) == -1)
		return NULL;
	
	struct jsontree_pair* rootdirpair = (struct jsontree_pair*) malloc(sizeof(struct jsontree_pair)); 
	memset(rootdirpair,0,sizeof(struct jsontree_pair));
	jsongarbagecollect[jsongci] = (void *)(rootdirpair);
	jsongci+=1;
	strcpy(rootdirpair->name,dirTreeRoot->name);
	
	rootdirpair->value = convertToJSONTree(dirTreeRoot);
	
	memset(&final_tree,0,sizeof(struct jsontree_object));
	
	final_tree.type = JSON_TYPE_OBJECT;
	final_tree.count = 1;
	final_tree.pairs = rootdirpair;
	
	volatile unsigned int k;
	
	for(k=0;k<gci;k++)
	{
		if(garbagecollect[k] != NULL){
			free(garbagecollect[k]);
			garbagecollect[k] = NULL;
		}
	}

	return (void*)(&final_tree);
}
