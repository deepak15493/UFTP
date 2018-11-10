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
#include "jsonparse.h"
#include "io.h"
#include "dirent.h" //POSIX wrapper for WIN32 directory APIs 

/*static int
accessPoints_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "SetAP", 5) == 0)
  {
    jsontree_write_atom(js_ctx, "{\"APSet\": \"");
    jsontree_write_atom(js_ctx, (char*)cfgRam.aWifiSSID);
    jsontree_write_atom(js_ctx, "(Device Server not connected!)\"}");
  }

  return 0;
}

static void swap (scramble_data_t *a, scramble_data_t *b)
{
    scramble_data_t temp; 
	temp.arr_size= a->arr_size;
	temp.arr = a->arr;
    a->arr_size = b->arr_size;
    a->arr = b->arr;
    b->arr_size = temp.arr_size;
    b->arr = temp.arr;
}

scramble_data_t *scram;

// A function to generate a random permutation of arr[]
static void scramble ( scramble_data_t arr[], unsigned int n )
{
	unsigned int i;
    // Use a different seed value so that we don't get same
    // result each time we run this program
    srand ( time(NULL) );
 
    // Start from the last element and swap one by one. We don't
    // need to run for the first element that's why i > 0
    for (i = n-1; i > 0; i--)
    {
        // Pick a random index from 0 to i
        int j = rand() % (i+1);
 
        // Swap arr[i] with the element at random index
        swap(&arr[i], &arr[j]);
    }
}

/*---------------------------------------------------------------------------*/
/*static void
json_copy_string(struct jsonparse_state *parser, char *string, int len)
{
  jsonparse_next(parser);
  jsonparse_copy_value(parser, string, len);
}
/*---------------------------------------------------------------------------*/
/*static int
accessPoint_select(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;
  bool flag = true;
  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "AP") == 0) {
        memset(&cfgRam.aWifiSSID[0],0,strlen((char*)cfgRam.aWifiSSID));
        json_copy_string(parser, (char*)cfgRam.aWifiSSID, 20);
        APSet = true;
      } else if(jsonparse_strcmp_value(parser, "PWD") == 0) {
        memset(&cfgRam.aWifiPassword[0],0,strlen((char*)cfgRam.aWifiPassword));
        json_copy_string(parser, (char*)cfgRam.aWifiPassword, 20);
        PWDSet = true;
      } else {
        flag = false;
      }
    }
  }

  return ((flag == false)? -1 : 0);
}

static void stringify(struct jsontree_context *js_ctx, uint8_t *buf, uint8_t size)
{
  uint8_t i = 0;
  char temp_buf[5];

  for(i=0; i<(size-1); i++)
  {
    sprintf(&temp_buf[0],"0x%02x",buf[i]);
    jsontree_write_atom(js_ctx, temp_buf);
    jsontree_write_atom(js_ctx, ",");
  }
  sprintf(&temp_buf[0],"0x%02x",buf[i]);
  jsontree_write_atom(js_ctx, temp_buf);

}
/*---------------------------------------------------------------------------*/
/*static int
cfg_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "MACAddr", 7) == 0) {
    jsontree_write_atom(js_ctx, "\"{");
    stringify(js_ctx,cfgRam.MACADDR, sizeof(cfgRam.MACADDR)-1);
    jsontree_write_atom(js_ctx, "}\"");
  } else if(strncmp(path, "PANID", 5) == 0) {
    jsontree_write_atom(js_ctx, "\"{");
    stringify(js_ctx,cfgRam.PANID, sizeof(cfgRam.PANID)-1);
    jsontree_write_atom(js_ctx, "}\"");
  } else if(strncmp(path, "ISMBand", 7) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.ISMBAND);
  } else if(strncmp(path, "DataRate", 8) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.DATA_RATE);
  } else if(strncmp(path, "MACSecurity", 11) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.MAC_SECURITY);
  } else if(strncmp(path, "SecKey", 6) == 0) {
    jsontree_write_atom(js_ctx, "\"{");
    stringify(js_ctx,cfgRam.SECURITY_KEY, sizeof(cfgRam.SECURITY_KEY)-1);
    jsontree_write_atom(js_ctx, "}\"");
  } else if(strncmp(path, "SyncInterval", 12) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.SYNC_INTERVAL);
  } else if(strncmp(path, "NotifyMSGDelay", 14) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.NOTIFY_MSG_DELAY);
  } else if(strncmp(path, "SlotInterval", 12) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.SLOT_INTERVAL);
  } else if(strncmp(path, "RXWindow", 8) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.RX_WINDOW);
  } else if(strncmp(path, "MaxNodesNotify", 14) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.MAX_NODES_NOTIFY);
  } else if(strncmp(path, "TFTPMaxRetrans", 14) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.TFTP_MAX_RETRANS);
  } else if(strncmp(path, "TFTPBlockSize", 13) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.TFTP_BLOCK_SIZE);
  } else if(strncmp(path, "OTAFUBlockInterval", 15) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.OTA_FU_BLOCK_INTERVAL);
  } else if(strncmp(path, "LogMethod", 9) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.LOG_METHOD);
  } else if(strncmp(path, "DBGLVL", 6) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.DEBUG_LEVEL);
  } else if(strncmp(path, "NodesRegistered", 6) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.NODESREGISTERED);
  }

  return 0;
}

static bool checkrange(uint8_t* c, uint8_t* pOut)
{
  return ((sscanf((char*)c,"%hx",pOut) > 0)? true : false);
}

static bool unstringify(struct jsonparse_state *parser, uint8_t *buf, uint8_t num)
{
  uint8_t i = 0;
  uint8_t len = (parser->vlen) + (parser->vstart);
  uint8_t temp_len = 0;

  for(i=parser->vstart; i < len; i++)
  {
    if((parser->json[i] == 'x') || (parser->json[i] == 'X'))
    {
      temp_len++;
    }
  }

  if(temp_len != num)
    return false;

  temp_len = 0;

  for(i=parser->vstart; i < len; i++)
  {
    if((parser->json[i] == 'x') || (parser->json[i] == 'X'))
    {
      if(checkrange((uint8_t*)&parser->json[i+1],(uint8_t*)(buf + temp_len)) == 0u)
      {
        return false;
      }
	  temp_len++;
    }
  }

  return true;
}

static int
cfg_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;

  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "PANID") == 0) {
        jsonparse_next(parser);
        if(unstringify(parser,cfgRam.PANID,sizeof(cfgRam.PANID)-1) == 0)
          return -1;
      }else if(jsonparse_strcmp_value(parser, "MACSecurity") == 0) {
        jsonparse_next(parser);
        cfgRam.MAC_SECURITY = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "SecKey") == 0) {
        jsonparse_next(parser);
        if(unstringify(parser,cfgRam.SECURITY_KEY,sizeof(cfgRam.SECURITY_KEY)-1) == 0)
          return -1;
      } else if(jsonparse_strcmp_value(parser, "SyncInterval") == 0) {
        jsonparse_next(parser);
        cfgRam.SYNC_INTERVAL = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "NotifyMSGDelay") == 0) {
        jsonparse_next(parser);
        cfgRam.NOTIFY_MSG_DELAY = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "SlotInterval") == 0) {
        jsonparse_next(parser);
        cfgRam.SLOT_INTERVAL = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "RXWindow") == 0) {
        jsonparse_next(parser);
        cfgRam.RX_WINDOW = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "MaxNodesNotify") == 0) {
        jsonparse_next(parser);
        cfgRam.MAX_NODES_NOTIFY = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "TFTPMaxRetrans") == 0) {
        jsonparse_next(parser);
        cfgRam.TFTP_MAX_RETRANS = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "TFTPBlockSize") == 0) {
        jsonparse_next(parser);
        cfgRam.TFTP_BLOCK_SIZE = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "OTAFUBlockInterval") == 0) {
        jsonparse_next(parser);
        cfgRam.OTA_FU_BLOCK_INTERVAL = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "LogMethod") == 0) {
        jsonparse_next(parser);
        cfgRam.LOG_METHOD = jsonparse_get_value_as_int(parser);
      } else if(jsonparse_strcmp_value(parser, "DBGLVL") == 0) {
        jsonparse_next(parser);
        cfgRam.DEBUG_LEVEL = jsonparse_get_value_as_int(parser);
      } else {
        return -1;
      }
    }
  }
  return 0;
}

static int
mqtt_cfg_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "TopicName", 9) == 0) {
    jsontree_write_string(js_ctx, (char*)cfgRam.aMQTTTopicName);
  } else if(strncmp(path, "BrokerIP", 8) == 0) {
    jsontree_write_string(js_ctx, (char*)cfgRam.aMQTTBrokerIp);
  } else if(strncmp(path, "BrokerPort", 10) == 0) {
    jsontree_write_string(js_ctx, (char*)cfgRam.aMQTTBrokerPort);
  }

  return 0;
}

static int
mqtt_cfg_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;
  uint8_t i, j =0;
  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "TopicName") == 0) {
        memset(&cfgRam.aMQTTTopicName[0],0,strlen((char*)cfgRam.aMQTTTopicName));
        json_copy_string(parser, (char*)cfgRam.aMQTTTopicName, 20);
      } else if(jsonparse_strcmp_value(parser, "BrokerIP") == 0) {
        jsonparse_next(parser);
        i = parser->vstart;
        while(i < (parser->vstart + parser->vlen)){
          if(parser->json[i] == '.')
          {
            j++;
          }
          i++;
        }
        if(j != 3)
          return -1;
        memset(&cfgRam.aMQTTBrokerIp[0],0,strlen((char*)cfgRam.aMQTTBrokerIp));
        jsonparse_copy_value(parser, (char*)cfgRam.aMQTTBrokerIp, 20);
      } else if(jsonparse_strcmp_value(parser, "BrokerPort") == 0) {
        memset(&cfgRam.aMQTTBrokerPort[0],0,strlen((char*)cfgRam.aMQTTBrokerPort));
        json_copy_string(parser, (char*)cfgRam.aMQTTBrokerPort, 6);
      } else {
        return -1;
      }
    }
  }
  return 0;
}

static int
lwm2m_cfg_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "ServerIP", 8) == 0) {
    jsontree_write_string(js_ctx, (char*)cfgRam.aLWM2MServerIp);
  } else if(strncmp(path, "ServerPort", 10) == 0) {
    jsontree_write_string(js_ctx, (char*)cfgRam.aLWM2MServerPort);
  }

  return 0;
}

static int
lwm2m_cfg_set(struct jsontree_context *js_ctx, struct jsonparse_state *parser)
{
  int type;
  uint8_t i, j =0;
  while((type = jsonparse_next(parser)) != 0) {
    if(type == JSON_TYPE_PAIR_NAME) {
      if(jsonparse_strcmp_value(parser, "ServerIP") == 0) {
        jsonparse_next(parser);
        i = parser->vstart;
        while(i < (parser->vstart + parser->vlen)){
          if(parser->json[i] == '.')
          {
            j++;
          }
          i++;
        }
        if(j != 3)
          return -1;
        memset(&cfgRam.aLWM2MServerIp[0],0,strlen((char*)cfgRam.aLWM2MServerIp));
        jsonparse_copy_value(parser, (char*)cfgRam.aLWM2MServerIp, 20);
      } else if(jsonparse_strcmp_value(parser, "ServerPort") == 0) {
        memset(&cfgRam.aLWM2MServerPort[0],0,strlen((char*)cfgRam.aLWM2MServerPort));
        json_copy_string(parser, (char*)cfgRam.aLWM2MServerPort, 6);
      } else {
        return -1;
      }
    }
  }
  return 0;
}

static int
stats_get(struct jsontree_context *js_ctx)
{
  const char *path = jsontree_path_name(js_ctx, js_ctx->depth - 1);

  if(strncmp(path, "WiFiPKTLoss", 11) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.WIFIPKTLOSS);
  } else if(strncmp(path, "LWM2MPKTLoss", 12) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.LWM2MPKTLOSS);
  } else if(strncmp(path, "6LoWPANPKTLoss", 14) == 0) {
    jsontree_write_uint(js_ctx, cfgRam.SIXLOWPANPKTLOSS);
  }

  return 0;
}

struct jsontree_callback accesspoints_callback = JSONTREE_CALLBACK(accessPoints_get, accessPoint_select);

static struct jsontree_callback cfg_callback = JSONTREE_CALLBACK(cfg_get, cfg_set);

static struct jsontree_callback mqtt_cfg_callback = JSONTREE_CALLBACK(mqtt_cfg_get, mqtt_cfg_set);

static struct jsontree_callback lwm2m_cfg_callback = JSONTREE_CALLBACK(lwm2m_cfg_get, lwm2m_cfg_set);

static struct jsontree_callback stats_callback = JSONTREE_CALLBACK(stats_get, NULL);

JSONTREE_OBJECT(config_tree,
                JSONTREE_PAIR("MACAddr", &cfg_callback),
                JSONTREE_PAIR("PANID", &cfg_callback),
                JSONTREE_PAIR("ISMBand", &cfg_callback),
                JSONTREE_PAIR("DataRate", &cfg_callback),
                JSONTREE_PAIR("MACSecurity", &cfg_callback),
                JSONTREE_PAIR("SecKey", &cfg_callback),
                JSONTREE_PAIR("SyncInterval", &cfg_callback),
                JSONTREE_PAIR("NotifyMSGDelay", &cfg_callback),
                JSONTREE_PAIR("SlotInterval", &cfg_callback),
                JSONTREE_PAIR("RXWindow", &cfg_callback),
                JSONTREE_PAIR("MaxNodesNotify", &cfg_callback),
                JSONTREE_PAIR("TFTPMaxRetrans", &cfg_callback),
                JSONTREE_PAIR("TFTPBlockSize", &cfg_callback),
                JSONTREE_PAIR("OTAFUBlockInterval", &cfg_callback),
                JSONTREE_PAIR("LogMethod", &cfg_callback),
                JSONTREE_PAIR("DBGLVL", &cfg_callback),
                JSONTREE_PAIR("NodesRegistered", &cfg_callback));

JSONTREE_OBJECT(mqtt_config_tree,
                JSONTREE_PAIR("TopicName", &mqtt_cfg_callback),
                JSONTREE_PAIR("BrokerIP", &mqtt_cfg_callback),
                JSONTREE_PAIR("BrokerPort", &mqtt_cfg_callback));

JSONTREE_OBJECT(lwm2m_config_tree,
                JSONTREE_PAIR("ServerIP", &lwm2m_cfg_callback),
                JSONTREE_PAIR("ServerPort", &lwm2m_cfg_callback));

JSONTREE_OBJECT(stats_tree,
                JSONTREE_PAIR("WiFiPKTLoss", &stats_callback),
                JSONTREE_PAIR("LWM2MPKTLoss", &stats_callback),
                JSONTREE_PAIR("6LoWPANPKTLoss", &stats_callback));

JSONTREE_OBJECT(ap_tree,
                JSONTREE_PAIR("AP", &accesspoints_callback),
                JSONTREE_PAIR("SetAP", &accesspoints_callback),
                JSONTREE_PAIR("PWD", &accesspoints_callback));

JSONTREE_OBJECT(final_tree,
                JSONTREE_PAIR("AP", &ap_tree),
                JSONTREE_PAIR("PacketLossStats", &stats_tree),
                JSONTREE_PAIR("MQTTCFG", &mqtt_config_tree),
                JSONTREE_PAIR("LWM2MCFG", &lwm2m_config_tree),
                JSONTREE_PAIR("ServerCFG", &config_tree));
/*----------------------------------------------------------------------------*/

