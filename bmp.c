#include "bmp.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int bmp_read_header(FILE* file, struct bmp_header* bmp_header);

void bmp_flip(struct bmp_file* bmp_file);

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file);

int bmp_read_dib_header(FILE* file, struct dib_header* dib_header);

void bmp_write_header(FILE* file, struct bmp_file* bmp_file);

int bmp_is_id_valid(const struct bmp_header* bmp_header);

void bmp_write_clean(FILE* file, struct bmp_file* bmp_file);

int bmp_allocate_data(struct pixel_data** data_ptr, int32_t width, int32_t height);

struct pixel_data bmp_median(const struct bmp_file* bmp_file, int32_t x, int32_t y);

int bmp_asc_comparator(const void* a, const void* b);

int8_t bmp_get_padding_bytes_no(const int32_t bmp_width);

struct pixel_data* bmp_reorganize_for_saving(const struct bmp_file* bmp_file);

uint bmp_width(struct bmp_file* s_bmp_file) {
    return s_bmp_file->s_dib_header.bitmap_width;
}

uint bmp_height(struct bmp_file* s_bmp_file) {
    return s_bmp_file->s_dib_header.bitmap_height;
}

int bmp_read_header(FILE* file, struct bmp_header* bmp_header) {
    if(file == NULL)
        return BMP_FILE_NULL;

    memset(&*bmp_header, 0, sizeof(struct bmp_header));
    fseek(file, 0, SEEK_SET);
    fread(&*bmp_header, 14, 1, file);

    // Check if BMP signature is equal to "BM"
    if(bmp_is_id_valid(bmp_header))
        return BHE_INVALID_SIGNATURE;

    if(bmp_header->file_size == 0) 
        return BHE_SIZE_ZERO;

    return bmp_header->pixel_data_offset == 0;
}

int bmp_allocate_data(struct pixel_data** data_ptr, int32_t width, int32_t height) {
    long bmp_size = width * height;
    (*data_ptr) = malloc(sizeof(struct pixel_data) * bmp_size);

    if((*data_ptr) == NULL)
        return BMP_MEMERROR;

    return 0;
}

int bmp_is_id_valid(const struct bmp_header* bmp_header) {
    if(bmp_header->id != 0x4D42)
        return BHE_INVALID_SIGNATURE;

    return 0;
}

// This function returns the closest scan line length in bytes
int8_t bmp_get_padding_bytes_no(const int32_t bmp_width) {
    int32_t actual_byte_length = sizeof(struct pixel_data) * bmp_width;
    return (4 - (actual_byte_length % 4)) % 4;
}

// 1. Compute the whole pixel data size in bytes (with padding)
// 2. Read the whole pixel data (with padding)
// 3. Load actual pixel data (without padding), but start filling bmp_file->data array from the end
int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file)
{
    if(file == NULL)
        return BMP_FILE_NULL;
    
    if(bmp_file->data == NULL)
        return BMP_DATA_NULL;

    // Move to the beginning of pixel data
    fseek(file, bmp_file->s_header.pixel_data_offset, SEEK_SET);

    int8_t padding_bytes_no = bmp_get_padding_bytes_no(bmp_file->s_dib_header.bitmap_width);
    bmp_file->padding_bytes_no = padding_bytes_no;
    unsigned long pixel_data_byte_size = (bmp_file->s_dib_header.bitmap_width
                                         * bmp_file->s_dib_header.bitmap_height
                                         * sizeof(struct pixel_data))
                                         + (bmp_file->s_dib_header.bitmap_height
                                         * padding_bytes_no);
    
    struct pixel_data* tmp = malloc(pixel_data_byte_size);
    struct pixel_data* tmp2 = tmp;

    if(tmp == NULL)
        return BMP_MEMERROR;

    fread(tmp, pixel_data_byte_size, 1, file);

    int32_t current_index = (bmp_file->s_dib_header.bitmap_height * bmp_file->s_dib_header.bitmap_width)
                            - bmp_file->s_dib_header.bitmap_width;

    while(current_index >= 0)
    {
        memcpy(&bmp_file->data[current_index], tmp2, sizeof(struct pixel_data) * bmp_file->s_dib_header.bitmap_width);
        current_index -= bmp_file->s_dib_header.bitmap_width;
        tmp2 += bmp_file->s_dib_header.bitmap_width;
        // Casting to char* lets us advance an address as far as we want and jump over padding bytes
        tmp2 = (struct pixel_data*)((char*)tmp2 + padding_bytes_no);
    }

    free(tmp);
    return 0;
}

