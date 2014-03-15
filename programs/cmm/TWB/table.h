struct Table
{
	byte active;
	byte max_cols;
	byte max_rows;
	byte cur_col;
	byte cur_row;
	int col_w[32];
	int row_start, row_h, row_max_h;
	void NewTable();
} table;

void Table::NewTable()
{
	cur_row = 0;
	cur_col = 0;
	max_rows = 0;
	max_cols = 0;
}