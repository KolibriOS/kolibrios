# TABLE

A spreadsheet for KolibriOS: 104 columns (A..CZ) by 100 rows, a cell
holds up to 255 characters. Texts in English, Russian or Spanish, by the
language of the build.

`table [file.csv]` opens the file at start.

## Files

CSV only. A file is saved as it was opened: UTF-8 with or without BOM,
or cp866; the separator (`,` or `;`, found from the first line) and the
line ends are kept. A new table is saved in UTF-8 with BOM, separated by
`,`. Cells are saved as typed, formulas too.

A file larger than the table (more rows or columns, longer texts, or
characters cp866 has not) opens as far as it fits, but without its name:
Save asks for a new one, so that the original is not cut.

## Formulas

A cell that starts with `=` is a formula, written as in Excel and
LibreOffice Calc; function names in any case, arguments apart by `,`
or `;`.

| | |
|---|---|
| Operators | `+ - * / ^`, `%` after a number (`50%` is 0.5), brackets |
| Comparisons | `= <> < > <= >=`: 1 or 0 |
| References | `A1` moves when copied, `$A$1` does not, `$A1` and `A$1` keep the column or the row; an empty cell is 0 |
| Ranges | `A1:B3` in `SUM`, `AVERAGE`, `MIN`, `MAX`, `AND`, `OR`, `XOR` |
| Logic | `IF(c, a[, b])`, `NOT`, `AND`, `OR`, `XOR`, `ISNUMBER` |
| Math | `SQRT`, `ABS`, `EXP`, `LN`, `LOG` (base 10), `ROUND(x, n)`, `FLOOR(x, s)`, `CEILING(x, s)`, `MOD`, `RAND()`, `PI()` |
| Trigonometry | `SIN`, `COS`, `TAN`, `COT`, `ASIN`, `ACOS`, `ATAN`, `RADIANS`, `DEGREES` |
| Errors | `#DIV/0!`, `#VALUE!`, `#REF!` (also a cell that depends on itself), `#NAME?`, `#NUM!`, `#ERROR!` (no formula) |

## Keys

| | |
|---|---|
| Ctrl+N, Ctrl+O, Ctrl+S | new table, open, save |
| F1 | help |
| Arrows, PgUp, PgDn | move; with Shift select |
| Home, End | first, last column |
| A character, F2 | edit the cell, from that character or with its text |
| Enter, Esc | end editing and go down; cancel it |
| Delete | clear the selection |
| Ctrl+X, Ctrl+C, Ctrl+V | cut, copy, paste |
| Ctrl+Z, Ctrl+Y | undo, redo |
| Ctrl+A | select all |
| Ctrl+F | show the formulas instead of their values |

## Mouse

- A click selects a cell, a drag a range, Shift+click extends it; a
  double click edits the cell; a click in the formula bar edits there.
- A header selects its column or row, a drag several; the corner selects
  all.
- A drag of a header line resizes the column or row; a double click on
  it fits the column to its texts or gives the row its usual height.
- The handle at the corner of the selection fills the cells it is
  dragged over with the selection repeated, references moved; dragged
  back inside, it clears the cells left out.
- The right button: cut, copy, paste, clear; on a header also insert
  before or after, and delete the columns or rows.