int bmp_read_dib_header(FILE* file, struct dib_header* dib_header) {
    if(file == NULL)
        return BMP_FILE_NULL;

    memset(&*dib_header, 0, sizeof(struct dib_header));
    fseek(file, 14, SEEK_SET);
    fread(&*dib_header, 40, 1, file);

    if(dib_header->bits_per_pixel != 24)
        return DHE_INVALID_FORMAT;
    
    return 0;
}

int bmp_load(const char* file_name, struct bmp_file* s_bmp_file) {
    FILE* file = NULL;
    memset(&*s_bmp_file, 0, sizeof(struct bmp_file));
    enum BMP_ERROR status = 0;

    if((file = fopen(file_name, "r")) == NULL)
        return BMP_FILE_NULL;

    if((status = bmp_read_header(file, &s_bmp_file->s_header)) != 0)
        return status;

    if((status = bmp_read_dib_header(file, &s_bmp_file->s_dib_header)) != 0)
        return status;

    if(bmp_allocate_data(&s_bmp_file->data, bmp_width(s_bmp_file), bmp_height(s_bmp_file)))
        return BMP_MEMERROR;
    
    if((status = bmp_read_pixel_data(file, s_bmp_file)) != 0)
    {
        bmp_free(s_bmp_file);
        return status;
    }

    fclose(file);
    return 0;
}

void bmp_free(struct bmp_file* s_bmp_file) {
    if(s_bmp_file && s_bmp_file->data) {
        free(s_bmp_file->data);
        s_bmp_file->data = NULL;
    }
}

struct pixel_data bmp_pixel_at(struct bmp_file* bmp_file, int x, int y) {
    if(bmp_file && x >= 0 && y >= 0)
        return bmp_file->data[y * bmp_width(bmp_file) + x];
    
    struct pixel_data empty;
    memset(&empty, 0, sizeof(struct pixel_data));
    return empty;
}

void bmp_set_pixel_at(struct bmp_file* bmp_file, struct pixel_data* new_pixel, int x, int y) {
    if(bmp_file && new_pixel && x >= 0 && y >= 0)
        memcpy(&bmp_file->data[y * bmp_width(bmp_file) + x], &*new_pixel, sizeof(struct pixel_data));
}

int bmp_init(struct bmp_file* bmp_file, int32_t width, int32_t height) {
    memset(bmp_file, 0, sizeof(struct bmp_file));

    if(bmp_allocate_data(&bmp_file->data, width, height))
        return BMP_MEMERROR;

    bmp_file->s_dib_header.bitmap_width = width;
    bmp_file->s_dib_header.bitmap_height = height;
    return 0;
}

