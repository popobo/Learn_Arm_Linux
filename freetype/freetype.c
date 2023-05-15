#include "freetype.h"
#include "lcd.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <assert.h>
#include <math.h>

#define INIT_FONT_SIZE 24

static FT_Library library;
static FT_Face face;
static FT_Error error;
static FT_GlyphSlot slot;

static void draw_bitmap(FT_Bitmap* bitmap, FT_Int x, FT_Int y)
{
    FT_Int i, j, p, q;
    x = x < 0 ? 0 : x;
    y = y < 0 ? 0 : y;
    FT_Int x_max = x + bitmap->width > lcd_width() ? lcd_width() : x + bitmap->width;
    FT_Int y_max = y + bitmap->rows > lcd_height() ? lcd_height() : y + bitmap->rows;

    for(j = y, q = 0; j < y_max; ++j, ++q)
    {
        for(i = x, p = 0; i < x_max; ++i, ++p)
        {
            lcd_put_pixel(i, j, bitmap->buffer[q * bitmap->width + p]);
        }
    }
}

int32_t freetype_init(const char* face_path, int32_t font_size)
{
    error = FT_Init_FreeType(&library);
    if (error != 0)
    {
        printf("failed to FT_Init_FreeType, error: %d", error);
        return -1;
    }

    error = FT_New_Face(library, face_path, 0, &face);
    if (error != 0)
    {
        printf("failed to FT_New_Face, error: %d", error);
        return -1;
    }

    slot = face->glyph;

    FT_Set_Pixel_Sizes(face, font_size, 0);
    
    return 0;
}

void freetype_set_font_size(int32_t font_size)
{
    assert(face != NULL);
    FT_Set_Pixel_Sizes(face, font_size, 0);
}

int32_t freetype_draw_char(int32_t x, int32_t y, const wchar_t *str)
{
    if (str == NULL || !lcd_is_init())
        return -1;
    error = FT_Load_Char(face, str[0], FT_LOAD_RENDER);
    if (error != 0)
    {
        printf("failed to FT_Load_Char, error: %d", error);
        return -1;
    }

    draw_bitmap(&slot->bitmap, x, y);

    return 0;
}

int32_t freetype_draw_char_angle(int32_t x, int32_t y, int32_t angle, const wchar_t *str)
{
    if (str == NULL || !lcd_is_init())
        return -1;

    double radian = (1.0 * angle / 360) * 3.14159 * 2;

    FT_Matrix matrix;
    matrix.xx = (FT_Fixed)( cos( radian ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( radian ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( radian ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( radian ) * 0x10000L );

    FT_Set_Transform(face, &matrix, NULL);

    error = FT_Load_Char(face, str[0], FT_LOAD_RENDER);
    if (error != 0)
    {
        printf("failed to FT_Load_Char, error: %d", error);
        return -1;
    }

    draw_bitmap(&slot->bitmap, x, y);

    return 0;
}
