#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define VIDEO_DEVICE    "/dev/video0"
#define WIDTH           640
#define HEIGHT          480
#define SERVER_IP       "127.0.0.1"  // 서버 IP 주소
#define SERVER_PORT     9600         // 서버 포트

int main() 
{
  int fd = open(VIDEO_DEVICE, O_RDWR);
  if (fd == -1) {
    perror("Failed to open video device");
    return 1;
  }

  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width = WIDTH;
  fmt.fmt.pix.height = HEIGHT;
  fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
  fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

  if (ioctl(fd, VIDIOC_S_FMT, &fmt) == -1) {
    perror("Failed to set format");
    close(fd);
    return 1;
  }

  // 버퍼 할당
  char *buffer = malloc(fmt.fmt.pix.sizeimage);
  if (!buffer) {
    perror("Failed to allocate buffer");
    close(fd);
    return 1;
  }

  // 소켓 생성
  int sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    free(buffer);
    close(fd);
    return 1;
  }

  // 서버 주소 설정
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(SERVER_PORT);
  
  if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
    perror("Invalid address");
    free(buffer);
    close(fd);
    close(sock);
    return 1;
  }

  // 서버에 연결
  if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
    perror("Connection failed");
    free(buffer);
    close(fd);
    close(sock);
    return 1;
  }

  printf("Connected to server. Starting video streaming...\n");

  while (1) {
    // 카메라에서 프레임 읽기
    int ret = read(fd, buffer, fmt.fmt.pix.sizeimage);
    if (ret == -1) {
      perror("Failed to read frame");
      break;
    }

    // 프레임 데이터 전송
    if (send(sock, buffer, ret, 0) < 0) {
      perror("Failed to send frame");
      break;
    }
    
    printf("Sent frame: %d bytes\n", ret);
    usleep(33333);  // ~30fps (약 33ms 간격)
  }

  free(buffer);
  close(fd);
  close(sock);
  return 0;
}
