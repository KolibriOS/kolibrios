#ifndef SHEET_UNDO_H
#define SHEET_UNDO_H

// undo: a change saves its cells x0..x1, y0..y1, or the size it changes,
// or both, as they are first; undo and redo give the cells they changed
// back; New and loading forget all
void undo_save(int x0, int y0, int x1, int y1);
void undo_save_size(int *sizes, int index); // col_w or row_h: a column or a row
int undo_save_table(int *sizes, int count); // all the cells, and col_w or row_h 1..count; 0 if no memory
// a step undone, or redone with redo; 1 if there was one
int table_undo(int redo, int *x0, int *y0, int *x1, int *y1);
int undo_steps(void), redo_steps(void);
void undo_forget(void);

#endif
