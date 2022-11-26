#include "pixel.h"


pixel_t pixel3_crear(bool r, bool g, bool b){
    pixel_t p = 0;
    if(r){
        p = p | 0x04;
    }
    if(g){
        p = p | 0x02;
    }
    if(b){
        p = p | 0x01;
    }
    return p;
}

void pixel3_a_rgb(pixel_t pixel3, uint8_t *r, uint8_t *g, uint8_t *b){
    *r = 0;
    *g = 0;
    *b = 0;
    if(pixel3 & 0x04){
        *r = 255;
    }
    if(pixel3 & 0x02){
        *g = 255;
    }
    if(pixel3 & 0x01){
        *b = 255;
    }
}

pixel_t pixel12_crear(uint8_t r, uint8_t g, uint8_t b){
    pixel_t p = 0;
    p = (p | r) << 4;
    p = (p | g) << 4;
    p = p | b;

    return p; //(r << 8) | (g<<4) | b;
}

void pixel12_a_rgb(pixel_t pixel12, uint8_t *r, uint8_t *g, uint8_t *b){
    *b = (pixel12 & 0x0f) << 4;
    *g = ((pixel12 >> 4) & 0x0f) << 4;
    *r = ((pixel12 >> 8) & 0x0f) << 4;
}

