/* 
 * Tux Racer 
 * Copyright (C) 1999-2001 Jasmin F. Patry
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef _FONTS_H_
#define _FONTS_H_

typedef struct font_ font_t;

void init_fonts();
void set_font_color(font_t* font, colour_t c);
void set_font_size(font_t* font, scalar_t size);
scalar_t get_font_size(font_t* font);

bool_t load_font( const char *fontname, const char *filename, const char *texname );

bool_t bind_font( const char *binding, const char *fontname, scalar_t size, 
		  colour_t colour );
bool_t get_font_binding( const char *binding, font_t **font );
bool_t unbind_font( const char *binding );

bool_t flush_fonts(void);

void bind_font_texture( font_t *font );
//void draw_character( font_t *font, char c);
void draw_string( font_t *font, const char *string);
void draw_string_with_markup( font_t *font, const char *string);
void get_font_metrics( font_t *font, const char *string,
		       int *width, int *max_ascent, int *max_descent);
void get_font_metrics_scalar( font_t *font, const char *string,
		       scalar_t *width, scalar_t *max_ascent, scalar_t *max_descent);
void register_font_callbacks(Tcl_Interp *ip);

#endif /* _FONTS_H_ */

#ifdef __cplusplus
} /* extern "C" */
#endif
