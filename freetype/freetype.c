#include "freetype.h"
#include "lcd.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include <assert.h>
#include <math.h>
#include <wchar.h>

#define INIT_FONT_SIZE 24

static FT_Library library;
static FT_Face face;
static FT_Error error;
static FT_GlyphSlot slot;

static int32_t max(int32_t a, int32_t b) {
    return (a > b) ? a : b;
}

static int32_t min(int32_t a, int32_t b) {
    return (a < b) ? a : b;
}

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

static FT_Matrix angle_maxtrix(int32_t angle)
{
    FT_Matrix matrix;

    double radian = (1.0 * angle / 360) * 3.14159 * 2;

    matrix.xx = (FT_Fixed)( cos( radian ) * 0x10000L );
	matrix.xy = (FT_Fixed)(-sin( radian ) * 0x10000L );
	matrix.yx = (FT_Fixed)( sin( radian ) * 0x10000L );
	matrix.yy = (FT_Fixed)( cos( radian ) * 0x10000L );
    
    return matrix;
}

int32_t freetype_draw_char_angle(int32_t x, int32_t y, int32_t angle, const wchar_t *str)
{
    if (str == NULL || !lcd_is_init())
        return -1;

    FT_Matrix matrix = angle_maxtrix(angle);

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

static int32_t compute_string_bbox(FT_Face face, const wchar_t* wstr, FT_BBox* pbbox, FT_Matrix* pmatrix)
{
    assert(face != NULL);
    assert(wstr != NULL);
    assert(pbbox != NULL);

    FT_Error error;
    FT_BBox bbox;
    FT_BBox glyph_bbox;
    FT_Vector pen;
    FT_Glyph glyph;
    FT_GlyphSlot slot = face->glyph;

    /* 初始化 */
    bbox.xMin = bbox.yMin = 32000;
    bbox.xMax = bbox.yMax = -32000;

    /* 指定原点(0, 0) */   
    pen.x = 0;
    pen.y = 0;

    /* 计算每个字符的bounding box */
    /* 先translate, 再load char, 就可以得到它的外框了 */
    for(int32_t i = 0; i < wcslen(wstr); ++i)
    {
        /* 转换：transformation */
        FT_Set_Transform(face, pmatrix, &pen);

        /* 加载位图：load glyph image into the slot (erase previous one)*/
        error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
        if (error != 0)
        {
            printf("FT_Load_Char, error:%d\n", error);
            return -1;
        }

        /* 取出glyph */
        error = FT_Get_Glyph(face->glyph, &glyph);
        if (error != 0) 
        {
            printf("FT_Get_Glyph error:%d\n", error);
            return -1;
        }

        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &glyph_bbox);

        /* 更新外框 */
        bbox.xMin = min(glyph_bbox.xMin, bbox.xMin);
        bbox.yMin = min(glyph_bbox.yMin, bbox.yMin);
        bbox.xMax = max(glyph_bbox.xMax, bbox.xMax);
        bbox.yMax = max(glyph_bbox.yMax, bbox.yMax);

        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }

    *pbbox = bbox;

    return 0;
}

int32_t free_type_display_string(int32_t lcd_x, int32_t lcd_y, int32_t angle, const wchar_t* wstr)
{
    FT_BBox bbox = {};
    FT_Vector delta;
    int32_t x = lcd_x;
    int32_t lcd_h = lcd_height();
    int32_t y = lcd_h - lcd_y;
    FT_Matrix matrix = angle_maxtrix(angle);

    compute_string_bbox(face, wstr, &bbox, &matrix);

    /* 反推原点 */
    delta.x = (x - bbox.xMin) * 64; /* freetype单位: 1/64像素 */
    delta.y = (y - bbox.yMax) * 64;

    /* 处理每个字符 */
    for(int32_t i = 0; i < wcslen(wstr); ++i)
    {
        FT_Set_Transform(face, &matrix, &delta);
        
        error = FT_Load_Char(face, wstr[i], FT_LOAD_RENDER);
        if (error != 0)
        {
            printf("FT_Load_Char, error:%d\n", error);
            return -1;
        }

        draw_bitmap(&slot->bitmap, slot->bitmap_left, lcd_h - slot->bitmap_top);

        /* 计算下一个字符的原点: increment pen position */
        delta.x += slot->advance.x;
        delta.y += slot->advance.y;
    }

    return 0;
}