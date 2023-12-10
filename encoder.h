#pragma once

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ERROR_CODE 12  // Posix Error Code for low memory

// Qoi-tags
#define QOI_OP_RUN 3 << 6
#define QOI_OP_DIFF 1 << 6
#define QOI_OP_LUMA 1 << 7
#define QOI_OP_RGB 254
#define QOI_OP_RGBA 255

// Shortcuts
#define EQUALPIXEL(a, b) ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]))
#define SWAP(num) (((num >> 24) & 0xff) | ((num << 8) & 0xff0000) | ((num << 24) & 0xff000000))
#define INDEX(a) ((a[0] * 3 + a[1] * 5 + a[2] * 7 + a[3] * 11) % 64)

// Qoi data structures
typedef struct ImgInfo {
    uint32_t width, height;
    uint8_t channels, colorspace;
} img_i;

typedef struct QoiChunk {
    uint8_t *data;
    int byteLength;
} qoi_c;

// Functions
img_i *getImgMetadata();
void writeHeader(img_i *metadata);
void encode(int channels);
void writeEndMarker();
