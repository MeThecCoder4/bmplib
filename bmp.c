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

int32_t bmp_proper_sl_length(const int32_t bmp_width);

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
    if((((u_int8_t*)&bmp_header->id)[0] != 0x42) ||
        (((u_int8_t*)&bmp_header->id)[1] != 0x4D))
        return BHE_INVALID_SIGNATURE;

    return 0;
}

// This function returns the closest scan line length in bytes
int32_t bmp_proper_sl_length(const int32_t bmp_width) {
    int32_t actual_length = sizeof(struct pixel_data) * bmp_width;

    // Find the closest length divisible by 4
    while(actual_length % 4 != 0) {
        actual_length++;
    }

    return actual_length;
}

/* This function works by firstly reading all pixel data into a temporary variable.
   It is done, because reading from disk is the most time consuming process,
   so we want to do it in one fread() call. Pay attention that there is also padding included
   and that's why we need to get the real pixel data out of the temporary data. */

int bmp_read_pixel_data(FILE* file, struct bmp_file* bmp_file) {
    if(file == NULL)
        return BMP_FILE_NULL;
    
    if(bmp_file->data == NULL)
        return BMP_DATA_NULL;
    
    // Move to the beginning of pixel data
    fseek(file, bmp_file->s_header.pixel_data_offset, SEEK_SET);
    int32_t scan_line_byte_length = bmp_proper_sl_length(bmp_file->s_dib_header.bitmap_width);
    struct pixel_data* tmp = NULL;
    bmp_file->scan_line_byte_length = scan_line_byte_length;
    
    if(bmp_allocate_data(&tmp, scan_line_byte_length, bmp_height(bmp_file)))
        return BMP_MEMERROR;

    // Read temporary data with padding
    int32_t data_with_padding_byte_length
     = scan_line_byte_length * bmp_height(bmp_file);
    int32_t pixels_num_with_padding = data_with_padding_byte_length / sizeof(struct pixel_data);
    fread(tmp, sizeof(struct pixel_data), pixels_num_with_padding,file);

    // Start reorganizing data from the last line
    struct pixel_data* current_line
     = tmp + pixels_num_with_padding - (scan_line_byte_length / sizeof(struct pixel_data));
    struct pixel_data* tmp2 = bmp_file->data;
    int32_t actual_data_line_byte_length = bmp_width(bmp_file) * sizeof(struct pixel_data);
    bmp_file->actual_data_line_byte_length = actual_data_line_byte_length;

    while(current_line >= tmp) {
        memcpy(tmp2, current_line, actual_data_line_byte_length);
        tmp2 += actual_data_line_byte_length / sizeof(struct pixel_data);
        current_line -= scan_line_byte_length / sizeof(struct pixel_data);
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
        return status;

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
    }
}

struct pixel_data* bmp_reorganize_for_saving(const struct bmp_file* bmp_file) {
    if(bmp_file == NULL)
        return NULL;
    
    struct pixel_data* tmp = NULL;

    if(bmp_allocate_data(&tmp, bmp_file->scan_line_byte_length,
                               bmp_height((struct bmp_file*) bmp_file)))
        return NULL;
                             
    struct pixel_data* data = bmp_file->data;
    struct pixel_data* current_line
     = tmp + (((bmp_file->scan_line_byte_length * bmp_height((struct bmp_file*) bmp_file)) - bmp_file->scan_line_byte_length)
           / sizeof(struct pixel_data));

    // Filling data with zeros first to ensure padding - this one is important,
    // there could be trash values otherwise
    memset(tmp, 0, bmp_file->scan_line_byte_length * bmp_height((struct bmp_file*) bmp_file));

    while(current_line >= tmp) {
        memcpy(current_line, data, bmp_file->actual_data_line_byte_length);
        current_line -= bmp_file->scan_line_byte_length / sizeof(struct pixel_data);
        data += bmp_file->actual_data_line_byte_length / sizeof(struct pixel_data);
    }

    // At this point the data has been reorganized for save operation
    return tmp;
}

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

    struct pixel_data* reorganized_data = bmp_reorganize_for_saving(s_bmp_file);
    fwrite(reorganized_data, sizeof(struct pixel_data),
           (s_bmp_file->scan_line_byte_length * bmp_height(s_bmp_file)) / sizeof(struct pixel_data), file);

    fclose(file);
    free(reorganized_data);
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