#include <asm-generic/socket.h>
#include <dirent.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <pb_decode.h>
#include <pb_encode.h>

#include "common.h"
#include "message.pb.h"

typedef struct MessageChunk {
  char buffer[BUFFER_SIZE];
} MessageChunk;

void evaluate_cmd(const char *arg);
void *listen_input(void *arg);
void *write_output(void *arg);

int main() {

  int sockfd;
  struct sockaddr_in server_addr;

  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  server_addr.sin_port = htons(PORT);

  if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) !=
      0) {

    perror("Failed to connect");
    return 1;
  }

  pthread_t thread_read, thread_write;
  pthread_create(&thread_read, NULL, listen_input, &sockfd);
  pthread_create(&thread_write, NULL, write_output, &sockfd);

  pthread_join(thread_read, NULL);
  pthread_join(thread_write, NULL);

  close(sockfd);

  return 0;
}

void *listen_input(void *arg) {

  int sockfd = *(int *)(arg);
  pb_ostream_t output = pb_ostream_from_socket(sockfd);

  while (1) {

    MessageChunk chunk;
    memset(chunk.buffer, 0, BUFFER_SIZE);

    int byte_read = read(STDIN_FILENO, chunk.buffer, BUFFER_SIZE - 1);

    evaluate_cmd((const char *)(chunk.buffer));
    // ChatMessage chat = {};
    // strcpy(chat.chat, chunk.buffer);
    //
    // if (!pb_encode_delimited(&output, ChatMessage_fields, &chat)) {
    //   fprintf(stderr, "Encoding failed: %s\n", PB_GET_ERROR(&output));
    // }
  }

  return NULL;
}

void *write_output(void *arg) {

  int sockfd = *(int *)(arg);
  char buffer[BUFFER_SIZE];

  while (1) {
    bzero(buffer, BUFFER_SIZE);
    int byte = read(sockfd, buffer, BUFFER_SIZE - 1);
    if (byte < 0) {
      break;
    }
    printf("%s", buffer);
  }

  return NULL;
}

void evaluate_cmd(const char *buffer) {
  int argc = 0;

  for (int i = 0; buffer[i] != '\0'; i++) {
    if (buffer[i] == ' ') {
      argc += 1;
    }
  }

  char *argv[argc + 1];
  int index = 0;
  int last_index = 0;
  for (int i = 0; buffer[i] != '\0'; i++) {
    // if (buffer[i] == ' ') {
    //   argv[index] = malloc(i - last_index + 1);
    //   memcpy(argv[i], buffer + last_index, (size_t)(i - last_index + 1));
    //   index += 1;
    //   last_index += i;
    // }
    if (buffer[i] == ' ') {
      argv[0] = (char *)malloc(i - last_index + 1);
      printf("The size is %d", i - last_index + 1);
      strncat(argv[0], buffer + last_index, i - last_index + 1);
      // memcpy(argv[i], buffer + last_index, (size_t)(i - last_index + 1));
      // index += 1;
      // last_index += i;
      break;
    }
  }

  // for (int i = 0; i <= argc; i++) {
  printf("The argument at index %d is %s\n", 0, argv[0]);
  // }
}
