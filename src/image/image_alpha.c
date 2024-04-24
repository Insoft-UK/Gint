#include <gint/image.h>

int image_alpha(int format)
{
    switch(format) {
    case IMAGE_RGB565A:
        return 0x0001;
    case IMAGE_P8_RGB565A:
        return -128;
    case IMAGE_P4_RGB565A:
        return 0;
    default:
        /* A value that cannot be found in any pixel of any format */
        return 0x10000;
    }
}
