#ifndef CMD_H
#define CMD_H

#include <stdio.h>

#define CMD_BUF_LEN 256

struct cmd {
  char *name;
  void (*handler)(FILE *fp, char *arg);
};

extern struct cmd commands[9];
extern int ncmds;

int dispatch_command(FILE *fp, char *cmd_buf, size_t len);
#define strip_newline(str, len) ((str)[(len)-1] = '\0')

#endif