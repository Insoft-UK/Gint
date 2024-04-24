#include <gint/image.h>
#include <gint/display.h>

void dimage_rgb16(int x, int y, image_t const *img, int eff)
{
	dsubimage_rgb16(x, y, img, 0, 0, img->width, img->height, eff);
}

void dsubimage_rgb16(int x, int y, image_t const *img,
	int left, int top, int w, int h, int eff)
{
	if(img->format == IMAGE_RGB565A)
		return dsubimage_rgb16_clearbg(x, y, img, left, top, w, h, eff,
			image_alpha(img->format));

	struct gint_image_box box = { x, y, w, h, left, top };
	struct gint_image_cmd cmd;

	if(!gint_image_mkcmd(&box, img, eff, false, false, &cmd, &dwindow))
		return;
	cmd.loop = gint_image_rgb16_normal;
	gint_image_rgb16_loop(DWIDTH, &cmd);
}

void dimage_rgb16_clearbg(int x, int y, image_t const *img, int eff,int bg)
{
	dsubimage_rgb16_clearbg(x, y, img, 0, 0, img->width, img->height, eff,
		bg);
}

void dsubimage_rgb16_clearbg(int x, int y, image_t const *img,
	int left, int top, int w, int h, int eff, int bg_color)
{
	struct gint_image_box box = { x, y, w, h, left, top };
	struct gint_image_cmd cmd;

	if(!gint_image_mkcmd(&box, img, eff, false, false, &cmd, &dwindow))
		return;
	cmd.color_1 = bg_color;
	cmd.loop = gint_image_rgb16_clearbg;
	gint_image_rgb16_loop(DWIDTH, &cmd);
}
