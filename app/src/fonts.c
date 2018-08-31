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

#include "fonts.h"
#include "hash.h"
#include "image.h"
#include "list.h"
#include "render_util.h"
#include "tcl_util.h"
#include "tex_font_metrics.h"
#include "textures.h"
#include "tuxracer.h"

typedef struct {
  tex_font_metrics_t *tfm;
  texture_node_t *tex;
  int ref_count;
} font_node_t;

struct font_ {
  font_node_t *node;
  scalar_t size;
  colour_t colour;
};

static bool_t initialized = False;
static hash_table_t font_table;
static hash_table_t binding_table;

void init_fonts() {
  if (!initialized) {
    font_table = create_hash_table();
    binding_table = create_hash_table();
  }
  initialized = True;
}

static bool_t get_font(const char *fontname, font_node_t **fontnode) {
  return get_hash_entry(font_table, fontname, (hash_entry_t *)fontnode);
}

void set_font_color(font_t *font, colour_t c) { font->colour = c; }

void set_font_size(font_t *font, scalar_t s) { font->size = s; }

bool_t get_font_binding(const char *binding, font_t **font) {
  if (get_hash_entry(binding_table, binding, (hash_entry_t *)(font))) {
    return True;
  }
  return False;
}

bool_t load_font(const char *fontname, const char *filename,
                 const char *texname) {
  font_node_t *fontnode;
  tex_font_metrics_t *tfm;
  texture_node_t *tex;

  print_debug(DEBUG_FONT, "Loading font %s from file: %s", fontname, filename);
  if (initialized == False) {
    check_assertion(0, "font module not initialized");
  }

  if (!get_texture(texname, &tex)) {
    print_warning(IMPORTANT_WARNING, "Texture `%s' does not exist", texname);
    return False;
  }

  tfm = load_tex_font_metrics(filename);

  if (tfm == NULL) {
    print_warning(IMPORTANT_WARNING, "couldn't load font file %s", filename);
    return False;
  }

  if (get_hash_entry(font_table, fontname, (hash_entry_t *)&fontnode)) {
    print_debug(DEBUG_FONT, "Font %s already exists, deleting...", fontname);
    delete_tex_font_metrics(fontnode->tfm);
    fontnode->tex->ref_count -= 1;
  } else {
    fontnode = (font_node_t *)malloc(sizeof(font_node_t));
    fontnode->ref_count = 0;
    add_hash_entry(font_table, fontname, (hash_entry_t)fontnode);
  }

  fontnode->tfm = tfm;
  fontnode->tex = tex;
  tex->ref_count += 1;

  return True;
}

static bool_t del_font(char *fontname) {
  font_node_t *fontnode;
  if (del_hash_entry(font_table, fontname, (hash_entry_t *)(&fontnode))) {
    check_assertion(fontnode->ref_count == 0,
                    "Trying to delete font with non-zero reference "
                    "count");
    delete_tex_font_metrics(fontnode->tfm);
    fontnode->tex->ref_count -= 1;
    free(fontnode);
    return True;
  }

  return False;
}

bool_t bind_font(const char *binding, const char *fontname, scalar_t size,
                 colour_t colour) {
  font_node_t *fontnode;
  font_t *font;

  print_debug(DEBUG_FONT, "Binding %s to font name: %s", binding, fontname);

  if (!get_font(fontname, &fontnode)) {
    check_assertion(0, "Attempt to bind to unloaded font");
    return False;
  }

  if (get_hash_entry(binding_table, binding, (hash_entry_t *)(&font))) {
    font->node->ref_count--;
  } else {
    font = (font_t *)malloc(sizeof(font_t));
    add_hash_entry(binding_table, binding, (hash_entry_t)font);
  }

  font->node = fontnode;
  font->size = size;
  font->colour = colour;

  font->node->ref_count += 1;

  return True;
}

bool_t unbind_font(const char *binding) {
  font_t *font;

  if (get_hash_entry(binding_table, binding, (hash_entry_t *)(&font))) {
    font->node->ref_count -= 1;
    if (!del_hash_entry(binding_table, binding, NULL)) {
      check_assertion(0, "Cannot delete known font binding");
      return False;
    }
    free(font);
    return True;
  }

  return False;
}

bool_t flush_fonts(void) {
  font_node_t *fontnode;
  hash_search_t sptr;
  list_t delete_list;
  list_elem_t elem;
  char *key;
  bool_t result;

  delete_list = create_list();

  begin_hash_scan(font_table, &sptr);
  while (next_hash_entry(sptr, &key, (hash_entry_t *)(&fontnode))) {
    if (fontnode->ref_count == 0) {
      elem = insert_list_elem(delete_list, NULL, (list_elem_data_t)key);
    }
  }
  end_hash_scan(sptr);

  elem = get_list_head(delete_list);
  while (elem != NULL) {
    key = (char *)get_list_elem_data(elem);

    result = del_font(key);
    check_assertion(result, "Attempt to flush non-existant font");

    elem = get_next_list_elem(delete_list, elem);
  }

  del_list(delete_list);

  return True;
}

