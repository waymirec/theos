#pragma once

typedef struct {
    unsigned char magic[2];
    unsigned char mode;
    unsigned char char_size;
} psf1_header_t;

typedef struct {
    psf1_header_t *header;
    void *glyph_buffer;
} psf1_font_t;
