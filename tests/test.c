/*
	cson
	File:/tests/test.c
	Date:2021.09.20
	By MIT License.
	Copyright (c) 2021 Suote127.All rights reserved.
*/

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

int main(void)
{
	char *err = NULL;
	const char *test = ",1";
	if (cson_parse(test,print_value,&err,NULL))
		puts(err);
	return 0;
}
