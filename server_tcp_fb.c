#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define FRAMEBUFFER_DEVICE  "/dev/fb0"
#define SERVER_PORT         9600
#define SERVER_IP           "127.0.0.1"
#define WIDTH               640
#define HEIGHT              480
#define BUFFER_SIZE         (WIDTH * HEIGHT * 2)  // YUYV 포맷은 픽셀당 2바이트

static struct fb_var_screeninfo vinfo;

void display_frame(uint16_t *fbp, uint8_t *data, int width, int height) 
{
    int x_offset = (vinfo.xres - width) / 2;
    int y_offset = (vinfo.yres - height) / 2;

    // YUYV -> RGB565 변환하여 프레임버퍼에 출력
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; x += 2) {
            uint8_t Y1 = data[(y * width + x) * 2];
            uint8_t U = data[(y * width + x) * 2 + 1];
            uint8_t Y2 = data[(y * width + x + 1) * 2];
            uint8_t V = data[(y * width + x + 1) * 2 + 1];

            int R1 = Y1 + 1.402 * (V - 128);
            int G1 = Y1 - 0.344136 * (U - 128) - 0.714136 * (V - 128);
            int B1 = Y1 + 1.772 * (U - 128);

            int R2 = Y2 + 1.402 * (V - 128);
            int G2 = Y2 - 0.344136 * (U - 128) - 0.714136 * (V - 128);
            int B2 = Y2 + 1.772 * (U - 128);

            // RGB565 포맷으로 변환 (R: 5비트, G: 6비트, B: 5비트)
            uint16_t pixel1 = ((R1 & 0xF8) << 8) | ((G1 & 0xFC) << 3) | (B1 >> 3);
            uint16_t pixel2 = ((R2 & 0xF8) << 8) | ((G2 & 0xFC) << 3) | (B2 >> 3);

            fbp[(y + y_offset) * vinfo.xres + (x + x_offset)] = pixel1;
            fbp[(y + y_offset) * vinfo.xres + (x + x_offset + 1)] = pixel2;
        }
    }
}

int main() 
{
    // 프레임버퍼 열기
    int fb_fd = open(FRAMEBUFFER_DEVICE, O_RDWR);
    if (fb_fd == -1) {
        perror("Error opening framebuffer device");
        exit(1);
    }

    if (ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo)) {
        perror("Error reading variable information");
        close(fb_fd);
        exit(1);
    }

    uint32_t screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;
    uint16_t *fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, 0);
    if ((intptr_t)fbp == -1) {
        perror("Error mapping framebuffer device to memory");
        close(fb_fd);
        exit(1);
    }

    // 소켓 생성
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket creation failed");
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    // SO_REUSEADDR 옵션 설정 (서버 재시작 시 "Address already in use" 오류 방지)
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(server_fd);
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    // 서버 주소 설정
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // 소켓에 주소 바인딩
    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_fd);
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    // 연결 대기 상태로 전환
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        close(server_fd);
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    printf("Server started at %s:%d\n", SERVER_IP, SERVER_PORT);
    printf("Waiting for client connection...\n");

    // 클라이언트 연결 수락
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
        perror("Accept failed");
        close(server_fd);
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    printf("Client connected: %s\n", inet_ntoa(client_addr.sin_addr));

    // 비디오 데이터 수신 버퍼
    uint8_t *buffer = malloc(BUFFER_SIZE);
    if (!buffer) {
        perror("Failed to allocate buffer");
        close(client_fd);
        close(server_fd);
        munmap(fbp, screensize);
        close(fb_fd);
        exit(1);
    }

    // 클라이언트로부터 데이터 수신 및 화면에 표시
    while (1) {
        int bytes_received = 0;
        int total_received = 0;

        // 한 프레임 전체를 수신할 때까지 반복
        while (total_received < BUFFER_SIZE) {
            bytes_received = recv(client_fd, buffer + total_received, BUFFER_SIZE - total_received, 0);

            if (bytes_received <= 0) {
                if (bytes_received == 0) {
                    printf("Client disconnected\n");
                    break;
                } else {
                    perror("Receive failed");
                    break;
                }
            }

            total_received += bytes_received;
            printf("Received %d/%d bytes\n", total_received, BUFFER_SIZE);
        }

        // 프레임 수신 완료, 화면에 표시
        if (total_received == BUFFER_SIZE) {
            display_frame(fbp, buffer, WIDTH, HEIGHT);
        } else {
            // 연결이 끊기거나 오류가 발생한 경우
            break;
        }
    }

    // 리소스 정리
    free(buffer);
    close(client_fd);
    close(server_fd);
    munmap(fbp, screensize);
    close(fb_fd);

    return 0;
}
