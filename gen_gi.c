#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <sys/mman.h>

#define FBDEVICE "/dev/fb0"

typedef unsigned char ubyte;
struct fb_var_screeninfo vinfo;

unsigned short makepixel(unsigned char r, unsigned char g, unsigned char b) {
    return (unsigned short)(((r>>3)<<11)|((g>>2)<<5)|(b>>3));
}

static void drawfacemmap(int fd, int start_x, int start_y, int end_x, int end_y, ubyte r, ubyte g, ubyte b) {
    int bpp=vinfo.bits_per_pixel;

    int scratch=bpp/8.;
    size_t framesize=vinfo.xres*vinfo.yres*scratch;
    ubyte* pfb=(ubyte*)mmap(0,framesize,PROT_READ | PROT_WRITE, MAP_SHARED,fd,0);

    unsigned short color = makepixel(r,g,b);

    for
}
