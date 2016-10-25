#include "cmd.h"
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
#include "file_transfer.h"

void mkdir_handler(FILE *fp, char *arg) {
  if (mkdir(arg, 0777) == -1) {
    fprintf(fp, "Error: ");
    if (errno == EEXIST) {
      fprintf(fp, "dir already exists");
    } else {
      fprintf(fp, "could not create dir");
    }
  } else {
    fprintf(fp, "Created dir %s", arg);
  }
  putc('\n', fp);
}
void rmdir_handler(FILE *fp, char *arg) {
  if (rmdir(arg) == -1) {
    fprintf(fp, "Error: could not remove dir");
  } else {
    fprintf(fp, "Removed dir %s", arg);
  }
  putc('\n', fp);
}
void rm_handler(FILE *fp, char *arg) {
  if (unlink(arg) == -1) {
    fprintf(fp, "Error: Could not remove file");
  } else {
    fprintf(fp, "Removed %s", arg);
  }
  putc('\n', fp);
}
void cat_handler(FILE *fp, char *arg) {
  FILE *fsrc;
  int n = 0;
  char buf[1024];

  fsrc = fopen(arg, "r");
  if (!fsrc) return;

  while ((n = fread(buf, 1, 1024, fsrc))) {
    fwrite(buf, 1, n, fp);
  }
  putc('\n', fp);
  fclose(fsrc);
}

char *t1 = "xwrxwrxwr-------";
char *t2 = "----------------";

void ls_file(FILE *fp, char *fname) {
  struct stat fstat, *sp;
  int r, i;
  char ftime[64];
  char linkname[256];
  int linklen;

  sp = &fstat;

  if ((r = lstat(fname, &fstat)) < 0) {
    fprintf(fp, "can't stat %s\n", fname);
    exit(1);
  }

  if ((sp->st_mode & 0xF000) == 0x8000) fprintf(fp, "%c", '-');
  if ((sp->st_mode & 0xF000) == 0x4000) fprintf(fp, "%c", 'd');
  if ((sp->st_mode & 0xF000) == 0xA000) fprintf(fp, "%c", 'l');

  for (i = 8; i >= 0; i--) {
    if (sp->st_mode & (1 << i))
      fprintf(fp, "%c", t1[i]);
    else
      fprintf(fp, "%c", t2[i]);
  }

  fprintf(fp, "%4lu ", sp->st_nlink);
  fprintf(fp, "%4u ", sp->st_gid);
  fprintf(fp, "%4u ", sp->st_uid);
  fprintf(fp, "%8lu ", sp->st_size);

  strcpy(ftime, ctime(&sp->st_ctime));
  ftime[strlen(ftime) - 1] = 0;
  fprintf(fp, "%s  ", ftime);

  fprintf(fp, "%s", basename(fname));

  if ((sp->st_mode & 0xF000) == 0xA000) {
    linklen = readlink(fname, linkname, 256);
    linkname[linklen] = 0;
    fprintf(fp, " -> %s", linkname);
  }
  fprintf(fp, "\n");
}

void ls_dir(FILE *fp, char *dname) {
  char path_buf[256];
  DIR *dir = opendir(dname);
  if (!dir) {
    fprintf(fp, "Could not read directory...");
    return;
  }

  struct dirent *de;
  while ((de = readdir(dir))) {
    sprintf(path_buf, "%s/%s", dname, de->d_name);
    ls_file(fp, path_buf);
  }

  closedir(dir);
}

void ls_handler(FILE *fp, char *arg) {
  struct stat mystat, *sp;
  int r;
  char *s;
  char name[1024], cwd[1024];

  s = arg;
  if (!arg) s = "./";

  sp = &mystat;
  if ((r = lstat(s, sp) < 0)) {
    ls_dir(fp, ".");
  }
  strcpy(name, s);
  if (s[0] != '/') {
    getcwd(cwd, 1024);
    strcpy(name, cwd);
    strcat(name, "/");
    strcat(name, s);
  }
  if (S_ISDIR(sp->st_mode))
    ls_dir(fp, name);
  else
    ls_file(fp, name);
}

void get_handler(FILE *fp, char *arg) { send_file(fp, arg); }
void put_handler(FILE *fp, char *arg) { receive_file(fp, arg); }

void cd_handler(FILE *fp, char *arg) { chdir(arg); }
void pwd_handler(FILE *fp, char *arg) {
  char cwd[1024];
  getcwd(cwd, 1024);
  fprintf(fp, "%s\n", cwd);
}

struct cmd commands[9] = {
    {"mkdir", mkdir_handler}, {"rmdir", rmdir_handler}, {"rm", rm_handler},
    {"cat", cat_handler},     {"ls", ls_handler},       {"get", get_handler},
    {"put", put_handler},     {"cd", cd_handler},       {"pwd", pwd_handler}};
int ncmds = sizeof(commands) / sizeof(struct cmd);

int dispatch_command(FILE *fp, char *cmd_buf, size_t len) {
  char *cmd = strtok(cmd_buf, " ");
  char *arg = strtok(NULL, " ");

  if (cmd == NULL) return 0;

  int i;
  for (i = 0; i < ncmds; i++) {
    if (strcmp(cmd, commands[i].name) == 0) {
      commands[i].handler(fp, arg);
      break;
    }
  }
  if (strcmp(cmd, "quit") == 0) {
    return -1;
  }
  if (i == ncmds) {
    fprintf(fp, "Invalid command '%s'\n", cmd);
  }
  return 0;
}
