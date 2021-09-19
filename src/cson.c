/*
	cson
	File:/src/cson.c
	Date:2021.09.19
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
