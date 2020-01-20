#ifndef BMP_H
#define BMP_H

#include <sys/types.h>

// BGR24 pixel BMP data model
#pragma pack(push, 1) // Use no padding at all
struct pixel_data {
    u_int8_t b;
    u_int8_t g;
    u_int8_t r;
};
#pragma pack(pop)

struct bmp_file {
    #pragma pack(push, 1)
    struct bmp_header {
        u_int16_t id;
        uint file_size;
        uint blank_unused;
        uint pixel_data_offset;
    } s_header;

    struct dib_header {
        uint header_length;
        int32_t bitmap_width;
        int32_t bitmap_height;
        u_int16_t color_planes_no;
        u_int16_t bits_per_pixel;
        uint bi_rgb;
        uint raw_bitmap_data_size;
        uint print_resolution_h;
        uint print_resolution_v;
        uint palette_colors_number;
        uint important_colors;
    } s_dib_header;
    #pragma pack(pop)

    struct pixel_data* data;
    int32_t scan_line_byte_length;
    int32_t actual_data_line_length;
};

enum BMP_ERROR {
    BMP_FILE_NULL = 1,
    BMP_NULL_STRUCT,
    BMP_MEMERROR,
    BMP_DATA_NULL,
    BHE_INVALID_SIGNATURE,
    BHE_SIZE_ZERO,
    DHE_INVALID_FORMAT,
};

int bmp_load(const char* file_name, struct bmp_file* s_bmp_file);

void bmp_free(struct bmp_file* s_bmp_file);

struct pixel_data bmp_pixel_at(struct bmp_file* bmp_file, int x, int y);

void bmp_set_pixel_at(struct bmp_file* bmp_file, struct pixel_data* new_pixel, int x, int y);

uint bmp_width(struct bmp_file* s_bmp_file);

uint bmp_height(struct bmp_file* s_bmp_file);

uint bmp_save(const char* file_name, struct bmp_file* s_bmp_file);

int bmp_init(struct bmp_file* bmp_file, int32_t width, int32_t height);

void bmp_negative_filter(struct bmp_file* bmp_file);

#endif