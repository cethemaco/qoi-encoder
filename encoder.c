#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include <string.h>

#define ERROR_CODE 12 //Posix Error Code for insufficient memory

// Qoi-Tags
#define QOI_OP_RUN 3 << 6
#define QOI_OP_DIFF 1 << 6
#define QOI_OP_LUMA 1 << 7
#define QOI_OP_RGB 254
#define QOI_OP_RGBA 255

// Shortcuts
#define EQUALPIXEL(a, b) ((a[0] == b[0]) && (a[1] == b[1]) && (a[2] == b[2]) && (a[3] == b[3]))
#define SWAP(num) (((num>>24)&0xff) | ((num<<8)&0xff0000) | ((num<<24)&0xff000000))
#define INDEX(a) ((a[0] * 3 + a[1] * 5 + a[2] * 7 + a[3] * 11) % 64)

void* s_calloc(size_t amount, size_t size, char origin[]) {
	void* new = calloc(amount, size);
	if (new == NULL) {
		fprintf(stderr, "%s(): Insufficient memory", origin);
		exit(ERROR_CODE);
	}

	return new;
}

typedef struct ImgInfo {
	uint32_t width, height;
	uint8_t channels, colorspace;
} img_i;

typedef struct QoiChunk {
	uint8_t* data;
	int byteLength;
} qoi_c;

int wrapSubtract(int a, int b) {
	// For the EdgeCases (0 - x) and (x - 0)
	if (a == 0) { a = 256; }
	if (b == 0) { b = 256; }

	return (a - b) % 256;
}

img_i* getImgMetadata() {
	// Parsing the amount of channels and that sRGB thing
	uint8_t colorspace = getchar();
	uint8_t channels = getchar();

	// Getting width & height:
	int width, height;

	fread(&width, 4, 1, stdin);
	fread(&height, 4, 1, stdin);	
	
	img_i* info = s_calloc(1, sizeof(img_i), "getImgMetadata");
	
	info->width = width;
	info->height = height;
	info->channels = channels;
	info->colorspace = colorspace; 
	return info; 
}

qoi_c* getQoiChunk(int* prev, int* curr, int** indices, int length){
	// First case: There has been a run of pixels before 
	if (length > 0) {
		// Checking if length needs to be split up:
		int remainder = length % 62;
		int lengthChunks = (length / 62) + (remainder > 0);
		
		uint8_t* qoi_run = s_calloc(lengthChunks, sizeof(uint8_t), "getQoiChunk");

		for (int i = 1; i <= lengthChunks; i++) {
			qoi_run[i - 1] = (uint8_t) (QOI_OP_RUN | 61); // saved with a bias of -1
		}

		if (remainder > 0) {
			qoi_run[lengthChunks - 1] = (uint8_t) (QOI_OP_RUN | (remainder - 1));
		}

		qoi_c* qoi_chunk = s_calloc(1, sizeof(qoi_c), "getQoiChunk");

		qoi_chunk->data = qoi_run;
		qoi_chunk->byteLength = lengthChunks;
		return qoi_chunk;
	}

	// Second case: Pixel was already seen
	int indexCurr = INDEX(curr);
	int* indexed = indices[indexCurr];	
	if (indexed == NULL) { 
		indexed = s_calloc(4, sizeof(uint8_t), "getQoiChunk"); 
	}

	if (EQUALPIXEL(indexed, curr)) {
		uint8_t* qoi_index = calloc(1, sizeof(uint8_t));
		qoi_index[0] = (uint8_t) indexCurr;

		qoi_c* qoi_chunk = s_calloc(1, sizeof(qoi_c), "getQoiChunk");

		qoi_chunk->data = qoi_index;
		qoi_chunk->byteLength = 1;
		return qoi_chunk;
	} 

	// Third case: Differences between Pixels is small enough
	if (curr[3] == prev[3]) { 	
		int dr = wrapSubtract(curr[0], prev[0]);
		int dg = wrapSubtract(curr[1], prev[1]);
		int db = wrapSubtract(curr[2], prev[2]);
	
		if ((-2 <= dr && dr <= 1) && (-2 <= dg && dg <= 1) && (-2 <= db && db <= 1)) {
			uint8_t* qoi_diff = s_calloc(1, sizeof(uint8_t), "getQoiChunk");	
			qoi_diff[0] = (uint8_t) QOI_OP_DIFF | (dr + 2) << 4 | (dg + 2) << 2 | (db + 2);

			qoi_c* qoi_chunk = s_calloc(1, sizeof(qoi_c), "getQoiChunk");
			qoi_chunk->data = qoi_diff;
			qoi_chunk->byteLength = 1;
			return qoi_chunk;
		} else if ((-32 <= dg && dg <= 31)) {
			int drdg = wrapSubtract(dr, dg);
			int dbdg = wrapSubtract(db, dg);

			if ((-8 <= drdg && drdg <= 7) && (-8 <= dbdg && dbdg <= 7)) {
				uint8_t* qoi_luma = calloc(2, sizeof(uint8_t));
				qoi_luma[0] = (uint8_t) QOI_OP_LUMA | (dg + 32);
				qoi_luma[1] = (uint8_t) (drdg + 8) << 4 | (dbdg + 8);

				qoi_c* qoi_chunk = s_calloc(1, sizeof(qoi_c), "getQoiChunk");

				qoi_chunk->data = qoi_luma;
				qoi_chunk->byteLength = 2;
				return qoi_chunk;
			}
		}
	}

	// Fourth case: Color values
	uint8_t alpha = prev[3] != curr[3];
	int byteLength = (alpha ? 5 : 4);
	uint8_t* qoi_rgb = s_calloc(byteLength, sizeof(uint8_t), "getQoiChunk");
	qoi_rgb[0] = (uint8_t) (alpha ? QOI_OP_RGBA : QOI_OP_RGB);
	qoi_rgb[1] = (uint8_t) curr[0];	
	qoi_rgb[2] = (uint8_t) curr[1];	
	qoi_rgb[3] = (uint8_t) curr[2];
	if (alpha) { qoi_rgb[4] = curr[3]; }

	qoi_c* qoi_chunk = s_calloc(1, sizeof(qoi_c), "getQoiChunk");

	qoi_chunk->data = qoi_rgb;
	qoi_chunk->byteLength = byteLength;
	return qoi_chunk;
}

