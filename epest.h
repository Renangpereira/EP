#ifndef EPEST_H
#define EPEST_H

#define FUSE_USE_VERSION 30

/* Cabeçalho do sistema de arquivos */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <fuse.h>
#include <dirent.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <libgen.h>
#include <fcntl.h>
#include <math.h>
#include <sys/types.h>
#include <openssl/md5.h>
#include <openssl/sha.h>

/* Preprocessar Macros */

#define _GNU_SOURCE
#define MAX_FILENAME 64
#define MAX_PATH 512
#define MINIMUM_PNG 2
#define HELP_OPTION "-help"
#define DEBUG_OPTION "-debug"
#define FORMAT_OPTION "-format"
#define EXPAND_OPTION "-expand"
#define MNT_OPTION "-mount"


extern int8_t ep_dbg;
int8_t ep_dbg;


/* Prototipos de funções */

//epest.c
void ep_usage();
int parseArgv(int argc, char *argv[], char *option);
int getMD5(char *filename, char *mds_sum);
//ep.c
int wipeFile(char *path);
int findFile(char *path);
int createFile(char *path);
void ep_expand();
void ep_format();
void writeRoot();
void readRoot();
void saveState();

#endif
