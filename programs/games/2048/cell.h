#ifndef CELL_H
#define CELL_H

#include "defines.h"
#include "paint.h"

typedef struct {
    rect    cell;       // current rect
    __u32   value;      // value, 0 - do not draw a tile
    __u8    animate;    // animation needed: true or false
    __u16   ani_step;   // step for animation
    rect    transition; // destination rect for animation
    __u8    merged;     // merge flag
    rect    merged_rect;// rect for drawing merged tile
} tile;

// Draw a tile (animation will started if needed)
__u8 tile_draw(tile* t);

// Check two tiles for merging
__u8 tile_mergeable(tile* from, tile* to);

#endif // CELL_H
