#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stropts.h>
#include <sys/socket.h>
#include <unistd.h>
#include "cmd.h"
#include "file_transfer.h"

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("usage: %s [server] [port]", argv[0]);
    return 1;
  }

  struct hostent *he = gethostbyname(argv[1]);
  if (!he) {
    printf("unknown host %s\n", argv[1]);
    return 1;
  }

  int server_ip = *(long *)he->h_addr;
  int server_port = atoi(argv[2]);

reset:;
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock == -1) {
    perror("socket");
    return 1;
  }

  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = server_ip;
  server_addr.sin_port = htons(server_port);

  int res = connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
  if (res == -1) {
    perror("connect");
    return 1;
  }

  FILE *fp = fdopen(sock, "r+");

  int n = 0;
  char eof = 0;
  char line[CMD_BUF_LEN];
  char ans[CMD_BUF_LEN];
  for (;;) {
    fflush(fp);
    printf("> ");
    fgets(line, CMD_BUF_LEN, stdin);
    size_t len = strlen(line);

    strip_newline(line, len);

    if (line[0] == 'l' && line[1] != 's') {
      dispatch_command(stdout, line + 1, len - 1);
      fflush(stdout);
      line[0] = '\0';
      len = 1;
      continue;
    }

    fwrite(line, 1, CMD_BUF_LEN, fp);
    fflush(fp);
    // write(sock, line, len);
    if (strncmp(line, "quit", 4) == 0) {
      break;
    }

    if (strncmp(line, "get", 3) == 0) {
      strip_newline(line, len);
      receive_file(fp, line + 4);

      fclose(fp);
      goto reset;
    } else if (strncmp(line, "put", 3) == 0) {
      strip_newline(line, len);
      send_file(fp, line + 4);
    }
    do {
      // n = fread(ans, 1, CMD_BUF_LEN, fp);
      n = read(sock, ans, CMD_BUF_LEN);
      ans[n] = 0;
      eof = ans[n - 1];
      ans[n - 1] = 0;
      printf("%s", ans);
    } while (eof != EOF);
  }

  fclose(fp);
  // close(sock);
  return 0;
}