# bmplib
## A simple BMP library written in C language

Current library version supports 24bits/pixel BMP formats only.

### Compilation instructions:
1. Download project files.
1. To build a static library simply type:
```bash
make
```
3. To compile your project using our newly created static library with gcc:
```bash
gcc -o exec main.c file1.c file2.c fileN.c bmplib.a
```
### Functions and usage:
1. `int bmp_load(const char* file_name, struct bmp_file* s_bmp_file)` - Use this function to load BMP file from disk.
A structure does not need to be initialized by `bmp_init()` first.
2. `void bmp_free(struct bmp_file* s_bmp_file)` - Use this function to free memory allocated for pixel data.
3. `struct pixel_data bmp_pixel_at(struct bmp_file* bmp_file, int x, int y)` - Use this function to get a particular pixel from (x, y).
4. `void bmp_set_pixel_at(struct bmp_file* bmp_file, struct pixel_data* new_pixel, int x, int y)` -
  Use this function to set a particular pixel at (x, y).
5. `uint bmp_width(struct bmp_file* s_bmp_file)` - Get image width.
6. `uint bmp_height(struct bmp_file* s_bmp_file)` - Get image height.
7. `uint bmp_save(const char* file_name, struct bmp_file* s_bmp_file)` - Use this function to save previously loaded or custom file.
8. `int bmp_init(struct bmp_file* bmp_file, int32_t width, int32_t height)` - Use this function to initialize custom file.
9. `void bmp_negative_filter(struct bmp_file* bmp_file)` - Use this function to apply the negative filter to a file.
