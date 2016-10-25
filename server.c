#include <netdb.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cmd.h"

char *done_msg = "done";

int handle_connection(int sock) {
  char cwd[1024];
  char cmd_buf[CMD_BUF_LEN];
  int n = 0;

  FILE *fp = fdopen(sock, "r+");

  for (;;) {
    lseek(sock, 0, SEEK_END);
    n = read(sock, cmd_buf, CMD_BUF_LEN);
    puts(cmd_buf);

    if (n == 0) {
      break;
    }
    strip_newline(cmd_buf, n);
    if (dispatch_command(fp, cmd_buf, CMD_BUF_LEN) == -1) {
      break;
    }
    putc(EOF, fp);
    fflush(fp);
  }

  fclose(fp);
  return 0;
}

int main(int argc, char **argv) {
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in server_addr = {.sin_family = AF_INET,
                                    .sin_addr.s_addr = htonl(INADDR_ANY),
                                    .sin_port = 0};

  if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1) {
    perror("bind");
    return 1;
  }

  struct sockaddr_in name_addr;
  socklen_t addr_len = sizeof(struct sockaddr_in);

  if (getsockname(sock, (struct sockaddr *)&name_addr, &addr_len) == -1) {
    perror("getsockname");
    return 1;
  }

  unsigned short port = ntohs(name_addr.sin_port);
  printf("server port: %d\n", port);

  listen(sock, 5);

  struct sockaddr_in client_addr;
  for (;;) {
    int client = accept(sock, (struct sockaddr *)&client_addr, &addr_len);
    if (client == -1) {
      perror("accept");
      continue;
    }
    if (handle_connection(client) == -1) {
      puts("client closed connection");
    }
  }

  close(sock);
  return 0;
}