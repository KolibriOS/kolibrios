struct SELECTION {
	dword start_x, start_y, start_offset;
	dword end_x, end_y, end_offset;
	dword color;
	bool is_active();
	void set_start();
	void set_end();
	void draw();
	void draw_line();
	void cancel();
	bool swap_start_end();
	void normalize();
	void select_all();
} selection;

bool SELECTION::is_active()
{
	if (start_offset) && (end_offset) && (start_offset != end_offset) {
		return true;
	} else {
		return false;
	}
}

void SELECTION::draw_line(dword x,y,w)
{
	DrawBuf.DrawBar(x, y, w, list.item_h, color);
}

void SELECTION::draw(int i)
{
	if (is_active()) {
		if (start_y == i) && (end_y == i) draw_line(start_x * list.font_w+2, start_y * list.item_h, end_x - start_x * list.font_w);
		else if (start_y == i) draw_line(start_x * list.font_w+2, start_y * list.item_h, list.w -2- calc(start_x * list.font_w));
		else if (end_y == i) draw_line(0, end_y * list.item_h, end_x * list.font_w+2);
		//DrawBuf.DrawBar(start_x * list.font_w + 2,  start_y * list.item_h, 2, list.item_h, 0x00FF00);
		//DrawBuf.DrawBar(end_x * list.font_w + 0,  end_y * list.item_h, 2, list.item_h, 0xFF00FF);
	}
	DrawBuf.DrawBar(list.cur_x * list.font_w + 2,  list.cur_y * list.item_h, 2, list.item_h, theme.cursor); //DrawCursor
}

void SELECTION::cancel()
{
	start_offset = end_offset = lines.get(list.cur_y) + list.cur_x;
	start_x = end_x = list.cur_x;
	start_y = end_y = list.cur_y;
	normalize();
}

void SELECTION::set_start()
{
	start_x = list.cur_x;
	start_y = list.cur_y;
	normalize();
	start_offset = lines.get(start_y) + start_x;
}

void SELECTION::set_end()
{
	end_x = list.cur_x;
	end_y = list.cur_y;
	normalize();
	end_offset = lines.get(end_y) + end_x;
	debugval("end_x", end_x);
	debugval("end_y", end_y);
}


void SELECTION::normalize()
{
	start_x = math.min(start_x, lines.get(start_y+1) - lines.get(start_y));
	end_x   = math.min(end_x,   lines.get(end_y+1) - lines.get(end_y));
}

void SELECTION::select_all()
{
	start_y = 0;
	start_x = 0;
	end_y = lines.count-2;
	end_x = lines.get(end_y+1) - lines.get(end_y);
	//normalize();
	start_offset = lines.get(start_y) + start_x;
	end_offset = lines.get(end_y) + end_x;
	debugval("end_x__", end_x);
	debugval("end_y__", end_y);
}

bool SELECTION::swap_start_end()
{
	start_offset >< end_offset;
	start_x >< end_x;
	start_y >< end_y;
	return true;
}
