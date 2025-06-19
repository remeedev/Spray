#include <stdio.h>
#include <windows.h>
#include <zlib.h>
#include <string.h>
#include "headers/log.h"

void PrintBytes(unsigned char *bytes, size_t len){
    printf("\n");
    for (int i = 0; i < len; i++){
        printf("%d ", bytes[i]);
        if (i%8 == 0 && i != 0){
            printf("\n");
        }
    }
    printf("\n");
}

int CompareBytes(unsigned char *bytes, unsigned char *target, size_t len){
    for (int i = 0; i < len; i++){
        if (bytes[i] != target[i]) return FALSE;
    }
    return TRUE;
}

int b2i(unsigned char *bytes, size_t size){
    int out = 0;
    for (int i = 0; i < size; i++){
        out |= bytes[i]<<8*(size-i-1);
    }
    return out;
}

size_t ReadNextSection(unsigned char **section_name, unsigned char **section_data, FILE *file){
    // Function will get the next part of the file to be read, and fills section_name and section_data with the section information (Returns size of data retrieved)
    size_t size = 0;
    char header_read[4];
    fread(header_read, 1, 4, file); // Read the first 4 bytes (Size)
    size = b2i(header_read, 4); // Convert Bytes to integer

    // Retrieving section information
    *section_name = (unsigned char *)malloc(4);
    if (*section_name == NULL){
        printf("Couldn't allocate space for section name!\n");
        return 0;
    }
    fread(*section_name, 1, 4, file);

    // Actual body
    *section_data = (unsigned char *)malloc(size);
    if (*section_data == NULL){
        printf("Couldn't allocate space for section data!\n");
        return 0;
    }
    fread(*section_data, 1, size, file);

    // Delete the final part of each chunk
    char crc_dump[4];
    fread(crc_dump, 1, 4, file);
    return size;
}

unsigned char *ReadPNG(char *file_name, size_t *widthOutput, size_t *heightOutput, size_t *BytesPerPixel, size_t *ImageLength){
    // Constants
    unsigned char *PNGSignature = (unsigned char [8]){137, 80, 78, 71, 13, 10, 26, 10};

    // Variables that will be used
    int width, height, color_profile; // Basic information of the Image when it comes to reading the image
    unsigned char *ImageData = NULL; // Will hold the binary of the ImageData
    int compressed_size = 0;

    // Opening the file
    FILE *file = fopen(file_name, "rb");
    if (file == NULL){
        printf("Couldn't find '%s'!\n", file_name);
        return NULL;
    }

    // Storing the header separately to check for the PNG Signature
    unsigned char header[8];
    fread(header, 1, 8, file);
    if (!CompareBytes(header, PNGSignature, 8)){
        printf("File is not a PNG!\n"); // At heart at least <3
        fclose(file);
        return NULL;
    }

    // Ready to read data
    unsigned char* section_name = NULL;
    unsigned char* section_data = NULL; // Data output for all sections
    size_t image_size = 0;
    size_t chunk_size;
    while ((chunk_size = ReadNextSection(&section_name, &section_data, file)) > 0){
        if (CompareBytes(section_name, "IHDR", 4)){
            width = b2i(section_data, 4);
            char height_bytes[4];
            for (int i = 0; i < 4; i++){
                height_bytes[i] = section_data[ i + 4 ];
            }
            height = b2i(height_bytes, 4);
            color_profile = section_data[9]==6 ? 4 : 3; // Don't plan on using grayscale, only RGBa
        }else{
            if (CompareBytes(section_name, "IDAT", 4)){
                if (ImageData == NULL){
                    ImageData = (unsigned char *)malloc(chunk_size);
                    if (ImageData == NULL){
                        printf("Error creating data holder for [%s]\n", file_name);
                        fclose(file);
                        free(section_data);
                        free(section_name);
                        return NULL;
                    }
            }else{
                unsigned char *tmp = (unsigned char *)realloc(ImageData, compressed_size+chunk_size);
                if (tmp == NULL){
                    printf("Error reallocating memory!\n");
                    fclose(file);
                    free(ImageData);
                    free(section_data);
                    free(section_name);
                    return NULL;
                }
                ImageData = tmp;
            }
            for (int i = 0; i < chunk_size; i++){
                ImageData[compressed_size++] = section_data[i];
            }
        }}
        free(section_data);
        free(section_name);
    }
    *heightOutput = height;
    *widthOutput = width;
    *BytesPerPixel = color_profile;
    *ImageLength = compressed_size;
    fclose(file);
    return ImageData;
}

