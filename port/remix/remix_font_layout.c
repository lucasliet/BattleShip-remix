/**
 * remix_font_layout.c - see remix_font_layout.h.
 */

#include "remix_font_layout.h"

#define SPACE_CHARACTER 0x20

static int hasGlyph(int character, int max_character)
{
    return (character > SPACE_CHARACTER) && (character <= max_character);
}

float remix_font_layout_advance(int character, const uint8_t *widths, int max_character, float scale)
{
    if (!hasGlyph(character, max_character))
    {
        return REMIX_FONT_LAYOUT_SPACE_ADVANCE * scale;
    }
    return ((float)widths[character] + 1.0f) * scale;
}

float remix_font_layout_aligned_width(const char *text, const uint8_t *widths, int max_character, float scale)
{
    float width = 0.0f;

    for (; *text != 0; text++)
    {
        width += remix_font_layout_advance((unsigned char)*text, widths, max_character, scale);
    }
    return width;
}

float remix_font_layout_fit_width(const char *text, const uint8_t *widths, int max_character, float scale)
{
    float cursor = 0.0f;
    float first_x = 0.0f;
    float last_x = 0.0f;
    int has_glyph = 0;

    for (; *text != 0; text++)
    {
        int character = (unsigned char)*text;

        if (hasGlyph(character, max_character))
        {
            if (!has_glyph)
            {
                first_x = cursor;
                has_glyph = 1;
            }
            last_x = cursor;
        }
        cursor += remix_font_layout_advance(character, widths, max_character, scale);
    }
    if (!has_glyph)
    {
        return 0.0f;
    }
    return last_x + REMIX_FONT_LAYOUT_SPRITE_WIDTH * scale - first_x;
}

float remix_font_layout_fit_factor(const char *text, const uint8_t *widths, int max_character, float scale, float max_width)
{
    float width = remix_font_layout_fit_width(text, widths, max_character, scale);

    if ((max_width <= 0.0f) || (width <= max_width))
    {
        return 1.0f;
    }
    return max_width / width;
}
