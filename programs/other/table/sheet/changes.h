#ifndef SHEET_CHANGES_H
#define SHEET_CHANGES_H

// The changes of ranges of cells, all undoable: fill, erase, the
// clipboard, inserting and deleting columns and rows; the formulas follow
// the cells they move with.

// the source rectangle x0..x1, y0..y1 repeated over the target one
// tx0..tx1, ty0..ty1 in all directions, its formulas shifted as they move;
// the source cells outside the target emptied
void table_fill(int x0, int y0, int x1, int y1, int tx0, int ty0, int tx1, int ty1);
void table_erase(int x0, int y0, int x1, int y1); // the texts away

// the clipboard: copy (or cut) a rectangle, paste it with x, y at its top left
void clip_copy(int x0, int y0, int x1, int y1, int cut);
void clip_paste(int x, int y);
int clip_cells(void); // how many cells it holds

// n columns (cols) or rows inserted before column or row at, the ones
// after it moved on; or, with n < 0, -n of them deleted from at on. An
// insertion pushes the last n out of the table: table_room() gives 1 if
// they are empty, so that no text is lost (a reference into them becomes
// #REF!). The caller makes sure of it, and of at + n - 1 in the table
int table_room(int cols, int n);
void table_insert(int cols, int at, int n);

#endif
