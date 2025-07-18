#ifndef __WAVHEADER_H
#define __WAVHEADER_H

typedef struct wavFile_t {
    unsigned char riffID[4];
    unsigned long riffLen;
    unsigned char waveID[4];
    unsigned char fmtID[4];
    unsigned long fmtLen;
    unsigned short fmtTag;
    unsigned short nChannels;
    unsigned long sampleRate;
    unsigned long avgBytesPerSec;
    unsigned short nblockAlign;
    unsigned short bitsPerSample;
    unsigned char dataID[4];
    unsigned long dataLen;
} WAVHEADER;

#endif
