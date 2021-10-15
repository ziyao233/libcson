/*
	cson
	File:/src/cson.c
	Date:2021.10.15
	By MIT License.
	Copyright (c) 2021 Suote127.All rights reserved.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#include"cson/cson.h"

void cson_error(CSON_Context *ctx,const char *msg)
{
	char *copy = (char*)malloc(strlen(msg) + 16);
	if (!copy) {
		ctx->msg = NULL;
		longjmp(ctx->state,0);
		return;
	}

	sprintf(copy,"On line %lu: %s",ctx->line,msg);

	ctx->msg = copy;

	longjmp(ctx->state,0);
	return;
}

static int is_space(const char *p)
{
	return *p == ' ' || *p == '\t' || *p == '\r' ||
	       *p == '\n';
}

static void skip_space(CSON_Context *ctx)
{
	const char *p = ctx->p;
	while (is_space(p) && *p) {
		if (*p == '\n')
			ctx->line++;
		p++;
	}
	ctx->p = p;
	return;
}

typedef struct {
	enum {
		TOKEN_EOS = 256,
		TOKEN_STRING,
		TOKEN_INTEGER,
		TOKEN_REAL
	}type;
	union {
		char *string;
		int integer;
		double real;
	}value;
	size_t line;
}Token;

static void next(CSON_Context *ctx,
		 Token *token)
{
	skip_space(ctx);

	const char *p = ctx->p;
	token->line = ctx->line;

	if (!*p) {
		token->type = TOKEN_EOS;
		return;
	} else if (*p == '{' || *p == '}' || *p  == '[' ||
		   *p == ']' || *p == ',' || *p == ':') {
		   token->type = *p;
		   ctx->p++;
	} else if (*p == '\"') {
		p++;
		const char *start = p;
		while (*p != '\"' && *p) {
			if (*p == '\n')
				ctx->line++;
			p++;
		}
		if (!*p)
			cson_error(ctx,"Expected \",got EOS");

		size_t length = p - start;
		token->value.string = (char*)malloc(length + 1);
		if (!(token->value.string))
			cson_error(ctx,"Cannot allocate memory");
		token->type = TOKEN_STRING;
		strncpy(token->value.string,start,length);
		token->value.string[length] = '\0';
		p++;
		ctx->p = p;
	} else if (isdigit(*p)) {
		token->type = TOKEN_INTEGER;
		token->value.integer = (int)strtol(p,(char**)&p,0);
		ctx->p = p;
	} else {
		cson_error(ctx,"Unknow character");
	}	

	return;
}

static void prefetch(CSON_Context *ctx,Token *token)
{
	const char *tmp = ctx->p;
	next(ctx,token);
	ctx->p = tmp;
	return;
}

static void unexpected(CSON_Context *ctx,const char *expected,
		       Token token)
{
	static const char *name[] = {
					[TOKEN_EOS - TOKEN_EOS] = "EOS",
					[TOKEN_INTEGER - TOKEN_EOS] =
					  	"integer",
					[TOKEN_STRING - TOKEN_EOS] =
						"string",
					[TOKEN_REAL - TOKEN_EOS] =
						"real"
				   };

	char msg[128];
	if (token.type > 255) {
		sprintf(msg,"Expected %s, got %s",
			expected,name[token.type - TOKEN_EOS]);
	} else {
		sprintf(msg,"Expected %s, got '%c'",
			expected,(char)token.type);
	}

	cson_error(ctx,msg);
	return;
}

static void p_value(CSON_Context *ctx);
static void p_object(CSON_Context *ctx);
static void p_array(CSON_Context *ctx);

static void p_object(CSON_Context *ctx)
{
	Token token;

	prefetch(ctx,&token);
	ctx->level++;

	while (token.type != TOKEN_EOS) {
		if (token.type == '}')
			break;
		if (token.type != TOKEN_STRING)
			unexpected(ctx,"string",token);
		next(ctx,&token);
		ctx->path[ctx->level - 1] = token.value.string;

		next(ctx,&token);
		if (token.type != ':')
			unexpected(ctx,"':'",token);
	
		p_value(ctx);

		free(ctx->path[ctx->level - 1]);
		prefetch(ctx,&token);
		if (token.type != ',')
			break;
		next(ctx,&token);

		prefetch(ctx,&token);
	}

	ctx->level--;

	return;
}

static void p_array(CSON_Context *ctx)
{
	Token token;

	char temp[24] = {0};
	ctx->path[ctx->level] = temp;
	ctx->level++;

	prefetch(ctx,&token);
	int i = 0;
	while (token.type != TOKEN_EOS) {
		sprintf(temp,"%d",i);

		if (token.type == ']')
			break;

		p_value(ctx);

		prefetch(ctx,&token);
		if (token.type == ']') {
			break;
		} else if (token.type != ',') {
			unexpected(ctx,"','",token);
		}
		next(ctx,&token);
		i++;
	}

	ctx->level--;

	return;
}

static void p_value(CSON_Context *ctx)
{
	Token token;
	next(ctx,&token);

	if (token.type == TOKEN_STRING) {
		CSON_Value value;
		value.type		= CSON_STRING,
		value.value.string	= token.value.string;
		ctx->handler(ctx,&value);
		free(token.value.string);
	} else if (token.type == TOKEN_INTEGER) {
		CSON_Value value;
		value.type		= CSON_INTEGER;
		value.value.integer	= token.value.integer;
		ctx->handler(ctx,&value);
	} else if (token.type == '{') {
		p_object(ctx);
		next(ctx,&token);
		if (token.type != '}')
			unexpected(ctx,"'}'",token);
	} else if (token.type == '[') {
		p_array(ctx);
		next(ctx,&token);
		if (token.type != ']')
			unexpected(ctx,"']'",token);
	} else {
		unexpected(ctx,"value",token);
	}

	return;
}

int cson_parse(const char *src,
	       void (*handler)(CSON_Context *ctx,const CSON_Value *value),
	       char **msg,void *data)
{
	CSON_Context ctx = (CSON_Context) {
						.p	= src,
						.level	= 0,
						.msg	= NULL,
						.line	= 1,
						.handler= handler,
						.ud	= data
					  };

	if (setjmp(ctx.state)) {
		*msg = ctx.msg;
		return -1;
	}

	p_value(&ctx);
	
	return 0;
}

char *cson_fullpath(CSON_Context *ctx,char *buffer,size_t size)
{
	size_t used = 0;

	for (int level = 0;level < ctx->level;level++) {
		used += strlen(ctx->path[level]);
		if (used > size)
			return NULL;
		strcat(buffer,ctx->path[level]);
		if (level != ctx->level - 1) {
			buffer[used] = '.';
			buffer[used + 1] = '\0';
			used++;
		}
	}

	return buffer;
}
