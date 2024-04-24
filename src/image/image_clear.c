#include <gint/image.h>

void image_clear(image_t *img)
{
    if(!IMAGE_IS_ALPHA(img->format))
        return;

    image_fill(img, image_alpha(img->format));
}