void writeHeader(img_i* metadata) {
	//File Header:
	printf("qoif");
	uint32_t le_width = SWAP(metadata->width);
	uint32_t le_height = SWAP(metadata->height);
	fwrite(&le_width, 1, 4, stdout);
	fwrite(&le_height, 1, 4, stdout);
	putchar(metadata->channels);
	putchar(metadata->colorspace);
}

void writeQoiChunk(qoi_c* chunk) {
	for (int i = 0; i < chunk->byteLength; i++) {
		putchar(chunk->data[i]);
	}
	free(chunk);
}

void encode(int channels) {
	// Array of already seen pixels
	int* indices[64] = {NULL};
	
	int prev[4] = {0, 0, 0, 255};
	int curr[4] = {0, 0, 0, 255};
	int length = 0;

	int bytesRead = 0;
	int pixelsProcessed = 0;
	int currentByte;

	while (currentByte != EOF) {
		if (bytesRead != channels) {
			currentByte = getchar();
			curr[bytesRead] = currentByte;
			bytesRead++;
		} else {
			// Run-Length check
			if (EQUALPIXEL(curr, prev)) {
				length++; pixelsProcessed++;
			} else {
				// Determining & writing the next QOI_OP
				qoi_c* qoi_chunk = getQoiChunk(prev, curr, indices, length);	
				writeQoiChunk(qoi_chunk);
				if (length > 0) {
					length = 0;
					qoi_chunk = getQoiChunk(prev, curr, indices, length);
					writeQoiChunk(qoi_chunk);
				}

				// Filling the index of the Last seen pixels
				int indexCurr = INDEX(curr); 
				if (indices[indexCurr] != NULL) { free(indices[indexCurr]); }
				indices[indexCurr] = s_calloc(4, sizeof(uint8_t), "encode");
				memcpy(indices[indexCurr], curr, sizeof(curr));
				memcpy(prev, curr, sizeof(curr));

				pixelsProcessed++;	
			} 
			bytesRead = 0;
		}	
	}

	// Encoder reached the end while encoding a QOI_RUN
	if (length > 0) {
		qoi_c* qoi_chunk = getQoiChunk(prev, curr, indices, length);
		writeQoiChunk(qoi_chunk);
	}
}

void writeEndMarker() {
	for (int i = 0; i < 7; i++) {
		putchar(0x00);
	}
	putchar(0x01);
}


int main() {
	img_i* meta = getImgMetadata();
	writeHeader(meta);
	encode(meta->channels);
	writeEndMarker();
	return 0;
}