unsigned char * SubFix(unsigned char * bytes, size_t len, size_t BytesPerPixel){ // Removes the filtering for compression
    unsigned char *destBytes = (unsigned char *)malloc(len);
    if (destBytes == NULL){
        printf("Unable to allocate space for destination bytes!\n");
        return NULL;
    }
    for (size_t i = 0; i < len; i++){
        if (i >= BytesPerPixel){
            destBytes[i] = bytes[i] + destBytes[i-BytesPerPixel] % 256;
        }else{
            destBytes[i] = bytes[i];
        }
    }
    return destBytes;
}

unsigned char * UncompressPNG(unsigned char*ImageData, size_t width, size_t height, size_t BytesPerPixel, size_t ImageLen, size_t *SizeOut){ // Return the crude values, will need unfiltering
    uLongf expected_size = height*(1+width*BytesPerPixel); // Filter Byte
    uLongf out_size = expected_size;
    unsigned char *raw_data = (unsigned char *)malloc(expected_size);
    int out = uncompress(raw_data, &out_size, ImageData, ImageLen);
    if (out != Z_OK){
        printf("Error uncompressing the data!\n");
        return NULL;
    }
    if (out_size != expected_size){
        printf("Size of data didn't match expected\n%d Bytes expected\n%d Bytes received after uncompression\n", expected_size, out_size);
        return NULL;
    }
    int curr_pos = 0;
    // PrintBytes(raw_data, out_size);
    unsigned char processed_data[width*4];
    memset(processed_data, 0, width*4);
    unsigned char *color_data = (unsigned char *)malloc(width*height*BytesPerPixel);
    int curr_color = 0;
    if (color_data == NULL){
        printf("Unable to allocate space for color data\n");
        return NULL;
    }
    for (int y = 0; y < height; y++){
        memset(processed_data, 0, width*4);
        int filter = raw_data[curr_pos++];
        for (int x = 1; x < width*4+1; x++){
            processed_data[x-1] = raw_data[curr_pos++];
        }
        if (filter == 1){
            unsigned char * clean_row = SubFix(processed_data, width*4, BytesPerPixel);
            for (size_t i = 0; i < width*4; i++){
                color_data[curr_color++] = clean_row[i];
            }
            free(clean_row);
        }
    }
    *SizeOut = width*height*4;
    return color_data;
}

HBITMAP ConvertBytesBMP(unsigned char *Bytes, size_t len, int width, int height, int BytesPerPixel){
    unsigned char *ColorInfo = (unsigned char *)malloc(len);
    unsigned char prev_alpha;
    for (size_t i = 0; i < len; i++){
        ColorInfo[i] = Bytes[i];
        if ((i+1)%4 == 0){
            int r, g, b;
            r = ColorInfo[i-BytesPerPixel+1];
            g = ColorInfo[i-BytesPerPixel+2];
            b = ColorInfo[i-BytesPerPixel+3];
            
            // INVERTING BLUE & RED
            ColorInfo[i-BytesPerPixel+1] = b;
            ColorInfo[i-BytesPerPixel+3] = r;
        }
    }

    // Preparing bmp for packaging
    BITMAPINFO bmi = {0};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = width;
    bmi.bmiHeader.biHeight = -height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void *pBits;
    HBITMAP bmp = CreateDIBSection(NULL, &bmi, DIB_RGB_COLORS, &pBits, NULL, 0);
    if (!bmp){
        printf("Error creating bitmap!\n");
        free(ColorInfo);
        return NULL;
    }
    memcpy(pBits, ColorInfo, len);
    free(ColorInfo);
    return bmp;
}

HBITMAP LoadPNGAsBmp(char *file_name){
    size_t width, height, bpp, ilen, clen;
    unsigned char *ImageData = ReadPNG(file_name, &width, &height, &bpp, &ilen);
    unsigned char *CleanData = UncompressPNG(ImageData, width, height, bpp, ilen, &clen);
    free(ImageData); // Don't need it no more
    HBITMAP bmp = ConvertBytesBMP(CleanData, clen, width, height, bpp);
    free(CleanData);
    return bmp;
}