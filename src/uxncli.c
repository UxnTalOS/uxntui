#include <stdio.h>
#include <stdlib.h>

#include "uxn.h"
#include "devices/system.h"
#include "devices/file.h"
#include "devices/datetime.h"

/*
Copyright (c) 2021 Devine Lu Linvega

Permission to use, copy, modify, and distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE.
*/

static int
error(char *msg, const char *err)
{
	fprintf(stderr, "Error %s: %s\n", msg, err);
	return 0;
}

static int
console_input(Uxn *u, char c)
{
	Uint8 *d = &u->dev[0x10];
	d[0x02] = c;
	return uxn_eval(u, GETVEC(d));
}

static void
console_deo(Uint8 *d, Uint8 port)
{
	FILE *fd = port == 0x8 ? stdout : port == 0x9 ? stderr
												  : 0;
	if(fd) {
		fputc(d[port], fd);
		fflush(fd);
	}
}

static Uint8
uxn11_dei(struct Uxn *u, Uint8 addr)
{
	Uint8 p = addr & 0x0f, d = addr & 0xf0;
	switch(d) {
	case 0xa0: return file_dei(0, &u->dev[d], p);
	case 0xb0: return file_dei(1, &u->dev[d], p);
	case 0xc0: return datetime_dei(&u->dev[d], p);
	}
	return u->dev[addr];
}

static void
uxn11_deo(Uxn *u, Uint8 addr, Uint8 v)
{
	Uint8 p = addr & 0x0f, d = addr & 0xf0;
	u->dev[addr] = v;
	switch(d) {
	case 0x00: system_deo(u, &u->dev[d], p); break;
	case 0x10: console_deo(&u->dev[d], p); break;
	case 0xa0: file_deo(0, u->ram, &u->dev[d], p); break;
	case 0xb0: file_deo(1, u->ram, &u->dev[d], p); break;
	}
}

static void
run(Uxn *u)
{
	while(!u->dev[0x0f]) {
		int c = fgetc(stdin);
		if(c != EOF)
			console_input(u, (Uint8)c);
	}
}

int
uxn_interrupt(void)
{
	return 1;
}

static int
start(Uxn *u)
{
	if(!uxn_boot(u, (Uint8 *)calloc(0x10200, sizeof(Uint8))))
		return error("Boot", "Failed");
	u->dei = uxn11_dei;
	u->deo = uxn11_deo;
	return 1;
}

int
main(int argc, char **argv)
{
	Uxn u;
	int i;
	if(argc < 2)
		return error("Usage", "uxncli game.rom args");
	if(!start(&u))
		return error("Start", "Failed");
	if(!load_rom(&u, argv[1]))
		return error("Load", "Failed");
	fprintf(stderr, ">> Loaded %s\n", argv[1]);
	if(!uxn_eval(&u, PAGE_PROGRAM))
		return error("Init", "Failed");
	for(i = 2; i < argc; i++) {
		char *p = argv[i];
		while(*p) console_input(&u, *p++);
		console_input(&u, '\n');
	}
	run(&u);
	return 0;
}
