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

#include "tex_font_metrics.h"
#include "primitive_draw.h"
#include "shaders.h"
#include "tuxracer.h"

#define MAX_TEX_FONT_CHARS 256

static GLfloat string_vertices[MAX_TEX_FONT_CHARS * 18];
static GLfloat string_texcoords[MAX_TEX_FONT_CHARS * 12];

static int num_batched_chars;

struct tex_font_metrics_ {
  int max_ascent;
  int max_descent;
  tfm_char_data_t *char_data[MAX_TEX_FONT_CHARS];
};

struct char_dims {
  unsigned short ch;
  unsigned char w;
  unsigned char h;
  signed char x_offset;
  signed char y_offset;
  signed char kern_width;
  short x_pixel;
  short y_pixel;
};

#define READ_BYTES(file, addr, bytes, swap)                                    \
  if (SDL_RWread(file, addr, 1, bytes) == 0) {                                 \
    err_msg = "Unexpected end of file";                                        \
    goto bail;                                                                 \
  } else {                                                                     \
    if ((swap)) {                                                              \
      if ((bytes) == 4) {                                                      \
        SWAP_WORD(*(addr));                                                    \
      } else if (bytes == 2) {                                                 \
        SWAP_SHORT(*(addr));                                                   \
      } else {                                                                 \
        check_assertion(0,                                                     \
                        "Can't byte-swap storage with size != 2 of 4 bytes");  \
      }                                                                        \
    }                                                                          \
  }

