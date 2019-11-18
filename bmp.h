#include <sys/types.h>
#include <stdio.h>

// BGR24 pixel BMP data model
struct pixel_data {
    u_int8_t b;
    u_int8_t g;
    u_int8_t r;
};

struct bmp_file {
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

    struct pixel_data* data;
};

enum BMP_ERROR {
    BMP_FILE_NULL = 1,
    BMP_MEMERROR,
    BMP_DATA_NULL,
    BHE_INVALID_SIGNATURE,
    BHE_SIZE_ZERO,
    DHE_INVALID_FORMAT
};

uint bmp_width(struct bmp_file* s_bmp_file);

uint bmp_height(struct bmp_file* s_bmp_file);

int bmp_read_header(FILE* file, struct bmp_header* bmp_header);

void bmp_flip(struct bmp_file* bmp_file);

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file);

int bmp_read_dib_header(FILE* file, struct dib_header* dib_header);

int bmp_load(const char* file_name, struct bmp_file* s_bmp_file);

void bmp_free(struct bmp_file* s_bmp_file);