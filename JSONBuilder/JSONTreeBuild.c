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
#include "jsontree.h"
#include "jsonparse.h"
#include "json-ws.h"
#include "io.h"
#include "dirent.h" //POSIX wrapper for WIN32 directory APIs 

static int
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
static void
json_copy_string(struct jsonparse_state *parser, char *string, int len)
{
  jsonparse_next(parser);
  jsonparse_copy_value(parser, string, len);
}
/*---------------------------------------------------------------------------*/
static int
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
static int
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

unsigned long garbagecollect[256];

void recursivelistdir(const char *name, int indent)
{
    DIR *dir;
    struct dirent *entry;
	unsigned int i = 0;

    if (!(dir = opendir(name)))
        return;

    while ((entry = readdir(dir)) != NULL) {
        if (entry->type == _A_SUBDIR) {
            char path[1024];
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;
            snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
            printf("%*s[%s]\n", indent, "", entry->d_name);
            recursivelistdir(path, indent + 2);
        } else if (entry->type == _A_NORMAL){
            printf("%*s- %s\n", indent, "", entry->d_name);
        }
    }
    closedir(dir);
}

int constructJSONDirTree(int argc, char *argv)
{
	
	memset(garbagecollect,0,256*sizeof(unsigned long));
	
	recursivelistdir(".", 0);
	
	json_ws_init(&final_tree);

        if(bConnected){
			if((fdsReady > 0) && (FD_ISSET(newsockfd, &readfds)))
			{
				fdsReady--;
#if (PLATFORM_LINUX == 1)
				socketBytes = read(newsockfd,jsonInput,512);
#elif (PLATFORM_WINDOWS == 1)
				socketBytes = _read(newsockfd, jsonInput, 512);
#endif
				if(socketBytes > 0)
				{
					handle_input(pHTTPD_data, socketBytes);
#if (PLATFORM_LINUX == 1)
					socketBytes = write(newsockfd,httpOutput,strlen((char*)httpOutput));
#elif (PLATFORM_WINDOWS == 1)
					socketBytes = write(newsockfd, httpOutput, strlen((char*)httpOutput));
#endif

					if(APSet && PWDSet)
					{
						APSet = PWDSet = false;
						config_save();
						system("rm -f deviceServer_config.ini");
						system("mv deviceServer_config_new.ini deviceServer_config.ini");
					}

					if(socketBytes < 0){
						sprintf(logBuff, "[FAIL] Socket write failed");
						logMess(logBuff, DBG_LEVEL1);
						exit_ds();
					}
					close(newsockfd);
					bConnected = false;
					memset(httpOutput,0,1024);
				}else if(socketBytes < 0)
				{
					sprintf(logBuff, "[FAIL] Socket read failed");
					                   logMess(logBuff, DBG_LEVEL1);
					exit_ds();
				} else if(!(((errno & EAGAIN) == EAGAIN) || ((errno & EWOULDBLOCK)==EWOULDBLOCK)))
				{
					close(newsockfd);
					bConnected = false;
				}
			}
        }
}