void bmp_write_clean(FILE* file, struct bmp_file* bmp_file) {
    if(file && bmp_file && bmp_file->data) {
        // Create BMP_HEADER
        struct bmp_header bmp_header;
        memset(&bmp_header, 0, sizeof(struct bmp_header));

        // Write BMP signature
        ((u_int8_t*)&bmp_header.id)[0] = 0x42; // B
        ((u_int8_t*)&bmp_header.id)[1] = 0x4D; // M

        bmp_header.file_size = 54 + bmp_file->s_dib_header.bitmap_width *
                                    bmp_file->s_dib_header.bitmap_height * 3; 
        bmp_header.pixel_data_offset = 54;

        memcpy(&bmp_file->s_header, &bmp_header, sizeof(struct bmp_header));
        fwrite(&bmp_file->s_header, sizeof(struct bmp_header), 1, file);

        // Create DIB_HEADER
        struct dib_header dib_header;
        dib_header.header_length = 40;
        dib_header.bitmap_width = bmp_file->s_dib_header.bitmap_width;
        dib_header.bitmap_height = bmp_file->s_dib_header.bitmap_height;
        dib_header.color_planes_no = 1;
        dib_header.bits_per_pixel = 24;
        dib_header.bi_rgb = 0;
        dib_header.raw_bitmap_data_size = bmp_file->s_dib_header.bitmap_width *
                                    bmp_file->s_dib_header.bitmap_height * 3;
        dib_header.print_resolution_h = 0; 
        dib_header.print_resolution_v = 0;
        dib_header.palette_colors_number = 0;
        dib_header.important_colors = 0; // All important       

        memcpy(&bmp_file->s_dib_header, &dib_header, sizeof(struct dib_header));
        fwrite(&bmp_file->s_dib_header, sizeof(struct dib_header), 1, file);

        // Calculate padding for scanlines
        bmp_file->padding_bytes_no = bmp_get_padding_bytes_no(bmp_file->s_dib_header.bitmap_width);
    }
}

// 1. Write header
// 2. Write DIB header
// 3. Write pixel data backwards (with padding)
uint bmp_save(const char* file_name, struct bmp_file* s_bmp_file) {
    FILE* file = NULL;

    if((file = fopen(file_name, "w")) == NULL)
        return BMP_FILE_NULL;

    // If there is no valid id, we have to fill all the metadata needed
    if(bmp_is_id_valid(&s_bmp_file->s_header)) 
        bmp_write_clean(file, s_bmp_file);
    else {
        fwrite(&s_bmp_file->s_header, sizeof(struct bmp_header), 1, file);
        fwrite(&s_bmp_file->s_dib_header, sizeof(struct dib_header), 1, file);
    }

    unsigned long pixel_data_byte_size = (s_bmp_file->s_dib_header.bitmap_width
                                         * s_bmp_file->s_dib_header.bitmap_height
                                         * sizeof(struct pixel_data))
                                         + (s_bmp_file->s_dib_header.bitmap_height
                                         * s_bmp_file->padding_bytes_no);
    
    struct pixel_data* tmp = malloc(pixel_data_byte_size);
    struct pixel_data* tmp2 = tmp;

    if(tmp == NULL)
        return BMP_MEMERROR;

    int32_t current_index = (s_bmp_file->s_dib_header.bitmap_height * s_bmp_file->s_dib_header.bitmap_width)
                            - s_bmp_file->s_dib_header.bitmap_width;

    while(current_index >= 0)
    {
        memcpy(tmp2, &s_bmp_file->data[current_index], sizeof(struct pixel_data) * s_bmp_file->s_dib_header.bitmap_width);
        current_index -= s_bmp_file->s_dib_header.bitmap_width;
        tmp2 += s_bmp_file->s_dib_header.bitmap_width;
        memset(tmp2, 0, s_bmp_file->padding_bytes_no);
        tmp2 = (struct pixel_data*)((char*)tmp2 + s_bmp_file->padding_bytes_no);
    }

    fwrite(tmp, pixel_data_byte_size, 1, file);
    free(tmp);
    fclose(file);
    return 0;
}

void bmp_negative_filter(struct bmp_file* bmp_file) {
    if(bmp_file && bmp_file->data) {
        for(int i = 0; i < bmp_height(bmp_file); i++) {
            for(int j = 0; j < bmp_width(bmp_file); j++) {
                struct pixel_data current = bmp_pixel_at(bmp_file, j, i);
                struct pixel_data new_color = {255 - current.b,
                                               255 - current.g,
                                               255 - current.r};

                bmp_set_pixel_at(bmp_file, &new_color, j, i);
            }
        }
    }
}