#include <ximage.h>

static uint8_t uniform_additive_noise_16x16_data[] =
{
    0xD9, 0xD9, 0xD9, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xC5, 0xC5, 0xC5, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 0x24, 0x24, 0x24, 0xFF, 0x24, 0x24, 0x24, 0xFF, 0x1C, 0x1C, 0x1C, 0xFF, 
    0x6F, 0x6F, 0x6F, 0xFF, 0x60, 0x60, 0x60, 0xFF, 0x24, 0x24, 0x24, 0xFF, 0x70, 0x70, 0x70, 0xFF, 0xAA, 0xAA, 0xAA, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x59, 0x59, 0x59, 0xFF, 0x3E, 0x3E, 0x3E, 0xFF, 0xCE, 0xCE, 0xCE, 0xFF, 0x90, 0x90, 0x90, 0xFF, 
    0x5D, 0x5D, 0x5D, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xC6, 0xC6, 0xC6, 0xFF, 0x85, 0x85, 0x85, 0xFF, 
    0xCB, 0xCB, 0xCB, 0xFF, 0xBF, 0xBF, 0xBF, 0xFF, 0xCB, 0xCB, 0xCB, 0xFF, 0x6E, 0x6E, 0x6E, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x64, 0x64, 0x64, 0xFF, 0x24, 0x24, 0x24, 0xFF, 0xD8, 0xD8, 0xD8, 0xFF, 0x86, 0x86, 0x86, 0xFF, 0xDD, 0xDD, 0xDD, 0xFF, 
    0x3D, 0x3D, 0x3D, 0xFF, 0xCE, 0xCE, 0xCE, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x95, 0x95, 0x95, 0xFF, 
    0x4C, 0x4C, 0x4C, 0xFF, 0x4D, 0x4D, 0x4D, 0xFF, 0xC8, 0xC8, 0xC8, 0xFF, 0x88, 0x88, 0x88, 0xFF, 0x86, 0x86, 0x86, 0xFF, 
    0xD2, 0xD2, 0xD2, 0xFF, 0xC8, 0xC8, 0xC8, 0xFF, 0x75, 0x75, 0x75, 0xFF, 0xB3, 0xB3, 0xB3, 0xFF, 0xDE, 0xDE, 0xDE, 0xFF, 
    0x34, 0x34, 0x34, 0xFF, 0x37, 0x37, 0x37, 0xFF, 0xDA, 0xDA, 0xDA, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xD4, 0xD4, 0xD4, 0xFF, 0x83, 0x83, 0x83, 0xFF, 0xB9, 0xB9, 0xB9, 0xFF, 0x50, 0x50, 0x50, 0xFF, 0xB3, 0xB3, 0xB3, 0xFF, 
    0x97, 0x97, 0x97, 0xFF, 0x43, 0x43, 0x43, 0xFF, 0xC0, 0xC0, 0xC0, 0xFF, 0xAF, 0xAF, 0xAF, 0xFF, 0x21, 0x21, 0x21, 0xFF, 
    0x96, 0x96, 0x96, 0xFF, 0xC8, 0xC8, 0xC8, 0xFF, 0x72, 0x72, 0x72, 0xFF, 0x46, 0x46, 0x46, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0x35, 0x35, 0x35, 0xFF, 0x67, 0x67, 0x67, 0xFF, 0x38, 0x38, 0x38, 0xFF, 0x27, 0x27, 0x27, 0xFF, 
    0xC7, 0xC7, 0xC7, 0xFF, 0x5C, 0x5C, 0x5C, 0xFF, 0x67, 0x67, 0x67, 0xFF, 0x91, 0x91, 0x91, 0xFF, 0x97, 0x97, 0x97, 0xFF, 
    0x8F, 0x8F, 0x8F, 0xFF, 0x92, 0x92, 0x92, 0xFF, 0x82, 0x82, 0x82, 0xFF, 0x99, 0x99, 0x99, 0xFF, 0xA2, 0xA2, 0xA2, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xB5, 0xB5, 0xB5, 0xFF, 0x72, 0x72, 0x72, 0xFF, 0x3D, 0x3D, 0x3D, 0xFF, 
    0x8F, 0x8F, 0x8F, 0xFF, 0x71, 0x71, 0x71, 0xFF, 0x89, 0x89, 0x89, 0xFF, 0xBB, 0xBB, 0xBB, 0xFF, 0x61, 0x61, 0x61, 0xFF, 
    0xAB, 0xAB, 0xAB, 0xFF, 0xB2, 0xB2, 0xB2, 0xFF, 0x89, 0x89, 0x89, 0xFF, 0x27, 0x27, 0x27, 0xFF, 0x2F, 0x2F, 0x2F, 0xFF, 
    0xA4, 0xA4, 0xA4, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xB9, 0xB9, 0xB9, 0xFF, 0xBB, 0xBB, 0xBB, 0xFF, 
    0x57, 0x57, 0x57, 0xFF, 0x42, 0x42, 0x42, 0xFF, 0x8F, 0x8F, 0x8F, 0xFF, 0x77, 0x77, 0x77, 0xFF, 0x6A, 0x6A, 0x6A, 0xFF, 
    0xBC, 0xBC, 0xBC, 0xFF, 0xB0, 0xB0, 0xB0, 0xFF, 0x5E, 0x5E, 0x5E, 0xFF, 0xD1, 0xD1, 0xD1, 0xFF, 0x2E, 0x2E, 0x2E, 0xFF, 
    0x94, 0x94, 0x94, 0xFF, 0x96, 0x96, 0x96, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x2C, 0x2C, 0x2C, 0xFF, 
    0xC6, 0xC6, 0xC6, 0xFF, 0x1E, 0x1E, 0x1E, 0xFF, 0x70, 0x70, 0x70, 0xFF, 0x7D, 0x7D, 0x7D, 0xFF, 0x3E, 0x3E, 0x3E, 0xFF, 
    0xC3, 0xC3, 0xC3, 0xFF, 0x3D, 0x3D, 0x3D, 0xFF, 0x31, 0x31, 0x31, 0xFF, 0x60, 0x60, 0x60, 0xFF, 0x51, 0x51, 0x51, 0xFF, 
    0x9B, 0x9B, 0x9B, 0xFF, 0x87, 0x87, 0x87, 0xFF, 0xC7, 0xC7, 0xC7, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xC6, 0xC6, 0xC6, 0xFF, 0x50, 0x50, 0x50, 0xFF, 0x3B, 0x3B, 0x3B, 0xFF, 0x87, 0x87, 0x87, 0xFF, 0x94, 0x94, 0x94, 0xFF, 
    0xBC, 0xBC, 0xBC, 0xFF, 0x28, 0x28, 0x28, 0xFF, 0xE3, 0xE3, 0xE3, 0xFF, 0x2D, 0x2D, 0x2D, 0xFF, 0xE2, 0xE2, 0xE2, 0xFF, 
    0xB4, 0xB4, 0xB4, 0xFF, 0x25, 0x25, 0x25, 0xFF, 0x5D, 0x5D, 0x5D, 0xFF, 0xA7, 0xA7, 0xA7, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xC8, 0xC8, 0xC8, 0xFF, 0xA9, 0xA9, 0xA9, 0xFF, 0x5C, 0x5C, 0x5C, 0xFF, 0x9A, 0x9A, 0x9A, 0xFF, 
    0x48, 0x48, 0x48, 0xFF, 0x53, 0x53, 0x53, 0xFF, 0x94, 0x94, 0x94, 0xFF, 0x25, 0x25, 0x25, 0xFF, 0x5A, 0x5A, 0x5A, 0xFF, 
    0x7E, 0x7E, 0x7E, 0xFF, 0x89, 0x89, 0x89, 0xFF, 0xC6, 0xC6, 0xC6, 0xFF, 0xC9, 0xC9, 0xC9, 0xFF, 0x39, 0x39, 0x39, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xD7, 0xD7, 0xD7, 0xFF, 0xCC, 0xCC, 0xCC, 0xFF, 0x43, 0x43, 0x43, 0xFF, 
    0x74, 0x74, 0x74, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x34, 0x34, 0x34, 0xFF, 0xDA, 0xDA, 0xDA, 0xFF, 0x49, 0x49, 0x49, 0xFF, 
    0x32, 0x32, 0x32, 0xFF, 0xCF, 0xCF, 0xCF, 0xFF, 0x1E, 0x1E, 0x1E, 0xFF, 0x9D, 0x9D, 0x9D, 0xFF, 0xB5, 0xB5, 0xB5, 0xFF, 
    0xBC, 0xBC, 0xBC, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x4B, 0x4B, 0x4B, 0xFF, 0x53, 0x53, 0x53, 0xFF, 
    0x20, 0x20, 0x20, 0xFF, 0x34, 0x34, 0x34, 0xFF, 0xAC, 0xAC, 0xAC, 0xFF, 0x86, 0x86, 0x86, 0xFF, 0xD5, 0xD5, 0xD5, 0xFF, 
    0x6E, 0x6E, 0x6E, 0xFF, 0xCE, 0xCE, 0xCE, 0xFF, 0x35, 0x35, 0x35, 0xFF, 0x53, 0x53, 0x53, 0xFF, 0xE2, 0xE2, 0xE2, 0xFF, 
    0xD2, 0xD2, 0xD2, 0xFF, 0xB7, 0xB7, 0xB7, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xB5, 0xB5, 0xB5, 0xFF, 
    0x2A, 0x2A, 0x2A, 0xFF, 0xC5, 0xC5, 0xC5, 0xFF, 0x5F, 0x5F, 0x5F, 0xFF, 0x2E, 0x2E, 0x2E, 0xFF, 0xE1, 0xE1, 0xE1, 0xFF, 
    0x2A, 0x2A, 0x2A, 0xFF, 0x4D, 0x4D, 0x4D, 0xFF, 0x84, 0x84, 0x84, 0xFF, 0xA1, 0xA1, 0xA1, 0xFF, 0x7A, 0x7A, 0x7A, 0xFF, 
    0x4A, 0x4A, 0x4A, 0xFF, 0x97, 0x97, 0x97, 0xFF, 0x8A, 0x8A, 0x8A, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x93, 0x93, 0x93, 0xFF, 0x38, 0x38, 0x38, 0xFF, 0x24, 0x24, 0x24, 0xFF, 0xD0, 0xD0, 0xD0, 0xFF, 0xA2, 0xA2, 0xA2, 0xFF, 
    0x6C, 0x6C, 0x6C, 0xFF, 0x21, 0x21, 0x21, 0xFF, 0x42, 0x42, 0x42, 0xFF, 0xA6, 0xA6, 0xA6, 0xFF, 0x93, 0x93, 0x93, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x72, 0x72, 0x72, 0xFF, 0x4C, 0x4C, 0x4C, 0xFF, 0xC6, 0xC6, 0xC6, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xC5, 0xC5, 0xC5, 0x00, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x4C, 0x4C, 0x4C, 0x00, 
};

static const ximage image_uniform_additive_noise_16x16 =
{
    &uniform_additive_noise_16x16_data[0],
    16, 16,
    64,
    XPixelFormatRGBA32
};