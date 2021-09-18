/*
	cson
	File:/src/cson.c
	Date:2021.09.18
	By MIT License.
	Copyright (c) 2021 Suote127.All rights reserved.
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include"cson/cson.h"

void cson_error(CSON_Context *ctx,const char *msg)
{
	char *copy = (char*)malloc(strlen(msg) + 1);
	if (!copy) {
		ctx->msg = NULL;
		longjmp(ctx->state,0);
		return;
	}

	ctx->msg = strcpy(copy,msg);
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

