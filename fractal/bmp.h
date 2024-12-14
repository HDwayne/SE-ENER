// https://stackoverflow.com/questions/2654480/writing-bmp-image-in-pure-c-c-without-other-libraries

// BMP constants
#define BYTES_PER_PIXEL 3
#define FILE_HEADER_SIZE 14
#define INFO_HEADER_SIZE 40

void writeBMP(const char *filename, unsigned char *pixels, int width, int height);
static unsigned char* createBitmapFileHeader(int height, int stride);
static unsigned char* createBitmapInfoHeader(int height, int width);