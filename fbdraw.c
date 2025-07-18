#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define FBDEVICE "/dev/fb0"

typedef unsigned char ubyte;
struct fb_var_screeninfo vinfo; /* 프레임 버퍼 정보 처리를 위한 구조체 */

unsigned short makepixel(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned short)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}

// unsigned short color
#if 1
//static int drawpoint(int fd, int x, int y, unsigned char r, unsigned char g, unsigned char b)
static int drawpoint(int fd, int x, int y, unsigned short color)
{

    /* 색상 출력을 위한 위치 계산 : offset  = (X의_위치+Y의_위치x해상도의_넓이)x2  */
    int offset = (x + y*vinfo.xres)*2;
    lseek(fd, offset, SEEK_SET);
    write(fd, &color, 2);
    return 0;
}
#else
/* 점을 그린다. */
static void drawpoint(int fd, int x, int y, ubyte r, ubyte g, ubyte b)
{
    ubyte a = 0xFF;

    /* 색상 출력을 위한 위치를 구한다. */
    /* offset = (X의_위치 + Y의_위치 × 해상도의_넓이) × 색상의_바이트_수 */
    int offset = (x + y*vinfo.xres)*vinfo.bits_per_pixel/8.; 
    lseek(fd, offset, SEEK_SET);
    write(fd, &b, 1);
    write(fd, &g, 1);
    write(fd, &r, 1);
    write(fd, &a, 1);
}
#endif
/*
   static void drawline(int fd, int start_x, int end_x, int y,ubyte r, ubyte g, ubyte b) {
   ubyte a=0xFF;
   for(int x=start_x; x<end_x; x++) {
   int offset=(x+y*vinfo.xres)*vinfo.bits_per_pixel/8.;
   lseek(fd, offset, SEEK_SET);
   write(fd, &b, 1);
   write(fd, &g, 1);
   write(fd, &r, 1);
   write(fd, &a, 1);
   }
   }
 */ //32bit
/*
   static void drawfacemmap(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
   ubyte *pfb;
   int bytes_per_pixel=2;
   unsigned short color=makepixel(r,g,b);

   if(end_x==0) end_x=vinfo.xres;
   if(end_y==0) end_y=vinfo.yres;

   size_t screensize=vinfo.xres*vinfo.yres*bytes_per_pixel;
   pfb=(ubyte*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if((int)pfb==-1) {
   perror("mmap failed");
   return;
   }

   for(int y = start_y; y<end_y; y++) {
   for(int x=start_x; x<end_x; x++) {
   int offset=(x+y*vinfo.xres)*bytes_per_pixel;
 *(unsigned short*)(pfb+offset)=color;
 }
 }
 munmap(pfb,screensize);
 }
 */
static void drawfacemmap(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
    int bpp = vinfo.bits_per_pixel;
    if (bpp != 16) {
        fprintf(stderr, "ERROR: Only supports 16bpp RGB565\n");
        return;
    }

    int bytes_per_pixel = 2;
    size_t screensize = vinfo.xres * vinfo.yres * bytes_per_pixel;
    ubyte* pfb = (ubyte*)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (pfb == MAP_FAILED) {
        perror("mmap");
        return;
    }

    unsigned short color = makepixel(r, g, b);

    for (int y = start_y; y < end_y; y++) {
        if (y < 0 || y >= vinfo.yres) continue;
        for (int x = start_x; x < end_x; x++) {
            if (x < 0 || x >= vinfo.xres) continue;
            int offset = (x + y * vinfo.xres) * bytes_per_pixel;
            *(unsigned short*)(pfb + offset) = color;
        }
    }

    munmap(pfb, screensize);
}

/*
   static void drawfacemmap(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
   ubyte *pfb, a=0xFF;
   int color=vinfo.bits_per_pixel/8.;

   if(end_x==0) end_x=vinfo.xres;
   if(end_y==0) end_y=vinfo.yres;

   pfb=(ubyte*)mmap(0, vinfo.xres*vinfo.yres*color, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

   for(int x = start_x; x<end_x*color; x+=color) {
   for(int y=start_y; y<end_y; y++) {
 *(pfb+(x+0) + y*vinfo.xres*color)=b;
 *(pfb+(x+1) + y*vinfo.xres*color)=g;
 *(pfb+(x+2) + y*vinfo.xres*color)=r;
 *(pfb+(x+3) + y*vinfo.xres*color)=a;
 }
 }
 munmap(pfb,vinfo.xres*vinfo.yres*color);
 }
 */
static void drawline(int fd, int start_x, int end_x, int y, ubyte r, ubyte g, ubyte b) {
    unsigned short color=makepixel(r,g,b);
    for(int x=start_x; x<end_x; x++) {
        int offset=(x+y*vinfo.xres)*2;
        lseek(fd, offset, SEEK_SET);
        write(fd, &color, 2);
    }
}

