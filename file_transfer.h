#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include <stdio.h>

void send_file(FILE *fp, char *fname);
void receive_file(FILE *sock, char *fname);

#endif