struct dirTreeNode_t{
	char name[64];
	unsigned type;
	unsigned int numchildren;
	void* children[20];
};

typedef struct dirTreeNode_t dirTreeNode;

void* garbagecollect[256];

static struct jsontree_object final_tree;

static struct jsontree_context jsonctx;

volatile static unsigned int outbuf_pos = 0;

static char jsonReturnBuf[16384];

void* jsongarbagecollect[512];

static char path[2048];

volatile static unsigned int gci = 0;
volatile static unsigned int jsongci = 0;

int recursivelistdir(const char *name, int indent,dirTreeNode * tempNode) //Recursively list directories and sub-directories
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

struct jsontree_value *
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
			temp_len += (k - (jsonctx.index[jsonctx.depth - 1]+1) + 1); //Add commas and final flower bracket
			if(outbuf_pos + temp_len > 16383){
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
			}
		}else if((jsonctx.depth-jsonctx.path)>2){
				jsonReturnBuf[outbuf_pos] = '}'; //Don't put the contents of this subdir in just yet
				outbuf_pos+=1;
				return -1;
		}
	}
	return c;
  }
  return 0;
}

void* fetchJSONBuffer(char* path){
	struct jsontree_value *v;
	
	memset(jsonReturnBuf,0,sizeof(jsonReturnBuf));
	outbuf_pos = 0;
	v = (struct jsontree_value *)(&final_tree);
	jsonctx.values[0] = v;
	jsontree_reset(&jsonctx);

	if(path[1] == '\0') {
		/* Default page: show full JSON tree. */
	} else {
		v = find_json_path(&jsonctx, &path[1]);
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

int main(int argc, char **argv)
{
	dirTreeNode * dirTreeRoot;
	
	memset(garbagecollect,0,256*sizeof(void*));
	
	dirTreeRoot = (dirTreeNode *)malloc(sizeof(dirTreeNode));
	
	memset(dirTreeRoot,0,sizeof(dirTreeNode));
	
	strcpy(dirTreeRoot->name,".");
	
	dirTreeRoot->type = _A_SUBDIR;
	
	garbagecollect[gci++] = (void *)(dirTreeRoot);
	
	recursivelistdir(".", 0,dirTreeRoot);
	
	memset(path,0,sizeof(path));
	
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
	fetchJSONBuffer(".");
	purgeJSONTree();

	return 0;
}