static scalar_t get_scale_factor(font_t *font) {
  char *empty_string = "";
  int dummy1, dummy2, max_ascent;

  get_tex_font_string_bbox(font->node->tfm, empty_string, &dummy1, &max_ascent,
                           &dummy2);

  return winsys_scale(font->size) / max_ascent;
}

void bind_font_texture(font_t *font) {
  glBindTexture(GL_TEXTURE_2D, font->node->tex->texture_id);
}

void draw_string(font_t *font, const char *string, float x, float y) {
  float argb[] = {(float)font->colour.r, (float)font->colour.g,
                  (float)font->colour.b, (float)font->colour.a};
  shader_set_color(argb);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  draw_tex_font_string(font->node->tfm, string, x, y, get_scale_factor(font));
}

void get_font_metrics(font_t *font, const char *string, int *width,
                      int *max_ascent, int *max_descent) {
  scalar_t scale_fact = get_scale_factor(font);
  get_tex_font_string_bbox(font->node->tfm, string, width, max_ascent,
                           max_descent);

  *width *= scale_fact;
  *max_ascent *= scale_fact;
  *max_descent *= scale_fact;
}

void get_font_metrics_scalar(font_t *font, const char *string, scalar_t *width,
                             scalar_t *max_ascent, scalar_t *max_descent) {
  scalar_t scale_fact = get_scale_factor(font);
  int w, a, d;
  get_tex_font_string_bbox(font->node->tfm, string, &w, &a, &d);

  *width = w * scale_fact;
  *max_ascent = a * scale_fact;
  *max_descent = d * scale_fact;
}

static int load_font_cb(ClientData cd, Tcl_Interp *ip, int argc,
                        const char *argv[]) {
  bool_t error = False;

  const char *fontname = NULL;
  const char *filename = NULL;
  const char *texturename = NULL;

  if (argc < 2) {
    error = True;
  }

  NEXT_ARG;

  while (!error && argc > 0) {

    if (strcmp("-name", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      fontname = *argv;
    } else if (strcmp("-file", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      filename = *argv;
    } else if (strcmp("-texture", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      texturename = *argv;
    } else {
      print_warning(TCL_WARNING,
                    "tux_load_font: unrecognized "
                    "parameter `%s'",
                    *argv);
    }

    NEXT_ARG;
  }

  /* Sanity check on values */
  if (fontname == NULL || filename == NULL || texturename == NULL) {
    error = True;
  }

  if (error) {
    print_warning(TCL_WARNING, "error in call to tux_load_font");
    Tcl_AppendResult(ip,
                     "\nUsage: tux_load_font -name <name> -file <file> "
                     "-texture <name> ",
                     (char *)0);
    return TCL_ERROR;
  }

  if (!load_font(fontname, filename, texturename)) {
    print_warning(TCL_WARNING, "Could not load font %s", filename);
    return TCL_ERROR;
  }

  return TCL_OK;
}

static int bind_font_cb(ClientData cd, Tcl_Interp *ip, int argc,
                        const char *argv[]) {
  double tmp_dbl;
  bool_t error = False;

  const char *binding = NULL;
  const char *fontname = NULL;
  colour_t colour = white;
  int size = 30;

  if (argc < 2) {
    error = True;
  }

  NEXT_ARG;

  while (!error && argc > 0) {

    if (strcmp("-binding", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      binding = *argv;
    } else if (strcmp("-font", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      fontname = *argv;
    } else if (strcmp("-size", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      if (Tcl_GetDouble(ip, *argv, &tmp_dbl) == TCL_ERROR) {
        error = True;
        break;
      }
      size = tmp_dbl;
    } else if (strcmp("-colour", *argv) == 0 || strcmp("-color", *argv) == 0) {
      NEXT_ARG;
      if (argc == 0) {
        error = True;
        break;
      }
      if (get_tcl_tuple(ip, *argv, (scalar_t *)&colour, 4) == TCL_ERROR) {
        error = True;
        break;
      }
    } else {
      print_warning(TCL_WARNING,
                    "tux_bind_font: unrecognized "
                    "parameter `%s'",
                    *argv);
    }

    NEXT_ARG;
  }

  /* Sanity check on values */
  if (binding == NULL || fontname == NULL || size < 0) {
    error = True;
  }

  if (error) {
    print_warning(TCL_WARNING, "error in call to tux_bind_font");
    Tcl_AppendResult(ip,
                     "\nUsage: tux_bind_font -binding <name> -font <name> "
                     "[-size <height>] ",
                     (char *)0);
    return TCL_ERROR;
  }

  if (!bind_font(binding, fontname, size, colour)) {
    print_warning(TCL_WARNING, "Could not bind font %s", binding);
    return TCL_ERROR;
  }

  return TCL_OK;
}

void register_font_callbacks(Tcl_Interp *ip) {
  Tcl_CreateCommand(ip, "tux_load_font", load_font_cb, 0, 0);
  Tcl_CreateCommand(ip, "tux_bind_font", bind_font_cb, 0, 0);
}