tex_font_metrics_t *load_tex_font_metrics(const char *filename) {
  tex_font_metrics_t *tfm = NULL;
  SDL_RWops *tfm_file = NULL;
  int i;
  char magic[4];
  char *err_msg;
  int endian_check;
  bool_t swap_bytes;
  struct char_dims ch_dims;
  int num_chars;
  int texture_width, texture_height;
  char dummy;

  check_assertion(sizeof(int) == 4, "This architecture's integer size is != 4");
  check_assertion(sizeof(short) == 2,
                  "This architecture's short integer size is != 2");
  check_assertion(sizeof(char) == 1, "This architecture's char size is != 1");

  /* Open file */
  tfm_file = SDL_RWFromFile(filename, "rb");
  if (tfm_file == NULL) {
    print_warning(MISSING_FILE_WARNING, "Couldn't open font metrics file %s",
                  filename);
    return NULL;
  }

  tfm = (tex_font_metrics_t *)malloc(sizeof(tex_font_metrics_t));
  check_assertion(tfm != NULL, "out of memory");

  /* Initialize tfm */
  for (i = 0; i < MAX_TEX_FONT_CHARS; i++) {
    tfm->char_data[i] = NULL;
  }

  /* Check magic number */
  READ_BYTES(tfm_file, magic, sizeof(magic), False);

  if (strncmp(magic, "\377tfm", 4) != 0) {
    err_msg = "File is not a valid tfm file";
    goto bail;
  }

  /* Check endian-ness */
  READ_BYTES(tfm_file, &endian_check, sizeof(int), False);

  if (endian_check == 0x12345678) {
    swap_bytes = False;
  } else if (endian_check == 0x78563412) {
    swap_bytes = True;
  } else {
    err_msg = "File is not a valid tfm file";
    goto bail;
  }

  // print_debug(DEBUG_OTHER, "swap bytes: %d", swap_bytes);

  /* Read in texture_width, texture_height, max_ascent, max_descent */
  READ_BYTES(tfm_file, &texture_width, sizeof(int), swap_bytes);
  READ_BYTES(tfm_file, &texture_height, sizeof(int), swap_bytes);
  READ_BYTES(tfm_file, &tfm->max_ascent, sizeof(int), swap_bytes);
  READ_BYTES(tfm_file, &tfm->max_descent, sizeof(int), swap_bytes);

  READ_BYTES(tfm_file, &num_chars, sizeof(int), swap_bytes);

  for (i = 0; i < num_chars; i++) {
    tfm_char_data_t *cd;
    scalar_t sstep = 0.5 / texture_width;
    scalar_t tstep = 0.5 / texture_height;

    READ_BYTES(tfm_file, &ch_dims.ch, sizeof(unsigned short), swap_bytes);
    READ_BYTES(tfm_file, &ch_dims.w, sizeof(unsigned char), False);
    READ_BYTES(tfm_file, &ch_dims.h, sizeof(unsigned char), False);
    READ_BYTES(tfm_file, &ch_dims.x_offset, sizeof(char), False);
    READ_BYTES(tfm_file, &ch_dims.y_offset, sizeof(char), False);
    READ_BYTES(tfm_file, &ch_dims.kern_width, sizeof(char), False);
    READ_BYTES(tfm_file, &dummy, sizeof(char), False);
    READ_BYTES(tfm_file, &ch_dims.x_pixel, sizeof(short), swap_bytes);
    READ_BYTES(tfm_file, &ch_dims.y_pixel, sizeof(short), swap_bytes);

    if (ch_dims.ch >= MAX_TEX_FONT_CHARS) {
      err_msg = "Two-byte characters are not supported";
      goto bail;
    }

    cd = (tfm_char_data_t *)malloc(sizeof(tfm_char_data_t));

    check_assertion(cd != NULL, "out of memory");

    cd->ll = make_point2d(ch_dims.x_offset, ch_dims.y_offset);
    cd->lr = make_point2d(cd->ll.x + ch_dims.w, cd->ll.y);
    cd->ur = make_point2d(cd->lr.x, cd->lr.y + ch_dims.h);
    cd->ul = make_point2d(cd->ur.x - ch_dims.w, cd->ur.y);

    cd->tex_ll =
        make_point2d(ch_dims.x_pixel / (scalar_t)texture_width + sstep,
                     ch_dims.y_pixel / (scalar_t)texture_height + tstep);
    cd->tex_lr =
        make_point2d(cd->tex_ll.x + sstep + ch_dims.w / (scalar_t)texture_width,
                     cd->tex_ll.y + tstep);
    cd->tex_ur = make_point2d(cd->tex_lr.x + sstep,
                              cd->tex_lr.y + tstep +
                                  ch_dims.h / (scalar_t)texture_height);
    cd->tex_ul =
        make_point2d(cd->tex_ur.x + sstep - ch_dims.w / (scalar_t)texture_width,
                     cd->tex_ur.y + tstep);

    cd->kern_width = ch_dims.kern_width;

    tfm->char_data[ch_dims.ch] = cd;

    // print_debug(DEBUG_OTHER, "character: %c w: %d h: %d x_offset: %d
    // y_offset: %d kern_width: %d y_pixel: %d x_pixel: %d", ch_dims.ch,
    // ch_dims.w, ch_dims.h, ch_dims.x_offset, ch_dims.y_offset,
    // ch_dims.kern_width, 	ch_dims.y_pixel, ch_dims.x_pixel);
  }

  SDL_RWclose(tfm_file);

  return tfm;

bail:
  if (tfm != NULL) {
    for (i = 0; i < MAX_TEX_FONT_CHARS; i++) {
      if (tfm->char_data[i] != NULL) {
        free(tfm->char_data[i]);
      }
    }
    free(tfm);
  }

  if (tfm_file != NULL) {
    SDL_RWclose(tfm_file);
  }

  print_warning(IMPORTANT_WARNING, "Error opening font metrics file `%s': %s\n",
                filename, err_msg);
  return NULL;
}

void delete_tex_font_metrics(tex_font_metrics_t *tfm) {
  int i;
  if (tfm != NULL) {
    for (i = 0; i < MAX_TEX_FONT_CHARS; i++) {
      if (tfm->char_data[i] != NULL) {
        free(tfm->char_data[i]);
      }
    }
    free(tfm);
  }
}

