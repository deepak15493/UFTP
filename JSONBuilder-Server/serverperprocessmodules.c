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

/*----------------------------------------------------------------------------*/

static struct jsontree_context jsonctx;

volatile static unsigned int outbuf_pos = 0;

static char jsonReturnBuf[16384];

struct jsontree_value *
find_json_path(struct jsontree_context *json, char *path)
{
  struct jsontree_value *v;
  char *start;
  char *end;
  int len;

  v = json->values[0];
  start = path;
  do {
    end = strchr(start, '/');
    if(end == start) {
      break;
    }
    if(end != NULL) {
      len = end - start;
      end++;
    } else {
      len = strlen(start);
    }
    if(v->type != JSON_TYPE_OBJECT) {
      v = NULL;
    } else {
      struct jsontree_object *o;
      int i;

      o = (struct jsontree_object *)v;
      v = NULL;
      for(i = 0; i < o->count; i++) {
        if(strncmp(start, o->pairs[i].name, len) == 0) {
          v = o->pairs[i].value;
          json->index[json->depth] = i;
          json->depth++;
          json->values[json->depth] = v;
          json->index[json->depth] = 0;
          break;
        }
      }
    }
    start = end;
  } while(end != NULL && *end != '\0' && v != NULL);
  json->callback_state = 0;
  return v;
}

static int
json_putchar(int c)
{
  if(outbuf_pos < 16384) {
	jsonReturnBuf[outbuf_pos] = c;
	outbuf_pos+=1;
	if((c == '{')){
		if((jsonctx.depth-jsonctx.path)==2){ //Put immediate sub-directories in the buffer, but not deeper
			struct jsontree_object* v = (struct jsontree_object* )jsonctx.values[jsonctx.depth];
			volatile unsigned int temp_len = 0;
			volatile unsigned int k = 0;
			for(k=0;k<v->count;k++)
			{
				struct jsontree_value* ov = v->pairs[k].value;
				temp_len += (3 + strlen(v->pairs[k].name)); //For double quotes and colon add 3, plus the name of the pair
				if(ov->type == JSON_TYPE_STRING){ //Is a normal file
					temp_len += (2 + strlen(((struct jsontree_string *)ov)->value)); //Add double quotes size and string length of the name of file
				}else if(ov->type == JSON_TYPE_OBJECT)
				{//Is a sub-directory
					temp_len += 2; //2 added for the flower brackets
				}
			}
			temp_len += (v->count); //Commas plus flower brace in the end
			v = (struct jsontree_object* )jsonctx.values[jsonctx.depth - 1];
			for(k=jsonctx.index[jsonctx.depth - 1]+1;k<v->count;k++)
			{
				temp_len += (5 + strlen(v->pairs[k].name)); //For double quotes, empty flower braces and colon add 5, plus the name of the object for subdir
			}
			temp_len += (k - (jsonctx.index[jsonctx.depth - 1]+1) + 2); //Add commas and final two flower brackets
			if(outbuf_pos + temp_len > 16383){
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
			}
		}else if((jsonctx.depth-jsonctx.path)==3){
			struct jsontree_object* v = (struct jsontree_object* )jsonctx.values[jsonctx.depth];
			volatile unsigned int temp_len = 0;
			volatile unsigned int k = 0;
			for(k=0;k<v->count;k++)
			{
				struct jsontree_value* ov = v->pairs[k].value;
				temp_len += (3 + strlen(v->pairs[k].name)); //For double quotes and colon add 3, plus the name of the pair
				if(ov->type == JSON_TYPE_STRING){ //Is a normal file
					temp_len += (2 + strlen(((struct jsontree_string *)ov)->value)); //Add double quotes size and string length of the name of file
				}else if(ov->type == JSON_TYPE_OBJECT)
				{//Is a sub-directory
					temp_len += 2; //2 added for the flower brackets
				}
			}
			temp_len += (v->count); //Commas plus flower brace in the end
			v = (struct jsontree_object* )jsonctx.values[jsonctx.depth - 1];
			for(k=jsonctx.index[jsonctx.depth - 1]+1;k<v->count;k++)
			{
				temp_len += (5 + strlen(v->pairs[k].name)); //For double quotes, empty flower braces and colon add 5, plus the name of the object for subdir
			}
			temp_len += (k - (jsonctx.index[jsonctx.depth - 1]+1) + 2); //Add commas and final two flower brackets
			v = (struct jsontree_object* )jsonctx.values[jsonctx.depth - 2];
			for(k=jsonctx.index[jsonctx.depth - 2]+1;k<v->count;k++)
			{
				temp_len += (5 + strlen(v->pairs[k].name)); //For double quotes, empty flower braces and colon add 5, plus the name of the object for subdir
			}
			temp_len += (k - (jsonctx.index[jsonctx.depth - 2]+1) + 2); //Add commas and final two flower brackets
			if(outbuf_pos + temp_len > 16383){
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
			}
		}else if((jsonctx.depth-jsonctx.path)>3){
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
		}
	}
	return c;
  }
  return 0;
}

void* fetchJSONBuffer(char* filepath,struct jsontree_object* final_tree){
	struct jsontree_value *v;
	
	memset(jsonReturnBuf,0,sizeof(jsonReturnBuf));
	outbuf_pos = 0;
	v = (struct jsontree_value *)(final_tree);
	jsonctx.values[0] = v;
	jsontree_reset(&jsonctx);

	if(filepath[1] == '\0') {
		/* Default page: show full JSON tree. */
	} else {
		v = find_json_path(&jsonctx, &filepath[0]);
	}
	if(v != NULL) {
		jsonctx.path = jsonctx.depth;
		jsonctx.putchar = json_putchar;
		while(jsontree_print_next(&jsonctx) && (jsonctx.path <= jsonctx.depth)) {
			//Keep putting the JSON tree into the buffer to send back to the client
		}
	}
	return (void *)(jsonReturnBuf);
}
