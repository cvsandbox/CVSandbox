#include <ximage.h>

static uint8_t replace_channel_16x16_data[] =
{
    0x00, 0x00, 0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 0x83, 0x00, 0x00, 0xFF, 0x8A, 0x00, 0x00, 0xFF, 0x98, 0x00, 0x00, 0xFF, 
    0xA5, 0x00, 0x00, 0xFF, 0xB2, 0x00, 0x00, 0xFF, 0xBF, 0x00, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x7F, 0x00, 0xFF, 0x00, 0x84, 0x00, 0xFF, 0x00, 0x91, 0x00, 0xFF, 0x00, 0x9E, 0x00, 0xFF, 0x00, 0xAB, 0x00, 0xFF, 
    0x00, 0xB7, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x83, 0x00, 0x00, 0xFF, 0x8A, 0x00, 0x00, 0xFF, 
    0x97, 0x00, 0x00, 0xFF, 0xA5, 0x00, 0x00, 0xFF, 0xB2, 0x00, 0x00, 0xFF, 0xBF, 0x00, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x7F, 0x00, 0xFF, 0x00, 0x84, 0x00, 0xFF, 0x00, 0x91, 0x00, 0xFF, 0x00, 0x9E, 0x00, 0xFF, 
    0x00, 0xAB, 0x00, 0xFF, 0x00, 0xB8, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x83, 0x00, 0x00, 0xFF, 
    0x8B, 0x00, 0x00, 0xFF, 0x97, 0x00, 0x00, 0xFF, 0xA5, 0x00, 0x00, 0xFF, 0xB2, 0x00, 0x00, 0xFF, 0xBF, 0x00, 0x00, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x7F, 0x00, 0xFF, 0x00, 0x85, 0x00, 0xFF, 0x00, 0x92, 0x00, 0xFF, 
    0x00, 0x9F, 0x00, 0xFF, 0x00, 0xAC, 0x00, 0xFF, 0x00, 0xB8, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x83, 0x00, 0x00, 0xFF, 0x8A, 0x00, 0x00, 0xFF, 0x98, 0x00, 0x00, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x7F, 0x00, 0xFF, 0x00, 0x85, 0x00, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0x00, 0x9E, 0x00, 0xFF, 0x00, 0xAB, 0x00, 0xFF, 0x00, 0xB8, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0x83, 0x00, 0x00, 0xFF, 0x8A, 0x00, 0x00, 0xFF, 0x97, 0x00, 0x00, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xBF, 0x00, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x7F, 0x00, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x00, 0xAC, 0x00, 0xFF, 0x00, 0xB8, 0x00, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x83, 0x00, 0x00, 0xFF, 0x8A, 0x00, 0x00, 0xFF, 0x98, 0x00, 0x00, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xB2, 0x00, 0x00, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x7F, 0x00, 0xFF, 0x00, 0x86, 0x00, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x00, 0x9F, 0x00, 0xFF, 0x00, 0xAC, 0x00, 0xFF, 
    0x00, 0xB9, 0x00, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 0xE9, 0xEC, 0x0E, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x00, 0x00, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x88, 0xFF, 0x00, 0x00, 0x94, 0xFF, 0x00, 0x00, 0xA0, 0xFF, 0x00, 0x00, 0xAD, 0xFF, 
    0x00, 0x00, 0xBA, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x88, 0x88, 0x88, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xA3, 0xA3, 0xA3, 0xFF, 0xB1, 0xB0, 0xB0, 0xFF, 0xBE, 0xBD, 0xBE, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x88, 0xFF, 0x00, 0x00, 0x94, 0xFF, 0x00, 0x00, 0xA1, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0x00, 0x00, 0xBA, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x80, 0x80, 0x80, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xA3, 0xA2, 0xA3, 0xFF, 0xB0, 0xB0, 0xB0, 0xFF, 0xBE, 0xBD, 0xBF, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x88, 0xFF, 0x00, 0x00, 0x94, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 
    0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0xA3, 0xA3, 0xA4, 0xFF, 0xB1, 0xB0, 0xB0, 0xFF, 
    0xBE, 0xBD, 0xBE, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x88, 0xFF, 
    0x00, 0x00, 0x95, 0xFF, 0x00, 0x00, 0xA1, 0xFF, 0xE9, 0xEC, 0x0E, 0xFF, 0x00, 0x00, 0xBA, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0xA0, 0xA0, 0xA0, 0xFF, 0x80, 0x80, 0x80, 0xFF, 0x89, 0x88, 0x89, 0xFF, 0x96, 0x96, 0x96, 0xFF, 0xA3, 0xA3, 0xA4, 0xFF, 
    0xB1, 0xB0, 0xB1, 0xFF, 0xBF, 0xBE, 0xBF, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x00, 0x00, 0x80, 0xFF, 
    0x00, 0x00, 0x88, 0xFF, 0x00, 0x00, 0x95, 0xFF, 0x00, 0x00, 0xA2, 0xFF, 0x00, 0x00, 0xAE, 0xFF, 0x00, 0x00, 0xBB, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x80, 0x80, 0x80, 0xFF, 0x89, 0x89, 0x89, 0xFF, 0x96, 0x96, 0x97, 0xFF, 
    0xA4, 0xA3, 0xA4, 0xFF, 0xB2, 0xB1, 0xB2, 0xFF, 0xBF, 0xBD, 0xBE, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 
    0x00, 0x00, 0x80, 0xFF, 0x00, 0x00, 0x88, 0xFF, 0x00, 0x00, 0x95, 0xFF, 0x00, 0x00, 0xA2, 0xFF, 0x00, 0x00, 0xAE, 0xFF, 
    0x00, 0x00, 0xBB, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0xA0, 0xA0, 0xA0, 0xFF, 0x80, 0x80, 0x80, 0xFF, 0x89, 0x89, 0x89, 0xFF, 
    0x96, 0x97, 0x97, 0xFF, 0xA4, 0xA4, 0xA4, 0xFF, 0xB2, 0xB1, 0xB2, 0xFF, 0xBF, 0xBE, 0xBF, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x40, 0x40, 0xFF, 
    0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 0x40, 0x40, 0x40, 0xFF, 
    0x00, 0x00, 0x00, 0x00, 
};

static const ximage image_replace_channel_16x16 =
{
    &replace_channel_16x16_data[0],
    16, 16,
    64,
    XPixelFormatRGBA32
};