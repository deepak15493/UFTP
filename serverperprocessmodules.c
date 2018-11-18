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
#include <s_dll.h>
/*----------------------------------------------------------------------------*/

static struct jsontree_context jsonctx;

volatile static unsigned int outbuf_pos = 0;

static char jsonReturnBuf[16384];

static struct jsontree_value *
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
		if((jsonctx.depth-jsonctx.path)>=1){
			struct jsontree_object* v = (struct jsontree_object* )jsonctx.values[jsonctx.depth];
			volatile unsigned int temp_len = 0;
			volatile unsigned int k = 0;
			volatile int tdepth = 0;
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
			temp_len += (v->count - 1); //Commas 
			tdepth = jsonctx.depth - jsonctx.path;
			do{
				v = (struct jsontree_object* )jsonctx.values[(tdepth + jsonctx.path) - 1];
				for(k=jsonctx.index[(tdepth + jsonctx.path) - 1]+1;k<v->count;k++)
				{
					temp_len += (5 + strlen(v->pairs[k].name)); //For double quotes, empty flower braces and colon add 5, plus the name of the object for subdir
				}
				temp_len += (k - (jsonctx.index[(tdepth + jsonctx.path) - 1])); //Add commas and final one flower brackets
				tdepth--;
			}while(tdepth > 0);
			
			if(outbuf_pos + temp_len > 16383){//Putting the entire subdir content into the buffer would exceed our length limit.
				temp_len = 1; //Assume we're writing } to the buffer instead of the contents
				tdepth = jsonctx.depth - jsonctx.path;
				do{
					v = (struct jsontree_object* )jsonctx.values[(tdepth + jsonctx.path) - 1];
					for(k=jsonctx.index[(tdepth + jsonctx.path) - 1]+1;k<v->count;k++)
					{
						temp_len += (5 + strlen(v->pairs[k].name)); //For double quotes, empty flower braces and colon add 5, plus the name of the object for subdir
					}
					temp_len += (k - (jsonctx.index[(tdepth + jsonctx.path) - 1])); //Add commas and final one flower brackets
					tdepth--;
				}while(tdepth > 0);
				if(outbuf_pos + temp_len > 16383){
					//Even putting an empty subdir into buffer is causing it to overflow. Just stop putting the parent subdir itself
					volatile unsigned int j = 0;
					volatile int breakNow = 0;
					jsonReturnBuf[outbuf_pos-1] = 0; 
					for(j=outbuf_pos-2;j>0;j--){
						if(jsonReturnBuf[j]=='{')
						{
							breakNow++;
						}else if(jsonReturnBuf[j]=='}')
						{
							breakNow--;
						}
						if(breakNow==1)
							break;
						jsonReturnBuf[j] = 0; //Clear content till we reach the parent subdir label
					}
					outbuf_pos = j+1;
					jsonctx.depth--; //Decrement one level
				}
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
			}
		}
	}
	return c;
  }else{
	volatile unsigned int j = 0;
	volatile int breakNow = 0; 
	for(j=outbuf_pos-1;j>0;j--){
		if(jsonReturnBuf[j]=='{')
		{
			breakNow++;
		}else if(jsonReturnBuf[j]=='}')
		{
			breakNow--;
		}
		if(breakNow==1)
			break;
		jsonReturnBuf[j] = 0; //Clear content till we reach the parent subdir label
	}
	outbuf_pos = j+1;
	jsonctx.depth--; //Decrement one level
	jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
	outbuf_pos+=1;
	return -1;
  }
  return 0;
}

DLLIMPORT void* fetchJSONBuffer(char* filepath,struct jsontree_object* final_tree){
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
