#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

void send_file(FILE *fp, char *fname) {
  FILE *target_file = fopen(fname, "r");
  if (!target_file) {
    fprintf(fp, "no such file %s\n", fname);
    return;
  }

  fseek(target_file, 0L, SEEK_END);
  int fsize = ftell(target_file);
  rewind(target_file);
  fwrite(&fsize, 1, sizeof(int), fp);

  unsigned char fbuf[32];
  size_t nr = 0;
  while ((nr = fread(fbuf, 1, sizeof(fbuf), target_file))) {
    fwrite(fbuf, 1, nr, fp);
    fflush(fp);
  }
  fclose(target_file);
}

void receive_file(FILE *sock, char *fname) {
  FILE *fp = fopen(fname, "w");
  if (!fp) {
    return;
  }

  int fsize;
  fread(&fsize, 1, sizeof(int), sock);

  int total = 0;
  while (total < fsize) {
    unsigned char b = fgetc(sock);
    fputc(b, fp);

    total++;
  }
  fclose(fp);
}