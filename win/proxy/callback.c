/* $Id: callback.c,v 1.1 2002-01-31 22:21:26 j_ali Exp $ */
/* Copyright (c) Slash'EM Development Team 2001-2002 */
/* NetHack may be freely redistributed.  See license for details. */

/* #define DEBUG */

#include "hack.h"
#include "nhxdr.h"
#include "winproxy.h"
#include "proxycb.h"

static void FDECL(callback_display_inventory, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fopen, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fgets, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_dlbh_fclose, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_flush_screen, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_doredraw, \
			(unsigned short, NhExtXdr *, NhExtXdr *));
static void FDECL(callback_status_mode, \
			(unsigned short, NhExtXdr *, NhExtXdr *));

static void
callback_display_inventory(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    display_inventory((char *)0, FALSE);
}

#ifndef FILE_AREAS
#define SET_FILE(f, a) if (1) { file = f; } else
#else
#define SET_FILE(f, a) if (1) { file = f; area = a; } else
#endif

static void
callback_dlbh_fopen(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    char *name, *mode, *file;
#ifdef FILE_AREAS
    char *area;
#endif
    int i, retval = 0;
    nhext_rpc_params(request, 2, EXT_STRING_P(name), EXT_STRING_P(mode));
    if (name[0] != '$' || name[1] != '(')
	retval = -1;
    else {
	for(i = 0; name[i] && name[i] != ')'; i++)
	    ;
	if (!name[i] || name[i + 1])
	    retval = -1;
	else
	    name[i] = '\0';
    }
    if (strcmp(mode, "r") && strcmp(mode, "rb"))
	retval = -1;
    if (!retval) {
	if (!strcmp(name + 2, "RECORD"))
	    SET_FILE(NH_RECORD, NH_RECORD_AREA);
	else if (!strcmp(name + 2, "HELP"))
	    SET_FILE(NH_HELP, NH_HELP_AREA);
	else if (!strcmp(name + 2, "SHELP"))
	    SET_FILE(NH_SHELP, NH_SHELP_AREA);
	else if (!strcmp(name + 2, "DEBUGHELP"))
	    SET_FILE(NH_DEBUGHELP, NH_DEBUGHELP_AREA);
#if 0
	else if (!strcmp(name + 2, "RUMORFILE"))
	    SET_FILE(NH_RUMORFILE, NH_RUMORAREA);
	else if (!strcmp(name + 2, "ORACLEFILE"))
	    SET_FILE(NH_ORACLEFILE, NH_ORACLEAREA);
#endif
	else if (!strcmp(name + 2, "DATAFILE"))
	    SET_FILE(NH_DATAFILE, NH_DATAAREA);
	else if (!strcmp(name + 2, "CMDHELPFILE"))
	    SET_FILE(NH_CMDHELPFILE, NH_CMDHELPAREA);
	else if (!strcmp(name + 2, "HISTORY"))
	    SET_FILE(NH_HISTORY, NH_HISTORY_AREA);
	else if (!strcmp(name + 2, "LICENSE"))
	    SET_FILE(NH_LICENSE, NH_LICENSE_AREA);
	else if (!strcmp(name + 2, "OPTIONFILE"))
	    SET_FILE(NH_OPTIONFILE, NH_OPTIONAREA);
	else if (!strcmp(name + 2, "OPTIONS_USED"))
	    SET_FILE(NH_OPTIONS_USED, NH_OPTIONS_USED_AREA);
	else if (!strcmp(name + 2, "GUIDEBOOK"))
	    SET_FILE(NH_GUIDEBOOK, NH_GUIDEBOOK_AREA);
	else
	    retval = -1;
    }
    if (!retval)
#ifndef FILE_AREAS
	retval = dlbh_fopen(file, mode);
#else
	retval = dlbh_fopen_area(area, file, mode);
#endif
    nhext_rpc_params(reply, 1, EXT_INT(retval));
    free(name);
    free(mode);
}

#undef SET_FILE

static void
callback_dlbh_fgets(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int fh, len;
    char *retval;
    char buf[BUFSIZ];
    extern char *dlbh_fgets();
    nhext_rpc_params(request, 2, EXT_INT_P(len), EXT_INT_P(fh));
    retval = dlbh_fgets(buf, len < BUFSIZ ? len : BUFSIZ, fh);
    nhext_rpc_params(reply, 1, EXT_STRING(retval ? retval : ""));
}

static void
callback_dlbh_fclose(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int fh, retval;
    nhext_rpc_params(request, 1, EXT_INT_P(fh));
    retval = dlbh_fclose(fh);
    nhext_rpc_params(reply, 1, EXT_INT(retval));
}

static void
callback_flush_screen(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    extern int proxy_curs_on_u;
    flush_screen(proxy_curs_on_u);
}

static void
callback_doredraw(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    (void)doredraw();
}

static void
callback_status_mode(id, request, reply)
unsigned short id;
NhExtXdr *request, *reply;
{
    int mode;
    nhext_rpc_params(request, 1, EXT_INT_P(mode));
    bot_set_handler(mode & 1 ? proxy_status : (void (*)())0L);
}

struct nhext_svc proxy_callbacks[] = {
    EXT_CID_DISPLAY_INVENTORY,		callback_display_inventory,
    EXT_CID_DLBH_FOPEN,			callback_dlbh_fopen,
    EXT_CID_DLBH_FGETS,			callback_dlbh_fgets,
    EXT_CID_DLBH_FCLOSE,		callback_dlbh_fclose,
    EXT_CID_FLUSH_SCREEN,		callback_flush_screen,
    EXT_CID_DOREDRAW,			callback_doredraw,
    EXT_CID_STATUS_MODE,		callback_status_mode,
    0, NULL,
};