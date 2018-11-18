/******************************************************************************
* FILENAME      : JSONTreeBuild                                                     
*									      
* DESCRIPTION   : Client device main module, which handles different events    
*                 from different modules.				      	
* 									      
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "jsonparse.h"
#include "io.h"
#include "dll.h"

//{\"F\":\"dirent.c\",\"F\":\"dirent.h\",\"F\":\"dirent.o\",\"F\":\"json.h\",\"F\":\"jsonparse.h\",\"F\":\"jsontree.c\",\"F\":\"jsontree.h\",\"F\":\"jsontree.o\",\"F\":\"JSONTreeBuild.c\",\"F\":\"JSONTreeBuild.o\",\"F\":\"Makefile.win\",\"F\":\"Project1.dev\",\"F\":\"Project1.exe\",\"F\":\"Project1.layout\",\"F\":\"serverperprocessmodules.c\",\"F\":\"serverperprocessmodules.o\",\"Test\":{\"F\":\"New Text Document.txt\",\"Test1\":{\"F\":\"New Text Document.txt\",\"Test2\":{\"F\":\"New Text Document.txt\",\"Test3\":{}}}}}

struct dirTreeNode_t{
	char name[64];
	unsigned type;
	unsigned int numchildren;
	void* children[21];
};

typedef struct dirTreeNode_t dirTreeNode;

void* garbagecollect[256];

static dirTreeNode * dirTreeRoot;

static dirTreeNode * curTreeRoot; //Keep track of current directory

static struct jsonparse_state js;

static char rqpath[2048] = ".";

volatile static unsigned int gci = 0;

//char JSONTreeBuf[] = "{\"F\":\"dirent.c\",\"F\":\"dirent.h\",\"F\":\"dirent.o\",\"F\":\"json.h\",\"F\":\"jsonparse.h\",\"F\":\"jsontree.c\",\"F\":\"jsontree.h\",\"F\":\"jsontree.o\",\"F\":\"JSONTreeBuild.c\",\"F\":\"JSONTreeBuild.o\",\"F\":\"Makefile.win\",\"F\":\"Project1.dev\",\"F\":\"Project1.exe\",\"F\":\"Project1.layout\",\"F\":\"serverperprocessmodules.c\",\"F\":\"serverperprocessmodules.o\",\"Test\":{\"F\":\"New Text Document.txt\",\"Test1\":{\"F\":\"New Text Document.txt\",\"Test2\":{\"F\":\"New Text Document.txt\",\"Test3\":{}}}}}";

DLLIMPORT void purgeDirTree()
{
	volatile unsigned int k;
	
	for(k=0;k<gci;k++)
	{
		if(garbagecollect[k] != NULL){
			free(garbagecollect[k]);
			garbagecollect[k] = NULL;
		}
	}
}

static dirTreeNode *GetNodeAddr(char* dirpath){
	volatile unsigned int k = 0;	
	dirTreeNode *retNode = NULL;
	volatile unsigned int offset = 0;
	dirTreeNode *tempNode = dirTreeRoot;
	dirTreeNode *tempParentNode = dirTreeRoot;
	unsigned int len = strlen(dirpath);
	volatile unsigned int tk = 0; //for children of dirNode
	
	do{
		if(strncmp(tempNode->name,&dirpath[offset],strlen(tempNode->name)) == 0)
		{
			if(dirpath[offset + strlen(tempNode->name)]){
				if(dirpath[offset + strlen(tempNode->name)]!='/'){
					tk+=1;
					tempNode = tempParentNode->children[tk]; //Check next child
					continue;
				}
			}
			for(k=offset;k<len;k++)
			{
				if(dirpath[k]=='/')
					break;
			}
			offset = (k+1);
			if(offset >= len){
				retNode = tempNode;
				break;
			}
			tempParentNode = tempNode;
			tempNode = tempParentNode->children[0];
			tk = 0;
		}else{
			tk+=1;
			tempNode = tempParentNode->children[tk]; //Check next child
		}
	}while((offset<len) && (tk < 20) && (tempNode));
	
	return retNode;
}

DLLIMPORT void* getCommand(void* filename){//Only check if the filename is valid. The Python script must take care of rest
	volatile unsigned int i = 0;
	bool bfullpath = false;
	char* temp = (char*)filename;
	for(i=0;i<strlen(filename);i++)
	{
		if(temp[i] == '/' && temp[i+1]!='\0')
		{
			bfullpath = true; //This is a full path
			break;
		}
	}
	
	if(bfullpath){
		if(GetNodeAddr(temp))
		{
			return (void*)(rqpath);
		}
	}else{
		for(i=0;i<20;i++)
		{
			if(curTreeRoot->children[i]){
				if(strncmp(((dirTreeNode*)(curTreeRoot->children[i]))->name,temp,strlen(temp))==0)
				{
					return (void*)(rqpath);
				}
			}
		}
	}
	return NULL;
}

static int recursiveInterpret(void* dirpath,void* JSONTreeBuf)
{
	volatile int type;
	char* cJSONTreeBuf = (char*)JSONTreeBuf;
	volatile unsigned int tk = 0;
	while((type = jsonparse_next(&js))){
		if(type == JSON_TYPE_PAIR_NAME){
			tk = 0;
			dirTreeNode *newNode;
			newNode = (dirTreeNode *)malloc(sizeof(dirTreeNode));
			memset(newNode,0,sizeof(dirTreeNode));
			garbagecollect[gci] = (void*)newNode;
			gci+=1;
			int len = 0;
			while(cJSONTreeBuf[js.vstart + len] != '"'){
				len++;
			}
			jsonparse_copy_value(&js, (char*)(newNode->name), (len+1));
			if(strncmp(newNode->name,".",1)==0){ //If there is another '.' in the JSON Buffer, simply discard
				free(newNode);
				continue;
			}
			if((type = jsonparse_next(&js)) == JSON_TYPE_STRING){//This is a file
				printf("\nFile success\n");
				memset(newNode,0,sizeof(dirTreeNode));
				newNode->type = _A_ARCH;
				len = 0;
				while(cJSONTreeBuf[js.vstart + len] != '"'){
					len++;
				}
				jsonparse_copy_value(&js, (char*)(newNode->name), (len+1));
				dirTreeNode *tempParent = GetNodeAddr((char*)(dirpath));
				while(tempParent->children[tk]){
					tk+=1;
				};
				tempParent->children[tk] = newNode;
				tempParent->numchildren +=1;
			}else {//This is a sub-directory
				printf("\nSUBDIR success\n");
				newNode->type = _A_SUBDIR;
				dirTreeNode *tempParent = GetNodeAddr((char*)(dirpath));
				while(tempParent->children[tk]){
					tk+=1;
				};
				tempParent->children[tk] = newNode;
				tempParent->numchildren +=1;
				if(cJSONTreeBuf[js.vstart + js.vlen + 3] != '"'){
					continue;
				}
				volatile unsigned int tempLen = strlen(dirpath);
				strcat(dirpath,"/");
				strcat(dirpath,newNode->name);
				recursiveInterpret(dirpath,JSONTreeBuf);
				*((char*)(dirpath + tempLen)) = '\0';
			}
		} else if(type == '}'){
			return 0;
		}
	}
	return 0;
}
	
DLLIMPORT int JSONTreeInterpret(void* JSONTreeBuf)
{
	jsonparse_setup(&js, (const char *)JSONTreeBuf, strlen((const char *)JSONTreeBuf));
	return recursiveInterpret((void*)(&rqpath[0]),JSONTreeBuf);
}

static void printDirTree(dirTreeNode *printtree,int tab){
	volatile int i;
	for(i=0;i<20;i++){
		if(printtree->children[i]){
			if(((dirTreeNode*)(printtree->children[i]))->type == _A_ARCH){
				printf("%*s- %s\n", tab, "", ((dirTreeNode*)(printtree->children[i]))->name);
			}else{
				printf("%*s[%s]\n", tab, "", ((dirTreeNode*)(printtree->children[i]))->name);
				//printDirTree(printtree->children[i],tab+1);
			}
		}
	}
}

DLLIMPORT void listDir(){ //Call this from the Python file when the user inputs "ls"
	printDirTree(curTreeRoot,0);
}

DLLIMPORT int changeDir(void* subDir){ //Call this from the Python file when the user inputs "cs path". Give the path to this function
	volatile unsigned int i = 0;
	bool bfullpath = false;
	char* temp = (char*)subDir;
	char goBack[] = "..";
	for(i=0;i<strlen(subDir);i++)
	{
		if(temp[i] == '/' && temp[i+1]!='\0')
		{
			bfullpath = true; //This is a full path
			break;
		}
	}
	i=0;
	dirTreeNode *tempNode = NULL;
	if(bfullpath){
		tempNode = GetNodeAddr(temp);
		if(tempNode){
			curTreeRoot = tempNode;
			memset(rqpath,0,sizeof(rqpath));
			strcpy(rqpath,temp);
			if(((dirTreeNode*)(curTreeRoot->children[0]))==NULL){//Dir is empty
					return 2; //If this is returned to the Python script, then send a request to server to get this subdir content
			}
		}
	}else if(strncmp(goBack,temp,strlen(goBack))==0){
		if(curTreeRoot != dirTreeRoot){
			volatile unsigned int k;
			for(k= strlen(rqpath)-1;k>0;k--){
				if(rqpath[k] == '/'){
					rqpath[k] = 0;
					break;
				}
				rqpath[k] = 0;
			}
			tempNode = GetNodeAddr(&rqpath[0]);
			if(tempNode){
				curTreeRoot = tempNode;
			}
		}
	}else{
		if(temp[0] == '.'){//The root of the tree
			memset(rqpath,0,sizeof(rqpath));
			rqpath[0] = '.';
			curTreeRoot = dirTreeRoot;
		}else{
			if(strncmp(curTreeRoot->name,temp,strlen(curTreeRoot->name))==0){
				if(temp[strlen(curTreeRoot->name)]!= '\0'){
					if(temp[strlen(curTreeRoot->name)]=='/'){
						return 0;
					}
				}else{
					return 0;
				}
			}
			for(i=0;i<20;i++)
			{
				if(curTreeRoot->children[i]){
					if(strncmp(((dirTreeNode*)(curTreeRoot->children[i]))->name,temp,strlen(((dirTreeNode*)(curTreeRoot->children[i]))->name))==0)
					{
						if(temp[strlen(((dirTreeNode*)(curTreeRoot->children[i]))->name)]){
							if(temp[strlen(((dirTreeNode*)(curTreeRoot->children[i]))->name)]!='/'){
								continue;
							}
						}
						strcat(rqpath,"/");
						strncat(rqpath,temp,strlen(((dirTreeNode*)(curTreeRoot->children[i]))->name));
						curTreeRoot = (dirTreeNode*)(curTreeRoot->children[i]);
						if(((dirTreeNode*)(curTreeRoot->children[0]))==NULL){//Dir is empty
							return 2; //If this is returned to the Python script, then send a request to server to get this subdir content
						}
						tempNode = 1;
						break;
					}
				}
			}
		}
	}
	if((tempNode == NULL) || (i >=20))
	{
		return 1; //If this is returned to the Python script, then print error message saying that the dir doesn't exist
	}
	return 0;
}

DLLIMPORT void* InitDirTree()
{
	
	memset(garbagecollect,0,256*sizeof(void*));
	
	dirTreeRoot = (dirTreeNode *)malloc(sizeof(dirTreeNode));
	
	memset(dirTreeRoot,0,sizeof(dirTreeNode));
	
	strcpy(dirTreeRoot->name,".");
	
	dirTreeRoot->type = _A_SUBDIR;
	
	garbagecollect[gci] = (void *)(dirTreeRoot);
	
	gci+=1;
	
	curTreeRoot = dirTreeRoot;

	return (void*)(rqpath);
}
