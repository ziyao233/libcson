/*
	cson
	File:/tests/test.c
	Date:2021.10.15
	By MIT License.
	Copyright (c) 2021 Suote127.All rights reserved.
*/

#include<assert.h>
#include<stdio.h>
#include<stdlib.h>

#include"cson/cson.h"

void print_value(CSON_Context *ctx,const CSON_Value *value)
{
	char key[256] = {0};
	if (!cson_fullpath(ctx,key,256))
		exit(-1);
	printf("%s:",key);

	if (value->type == CSON_STRING) {
		printf("%s\n",value->value.string);
	} else {
		printf("%d\n",value->value.integer);
	}

	return;
}

int main(int argc,const char *argv[])
{
	if (argc != 2) {
		fputs("Configure file path is required\n",stderr);
		return -1;
	}

	FILE *conf = fopen(argv[1],"r");
	assert(conf);
	fseek(conf,0,SEEK_END);
	long int size = ftell(conf);
	rewind(conf);

	char temp[1024] = {0};
	char *err = NULL;
	fread(temp,size,1,conf);
	fclose(conf);
	if (cson_parse(temp,print_value,&err,NULL))
		puts(err);
	return 0;
}
