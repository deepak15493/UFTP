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
#include "jsonparse.h"
#include "io.h"

//{"F":"dirent.c","F":"dirent.h","F":"dirent.o","F":"json.h","F":"jsonparse.h","F":"jsontree.c","F":"jsontree.h","F":"jsontree.o","F":"JSONTreeBuild.c","F":"JSONTreeBuild.o","F":"Makefile.win","F":"Project1.dev","F":"Project1.exe","F":"Project1.layout","F":"serverperprocessmodules.c","F":"serverperprocessmodules.o","Test":{"F":"New Text Document.txt","Test1":{"F":"New Text Document.txt","Test2":{"F":"New Text Document.txt","Test3":{}}}}}

struct dirTreeNode_t{
	char name[64];
	unsigned type;
	unsigned int numchildren;
	void* children[21];
};

typedef struct dirTreeNode_t dirTreeNode;

void* garbagecollect[256];

static dirTreeNode * dirTreeRoot;

static struct jsonparse_state js;

static char rqpath[2048] = "./";

volatile static unsigned int gci = 0;

char JSONBuf[] = "{\"F\":\"dirent.c\",\"F\":\"dirent.h\",\"F\":\"dirent.o\",\"F\":\"json.h\",\"F\":\"jsonparse.h\",\"F\":\"jsontree.c\",\"F\":\"jsontree.h\",\"F\":\"jsontree.o\",\"F\":\"JSONTreeBuild.c\",\"F\":\"JSONTreeBuild.o\",\"F\":\"Makefile.win\",\"F\":\"Project1.dev\",\"F\":\"Project1.exe\",\"F\":\"Project1.layout\",\"F\":\"serverperprocessmodules.c\",\"F\":\"serverperprocessmodules.o\",\"Test\":{\"F\":\"New Text Document.txt\",\"Test1\":{\"F\":\"New Text Document.txt\",\"Test2\":{\"F\":\"New Text Document.txt\",\"Test3\":{}}}}}";

void purgeDirTree()
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

dirTreeNode *GetNodeAddr(char* dirpath){
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
			tk = 0;
		}else{
			tempNode = tempParentNode->children[tk]; //Check next child
			tk+=1;
		}
	}while((offset<len) && (tk < 20) && (tempNode));
	
	return retNode;
}

int recursiveInterpret(void* dirpath,void* JSONTreeBuf)
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
			garbagecollect[gci++] = (void*)newNode;
			int len = 0;
			while(cJSONTreeBuf[js.vstart + len] != '"'){
				len++;
			}
			jsonparse_copy_value(&js, (char*)(newNode->name), (len+1));
			if((type = jsonparse_next(&js)) == JSON_TYPE_STRING){//This is a file
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
				tk+=1;
				tempParent->numchildren +=1;
			}else {//This is a sub-directory
				newNode->type = _A_SUBDIR;
				dirTreeNode *tempParent = GetNodeAddr((char*)(dirpath));
				while(tempParent->children[tk]){
					tk+=1;
				};
				tempParent->children[tk] = newNode;
				tk+=1;
				tempParent->numchildren +=1;
				if(cJSONTreeBuf[js.vstart + 1] == '}'){
					continue;
				}
				volatile unsigned int tempLen = strlen(dirpath);
				strcat(dirpath,newNode->name);
				strcat(dirpath,"/");
				recursiveInterpret(dirpath,JSONTreeBuf);
				*((char*)(dirpath + tempLen)) = '\0';
			}
		}
	}
}
	
int JSONTreeInterpret(void* dirpath,void* JSONTreeBuf)
{
	jsonparse_setup(&js, (const char *)JSONTreeBuf, strlen((const char *)JSONTreeBuf));
	recursiveInterpret(dirpath,JSONTreeBuf);
}

int main()
{
	
	memset(garbagecollect,0,256*sizeof(void*));
	
	dirTreeRoot = (dirTreeNode *)malloc(sizeof(dirTreeNode));
	
	memset(dirTreeRoot,0,sizeof(dirTreeNode));
	
	strcpy(dirTreeRoot->name,".");
	
	dirTreeRoot->type = _A_SUBDIR;
	
	garbagecollect[gci++] = (void *)(dirTreeRoot);
	
	JSONTreeInterpret((void*)(&rqpath[0]),(void*)(JSONBuf));
	
	purgeDirTree();

	return 0;
}
