#include <kernel/ansi.h>
#include <stddef.h>
#include <stdint.h>

#define ESC '\x1B'

struct ansi_state ansi_init(void)
{
    struct ansi_state rv = {
        .state = ANSI_ESC,
        .style = 0x0F,
        .next_style = 0x0F};
    return rv;
}

static inline uint8_t ansi_convert_color(uint8_t color)
{
    const uint8_t lookup_table[8] = {0, 4, 2, 6, 1, 5, 3, 7};
    return lookup_table[(int)color];
}

struct color_char ansi_process(struct ansi_state *state, char x)
{
    struct color_char rv = {
        .style = state->style,
        .ascii = '\0'};
    switch (state->state)
    {
    case ANSI_ESC:
        if (x == ESC)
            state->state = ANSI_BRACKET;
        else
        {
            rv.ascii = x;
        }
        break;
    case ANSI_BRACKET:
        if (x == '[')
            state->state = ANSI_PARSE;
        else
        {
            state->state = ANSI_ESC;
            rv.ascii = x;
        }
        break;
    case ANSI_PARSE:
        if (x == '3')
        {
            state->state = ANSI_FGCOLOR;
        }
        else if (x == '4')
        {
            state->state = ANSI_BGCOLOR;
        }
        else if (x == '0')
        {
            state->state = ANSI_ENDVAL;
            state->next_style = 0x0F;
        }
        else if (x == '1')
        {
            state->state = ANSI_ENDVAL;
            state->next_style |= (1 << 3);
        }
        else if (x == '=')
        {
            state->state = ANSI_EQUALS;
        }
        else
        {
            state->state = ANSI_ESC;
            state->next_style = state->style;
            rv.ascii = x;
        }
        break;
    case ANSI_BGCOLOR:
        if (x >= '0' && x <= '7')
        {
            state->state = ANSI_ENDVAL;
            state->next_style &= 0x1F;
            state->next_style |= ansi_convert_color(x - '0') << 4;
        }
        else
        {
            state->state = ANSI_ESC;
            state->next_style = state->style;
            rv.ascii = x;
        }
        break;
    case ANSI_FGCOLOR:
        if (x >= '0' && x <= '7')
        {
            state->state = ANSI_ENDVAL;
            state->next_style &= 0xF8;
            state->next_style |= ansi_convert_color(x - '0');
        }
        else
        {
            state->state = ANSI_ESC;
            state->next_style = state->style;
            rv.ascii = x;
        }
        break;
    case ANSI_EQUALS:
        if (x == '1')
        {
            state->state = ANSI_ENDVAL;
            state->next_style &= ~(1 << 3);
        }
        else
        {
            state->state = ANSI_ESC;
            state->next_style = state->style;
            rv.ascii = x;
        }
        break;
    case ANSI_ENDVAL:
        if (x == ';')
        {
            state->state = ANSI_PARSE;
        }
        else if (x == 'm')
        {
            // Finish and apply styles
            state->state = ANSI_ESC;
            state->style = state->next_style;
        }
        else
        {
            state->state = ANSI_ESC;
            state->next_style = state->style;
            rv.ascii = x;
        }
        break;
    default:
        break;
    }
    return rv;
}
