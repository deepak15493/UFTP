/*
 * Copyright (c) 2011-2012, Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */

/**
 * \file
 *         JSON webservice util
 * \author
 *         Niclas Finne    <nfi@sics.se>
 *         Joakim Eriksson <joakime@sics.se>
 *         Joel Hoglund    <joel@sics.se>
 */
#include "jsontree.h"
#include "jsonparse.h"
#include "json-ws.h"
#include <stdio.h>
#include <string.h>

#define PRINTF(...)

static const char *callback_json_path = NULL;
static struct jsontree_object *tree;

/*---------------------------------------------------------------------------*/
static void
json_copy_string(struct jsonparse_state *parser, char *string, int len)
{
  jsonparse_next(parser);
  jsonparse_next(parser);
  jsonparse_copy_value(parser, string, len);
}
/*---------------------------------------------------------------------------*/
static struct httpd_ws_state *json_putchar_context;
static int
json_putchar(int c)
{
  if(json_putchar_context != NULL &&
     json_putchar_context->outbuf_pos < HTTPD_OUTBUF_SIZE) {
    json_putchar_context->outbuf[json_putchar_context->outbuf_pos++] = c;
    return c;
  }
  return 0;
}
static int putchar_size = 0;
static int
json_putchar_count(int c)
{
  putchar_size++;
  return c;
}
/*---------------------------------------------------------------------------*/
static
PT_THREAD(send_values(struct httpd_ws_state *s))
{
  json_putchar_context = s;

  PSOCK_BEGIN(&s->sout);

  s->json.putchar = json_putchar;
  s->outbuf_pos = 0;

  if(s->json.values[0] == NULL) {
    /* Nothing to do */

  } else if(s->request_type == HTTPD_WS_POST &&
            s->state == HTTPD_WS_STATE_OUTPUT) {
    /* Set value */
    struct jsontree_value *v;
    struct jsontree_callback *c;

    while((v = jsontree_find_next(&s->json, JSON_TYPE_CALLBACK)) != NULL) {
      c = (struct jsontree_callback *)v;
      if(c->set != NULL) {
        struct jsonparse_state js;

        jsonparse_setup(&js, s->inputbuf, s->content_len);
        c->set(&s->json, &js);
      }
    }
    memcpy(s->outbuf, "{\"Status\":\"OK\"}", 15);
    s->outbuf_pos = 15;

  } else {
    /* Get value */
    while(jsontree_print_next(&s->json) && s->json.path <= s->json.depth) {
      if(s->outbuf_pos >= UIP_TCP_MSS) {
        SEND_STRING(&s->sout, s->outbuf, UIP_TCP_MSS);
        s->outbuf_pos -= UIP_TCP_MSS;
        if(s->outbuf_pos > 0) {
          memcpy(s->outbuf, &s->outbuf[UIP_TCP_MSS], s->outbuf_pos);
        }
      }
    }
  }

  if(s->outbuf_pos > 0) {
    SEND_STRING(&s->sout, s->outbuf, s->outbuf_pos);
    s->outbuf_pos = 0;
  }
  PSOCK_END(&s->sout);
}
/*---------------------------------------------------------------------------*/
struct jsontree_value *
find_json_path(struct jsontree_context *json, const char *path)
{
  struct jsontree_value *v;
  const char *start;
  const char *end;
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
/*---------------------------------------------------------------------------*/
httpd_ws_script_t
httpd_ws_get_script(struct httpd_ws_state *s)
{
  struct jsontree_value *v;

  s->json.values[0] = v = (struct jsontree_value *)tree;
  jsontree_reset(&s->json);

  if(s->filename[1] == '\0') {
    /* Default page: show full JSON tree. */
  } else {
    v = find_json_path(&s->json, &s->filename[1]);
  }
  if(v != NULL) {
    s->json.path = s->json.depth;
    s->content_type = http_content_type_json;
    return send_values;
  }
  return NULL;
}
/*---------------------------------------------------------------------------*/
void
json_ws_init(struct jsontree_object *json)
{
  tree = json;
}
/*---------------------------------------------------------------------------*/