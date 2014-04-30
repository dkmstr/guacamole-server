/*
 * Copyright (C) 2014 Glyptodon LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "config.h"
#include "guac_surface.h"

#include <cairo/cairo.h>
#include <guacamole/layer.h>
#include <guacamole/protocol.h>
#include <guacamole/socket.h>

#include <stdlib.h>

static void __guac_common_mark_dirty(guac_common_surface* surface, int x, int y, int w, int h) {

    /* If already dirty, update existing rect */
    if (surface->dirty) {

        /* Calculate extents of existing dirty rect */
        int dirty_left   = surface->dirty_x;
        int dirty_top    = surface->dirty_y;
        int dirty_right  = dirty_left + surface->dirty_width;
        int dirty_bottom = dirty_top  + surface->dirty_height;

        /* Calculate missing extents of given new rect */
        int right  = x + w;
        int bottom = y + h;

        /* Update minimums */
        if (x      < dirty_left)   dirty_left   = x;
        if (y      < dirty_top)    dirty_top    = y;
        if (right  > dirty_right)  dirty_right  = right;
        if (bottom > dirty_bottom) dirty_bottom = bottom;

        /* Commit rect */
        surface->dirty_x      = dirty_left;
        surface->dirty_y      = dirty_top;
        surface->dirty_width  = dirty_right  - dirty_left;
        surface->dirty_height = dirty_bottom - dirty_top;

    }

    /* Otherwise init dirty rect */
    else {
        surface->dirty_x = x;
        surface->dirty_y = y;
        surface->dirty_width = w;
        surface->dirty_height = h;
        surface->dirty = 1;
    }

}

guac_common_surface* guac_common_surface_alloc(guac_socket* socket, const guac_layer* layer, int w, int h) {

    /* Init surface */
    guac_common_surface* surface = malloc(sizeof(guac_common_surface));
    surface->layer = layer;
    surface->socket = socket;
    surface->width = w;
    surface->height = h;
    surface->dirty = 0;

    /* Create corresponding Cairo surface */
    surface->stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24, w);
    surface->buffer = malloc(surface->stride * h);
    surface->surface = cairo_image_surface_create_for_data(surface->buffer, CAIRO_FORMAT_RGB24,
                                                           w, h, surface->stride);

    guac_protocol_send_size(socket, layer, w, h);
    return surface;
}

void guac_common_surface_free(guac_common_surface* surface) {
    guac_protocol_send_dispose(surface->socket, surface->layer);
    cairo_surface_destroy(surface->surface);
    free(surface->buffer);
    free(surface);
}

void guac_common_surface_draw(guac_common_surface* surface, int x, int y, cairo_surface_t* src) {

    guac_socket* socket = surface->socket;
    const guac_layer* layer = surface->layer;

    /* STUB */
    __guac_common_mark_dirty(surface, x, y,
                             cairo_image_surface_get_width(src),
                             cairo_image_surface_get_height(src));
    guac_protocol_send_png(socket, GUAC_COMP_OVER, layer, x, y, src);

}

void guac_common_surface_copy(guac_common_surface* src, int sx, int sy, int w, int h,
                              guac_common_surface* dst, int dx, int dy) {

    guac_socket* socket = dst->socket;
    const guac_layer* src_layer = src->layer;
    const guac_layer* dst_layer = dst->layer;

    /* STUB */
    __guac_common_mark_dirty(dst, dx, dy, w, h);
    guac_protocol_send_copy(socket, src_layer, sx, sy, w, h, GUAC_COMP_OVER, dst_layer, dx, dy);

}

void guac_common_surface_transfer(guac_common_surface* src, int sx, int sy, int w, int h,
                                  guac_transfer_function op, guac_common_surface* dst, int dx, int dy) {

    guac_socket* socket = dst->socket;
    const guac_layer* src_layer = src->layer;
    const guac_layer* dst_layer = dst->layer;

    /* STUB */
    __guac_common_mark_dirty(dst, dx, dy, w, h);
    guac_protocol_send_transfer(socket, src_layer, sx, sy, w, h, op, dst_layer, dx, dy);

}

void guac_common_surface_rect(guac_common_surface* surface,
                              int x, int y, int w, int h,
                              int red, int green, int blue) {

    guac_socket* socket = surface->socket;
    const guac_layer* layer = surface->layer;

    /* STUB */
    __guac_common_mark_dirty(surface, x, y, w, h);
    guac_protocol_send_rect(socket, layer, x, y, w, h);
    guac_protocol_send_cfill(socket, GUAC_COMP_OVER, layer, red, green, blue, 0xFF);

}

void guac_common_surface_flush(guac_common_surface* surface) {

    /* STUB */
    if (surface->dirty) {
        guac_socket* socket = surface->socket;
        guac_protocol_send_log(socket, "Flushing surface %i: (%i, %i) %ix%i",
                               surface->layer->index,
                               surface->dirty_x, surface->dirty_y,
                               surface->dirty_width, surface->dirty_height);
        surface->dirty = 0;
    }

}

