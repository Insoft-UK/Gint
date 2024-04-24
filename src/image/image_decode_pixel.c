#include <gint/image.h>

int image_decode_pixel(image_t const *img, int pixel)
{
    if(IMAGE_IS_RGB16(img->format))
        return pixel;
    else if(IMAGE_IS_P8(img->format))
        return img->palette[pixel+128];
    else if(IMAGE_IS_P4(img->format))
        return img->palette[pixel];
    return -1;
}