static void drawcircle(int fd, int center_x, int center_y, int radius, ubyte r, ubyte g, ubyte b) {
    int x=radius, y=0;
    int radiusError=1-x;
    unsigned short color = makepixel(r, g, b);
    while(x>=y) {
        drawpoint(fd, x+center_x, y+center_y, color);
        drawpoint(fd, y+center_x, x+center_y, color);
        drawpoint(fd, -x+center_x, y+center_y, color);
        drawpoint(fd, -y+center_x, x+center_y, color);
        drawpoint(fd, -x+center_x, -y+center_y, color);
        drawpoint(fd, -y+center_x, -x+center_y, color);
        drawpoint(fd, x+center_x, -y+center_y, color);
        drawpoint(fd, y+center_x, -x+center_y, color);

        y++;
        if (radiusError<0) {
            radiusError+=2*y+1;
        }else{
            x--;
            radiusError+=2*(y-x+1);
        }
    }
}

static void drawface(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
    ubyte a=0xFF;
    if(end_x==0) end_x = vinfo.xres;
    if(end_y==0) end_y = vinfo.yres;
    for(int x=start_x; x<end_x; x++) {
        for(int y=start_y; y<end_y; y++) {
            int offset=(x+y*vinfo.xres)*vinfo.bits_per_pixel/8.;
            lseek(fd,offset,SEEK_SET);
            write(fd,&b,1);
            write(fd,&g,1);
            write(fd,&r,1);
            write(fd,&a,1);
        }
    }
}

int main(int argc, char **argv)
{
    //int fbfd, status, offset;

    int fbfd = open(FBDEVICE, O_RDWR);
    if (fbfd < 0) {
        perror("Error: cannot open framebuffer device");
        return -1;
    }

    if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Error reading variable information");
        close(fbfd);
        return -1;
    }

    int flag_width = 150;
    int flag_height = 100;
    int center_x = vinfo.xres / 2;
    int center_y = vinfo.yres / 2;

    int start_x = center_x - flag_width / 2;
    int start_y = center_y - flag_height / 2;
    int one_third = flag_width / 3;

    drawfacemmap(fbfd, start_x, start_y, start_x + one_third, start_y + flag_height, 0, 0, 255);        // 파랑
    drawfacemmap(fbfd, start_x + one_third, start_y, start_x + one_third * 2, start_y + flag_height, 255, 255, 255); // 하양
    drawfacemmap(fbfd, start_x + one_third * 2, start_y, start_x + flag_width, start_y + flag_height, 255, 0, 0);    // 빨강

    fbfd = open(FBDEVICE, O_RDWR);  /* 사용할 프레임 버퍼 디바이스를 연다. */
    if(fbfd < 0) {
        perror("Error: cannot open framebuffer device");
        return -1;
    }

    /* 현재 프레임 버퍼에 대한 화면 정보를 얻어온다. */
    if(ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) < 0) {
        perror("Error reading fixed information");
        return -1;
    }
#if 1
    //    drawcircle(fbfd, 200, 200, 100, 255, 0, 255);
    //    drawline(fbfd,0,100,200,0,255,255);

//    drawfacemmap(fbfd,start_x,start_y,start_x+one_third,start_y+flag_height,0,0,255);
//    drawfacemmap(fbfd,start_x+one_third,start_y,start_x+one_third*2,start_y+flag_height,255,255,255);
//    drawfacemmap(fbfd,start_x+one_third*2,start_y,start_x+flag_width,start_y+flag_height,255,0,0);

    //    drawfacemmap(fbfd,0,0,0,0,255,255,0);
    //    drawface(fbfd,0,0,0,0, 255,255,0);

    //    drawpoint(fbfd, 50, 50, makepixel(255, 0, 0));            /*  Red 점을 출력 */
    //    drawpoint(fbfd, 100, 100, makepixel(0, 255, 0));            /*  Green 점을 출력 */
    //    drawpoint(fbfd, 150, 150, makepixel(0, 0, 255));            /*  Blue 점을 출력 */
#else
    //    drawcircle(fbfd, 200, 200, 100, 255, 0, 255);
    //    drawline(fbfd, 0, 100, 200, 0, 255, 255);
    //    drawpoint(fbfd, 50, 50, 255, 0, 0);         /* 빨간색(Red) 점을 출력 */
    //    drawpoint(fbfd, 100, 100, 0, 255, 0);   /* 초록색(Green) 점을 출력 */
    //    drawpoint(fbfd, 150, 150, 0, 0, 255);   /* 파란색(Blue) 점을 출력 */
#endif

    close(fbfd);        /* 사용이 끝난 프레임 버퍼 디바이스를 닫는다. */

    return 0;
}

