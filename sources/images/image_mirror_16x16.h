#include <ximage.h>

static uint8_t mirror_16x16_data[] =
{
    0x35, 0x35, 0x35, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x2B, 0x2B, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x23, 0x23, 0x22, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0x1A, 0x1A, 0x1A, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x12, 0x13, 0x12, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0x0B, 0x0B, 0x0B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x06, 0x05, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x30, 0x30, 0x31, 0xFF, 0x2C, 0x2B, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 0x30, 0x30, 0x30, 0xFF, 0x2B, 0x2B, 0x2C, 0xFF, 
    0x27, 0x26, 0x27, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x16, 0x16, 0x16, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x2C, 0x2B, 0x2B, 0xFF, 0x27, 0x27, 0x27, 0xFF, 0x22, 0x22, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x27, 0x27, 0x27, 0xFF, 0x23, 0x22, 0x23, 0xFF, 0x1E, 0x1E, 0x1E, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0x16, 0x16, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x22, 0x22, 0x23, 0xFF, 
    0x1E, 0x1E, 0x1E, 0xFF, 0x1A, 0x19, 0x1A, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0x1E, 0x1E, 0x1E, 0xFF, 0x1A, 0x1A, 0x19, 0xFF, 0x16, 0x16, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x2C, 0x2C, 0x2C, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0x23, 0x23, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x1A, 0x1A, 0x1A, 0xFF, 0x16, 0x16, 0x16, 0xFF, 
    0x12, 0x12, 0x12, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0B, 0x0B, 0x0C, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x05, 0x05, 0x05, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x15, 0x16, 0x16, 0xFF, 0x13, 0x12, 0x12, 0xFF, 0x0E, 0x0F, 0x0F, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x12, 0x12, 0x13, 0xFF, 0x0F, 0x0F, 0x0E, 0xFF, 0x0C, 0x0B, 0x0B, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x35, 0x35, 0x35, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x16, 0x16, 0x16, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0F, 0x0F, 0x0F, 0xFF, 
    0x0B, 0x0B, 0x0C, 0xFF, 0x08, 0x08, 0x08, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0x0B, 0x0B, 0x0B, 0xFF, 0x08, 0x08, 0x08, 0xFF, 0x06, 0x05, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x16, 0x16, 0x16, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x08, 0x08, 0x08, 0xFF, 0x05, 0x05, 0x06, 0xFF, 
    0x02, 0x02, 0x03, 0xFF, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0x05, 0x05, 0x05, 0xFF, 0x02, 0x02, 0x02, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0x35, 0x35, 0x35, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0x2B, 0x2B, 0x2B, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x23, 0x23, 0x22, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0x1A, 0x1A, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x12, 0x12, 0x12, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x0B, 0x0B, 0x0B, 0xFF, 
    0xFF, 0xFF, 0xFF, 0x00, 0x05, 0x05, 0x05, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 0xFF, 0xFF, 0xFF, 0x00, 
    0xFF, 0xFF, 0xFF, 0x00, 
};

static const ximage image_mirror_16x16 =
{
    &mirror_16x16_data[0],
    16, 16,
    64,
    XPixelFormatRGBA32
};