static tfm_char_data_t *find_char_data(tex_font_metrics_t *tfm, char c) {
  int i;
  if (tfm->char_data[(int)c] != NULL) {
    return tfm->char_data[(int)c];
  } else if (isupper(c) && tfm->char_data[tolower(c)] != NULL) {
    return tfm->char_data[tolower(c)];
  } else if (islower(c) && tfm->char_data[toupper(c)] != NULL) {
    return tfm->char_data[toupper(c)];
  } else if (tfm->char_data[' '] != NULL) {
    print_warning(IMPORTANT_WARNING,
                  "Font does not have a representation of "
                  "character `%c'; using space as placeholder",
                  c);
    return tfm->char_data[' '];
  } else {
    for (i = 0; i < MAX_TEX_FONT_CHARS; i++) {
      if (tfm->char_data[i] != NULL) {
        print_warning(IMPORTANT_WARNING,
                      "Font does not have a representation of "
                      "character `%c'; using `%c' as placeholder",
                      c, i);
        return tfm->char_data[i];
      }
    }
    check_assertion(0, "font contains no characters");
  }

  /* Shouldn't get here */
  return NULL;
}

void get_tex_font_string_bbox(tex_font_metrics_t *tfm, const char *string,
                              int *width, int *max_ascent, int *max_descent) {
  int i;
  int len;
  tfm_char_data_t *cd;
  *width = 0;

  len = strlen(string);

  for (i = 0; i < len; i++) {
    cd = find_char_data(tfm, string[i]);
    *width += cd->kern_width;
  }

  *max_ascent = tfm->max_ascent;
  *max_descent = tfm->max_descent;
}

float batch_tex_font_char(tfm_char_data_t *cd, char c, float x, float y,
                          float scale) {
#define TO_RELATIVE(screen, val) (((val) / screen * 2.0) - 1.0)
  float screen_w = getparam_x_resolution();
  float screen_h = getparam_y_resolution();

  int i;

  GLfloat texcoords[] = {cd->tex_ll.x, cd->tex_ll.y, cd->tex_ul.x,
                         cd->tex_ul.y, cd->tex_ur.x, cd->tex_ur.y,
                         cd->tex_ur.x, cd->tex_ur.y, cd->tex_lr.x,
                         cd->tex_lr.y, cd->tex_ll.x, cd->tex_ll.y};

  GLfloat vertices[] = {
      scale * cd->ll.x + x, scale * cd->ll.y + y, scale * cd->ul.x + x,
      scale * cd->ul.y + y, scale * cd->ur.x + x, scale * cd->ur.y + y,
      scale * cd->ur.x + x, scale * cd->ur.y + y, scale * cd->lr.x + x,
      scale * cd->lr.y + y, scale * cd->ll.x + x, scale * cd->ll.y + y,
  };

  for (i = 0; i < 6; i++) {
    string_vertices[num_batched_chars * 12 + i * 2] =
        TO_RELATIVE(screen_w, vertices[i * 2]);
    string_vertices[num_batched_chars * 12 + i * 2 + 1] =
        TO_RELATIVE(screen_h, vertices[i * 2 + 1]);
  }

  for (i = 0; i < 12; i++) {
    string_texcoords[num_batched_chars * 12 + i] = texcoords[i];
  }

  num_batched_chars++;

  return scale * cd->kern_width;
}

void draw_tex_font_string(tex_font_metrics_t *tfm, const char *string, float x,
                          float y, float scale) {
  int i;
  int len;
  tfm_char_data_t *cd;

  len = strlen(string);

  num_batched_chars = 0;

  for (i = 0; i < len; i++) {
    cd = find_char_data(tfm, string[i]);
    x += batch_tex_font_char(cd, string[i], x, y, scale);
  }

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glVertexAttribPointer(shader_get_attrib_location(SHADER_VERTEX_NAME), 2,
                        GL_FLOAT, GL_FALSE, 0, string_vertices);
  glEnableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));

  glVertexAttribPointer(shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME),
                        2, GL_FLOAT, GL_FALSE, 0, string_texcoords);
  glEnableVertexAttribArray(
      shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));

  glDrawArrays(GL_TRIANGLES, 0, num_batched_chars * 6);

  glDisableVertexAttribArray(shader_get_attrib_location(SHADER_VERTEX_NAME));
  glDisableVertexAttribArray(
      shader_get_attrib_location(SHADER_TEXTURE_COORD_NAME));
}

bool_t is_character_in_tex_font(tex_font_metrics_t *tfm, char c) {
  return (bool_t)(tfm->char_data[(int)c] != NULL);
}

/* EOF */
