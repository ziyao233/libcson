/*
	cson
	File:/src/cson/cson.h
	Date:2021.09.20
	By MIT License.
	Copyright (c) 2021 Suote127.All rights reserved.
*/

#ifndef __CSON_H_INC__
#define __CSON_H_INC__

#include<stdlib.h>
#include<setjmp.h>

typedef struct {
	enum {
		CSON_INTEGER,
		CSON_STRING,
		CSON_REAL
	}type;

	union {
		int integer;
		const char *string;
	}value;
}CSON_Value;

typedef struct CSON_Context {
	char *path[16];
	int level;
	const char *p;
	char *msg;
	jmp_buf state;
	unsigned long line;
	void *ud;
	void (*handler)(struct CSON_Context *ctx,const CSON_Value *value);
}CSON_Context;

#define cson_path(ctx) (((const char *[])(CSON_Context*)ctx->path))

int cson_parse(const char *src,
	       void (*handler)(CSON_Context *ctx,
			       const CSON_Value *value),
	       char **err,void *data);

void cson_error(CSON_Context *ctx,const char *error);
char *cson_fullpath(CSON_Context *ctx,char *buffer,size_t size);

#endif	// __CSON_H_INC__
