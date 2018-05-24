/*========================================================
=                                                        =
=                        STYLE                           =
=                                                        =
========================================================*/
#define HTML_PADDING_X 8;
#define HTML_PADDING_Y 5;

struct _style {
	bool bold, underlined, italic, strike;
	bool h1, h2, h3, h4, h5, h6;
	bool a;
	bool pre;
	bool ignore;
	bool body;
	dword color;
	void clear();
};

void _style::clear()
{
	bold=underlined=italic=strike=0;
	h1=h2=h3=h4=h5=h6=0;
	a=0;
	pre=0;
	ignore=0;
	color=0;
	body=0;
}

/*========================================================
=                                                        =
=                         TAG                            =
=                                                        =
========================================================*/
struct _tag {
	dword start;
	dword end;
	dword name;

	bool opens;

	dword attribute;
	dword value;

	void parse();
	int nameis();
};

void _tag::parse()
{
	dword START = start;

	calc(1); //WTF

	if (ESBYTE[START]=='/') {
		START++;
		opens = false;
	} else opens = true;

	name = START;
	
	while ( (START < end) && (!__isWhite(ESBYTE[START])) ) {
		START++;
	}
	if (START!=end) ESBYTE[START] = '\0';
	START++;

	strlwr(name);

	start = START;
}

int _tag::nameis(dword _in_tag_name)
{
	if (name) && (streq(_in_tag_name, name)) return true;
	return false;
}

/*========================================================
=                                                        =
=                         DRAW                           =
=                                                        =
========================================================*/
struct _draw
{
	dword x;
	dword y;
	void init();
	void line_break();
};

void _draw::init()
{
	x = HTML_PADDING_X;
	y = HTML_PADDING_Y;
}
void _draw::line_break()
{
	y+= list.item_h;
	x = HTML_PADDING_X;	
}

/*========================================================
=                                                        =
=                         BUF                            =
=                                                        =
========================================================*/
struct _buf
{
	dword pointer;
	dword len;
	dword start;
	dword end;
	void init();
};

void _buf::init(dword _buf_pointer, _buf_len)
{
	pointer = _buf_pointer;
	len = _buf_len;
	start = malloc(len);
	end = start + len;
	strlcpy(start, pointer, len);	
}

/*========================================================
=                                                        =
=                         TEXT                           =
=                                                        =
========================================================*/
struct _text {
	int size_pt_change;
	dword start;
	dword end;

	dword width;
};

/*
dword line_break;
byte char_holder;

if (ESBYTE[buf.pos]==0x0A) {
	if (style.pre) {
		draw.line_break();
		continue;
	}
}

while (get_label_len(text.start) + draw.x + 30 > list.w)
{
	for (line_break=tag.start-1; line_break>text.start; line_break--;)
	{
		char_holder = ESBYTE[line_break]; //set line end
		ESBYTE[line_break] = '\0';
		if (get_label_len(text.start) + draw.x + 30 <= list.w) break;
		ESBYTE[line_break] = char_holder; //restore line
	}
	if (draw_on) {
		if (style.a) {
			link.add(draw.x,draw.y + size_pt_change,get_label_len(text.start),list.item_h,text.start," ");
			label_draw_bar(draw.x, draw.y+kfont.size.pt+1, get_label_len(text.start), style.color);
		}
		WriteTextIntoBuf(draw.x, draw.y, style.color, text.start);
	}
	draw.x+=char_width[' '];
	ESBYTE[line_break] = char_holder; //restore line
	text.start = line_break;
	draw.line_break();
}
if (draw_on) {
	if (style.a) {
		link.add(draw.x,draw.y + size_pt_change,get_label_len(text.start),list.item_h,text.start," ");	
		label_draw_bar(draw.x, draw.y+kfont.size.pt+1, get_label_len(text.start), style.color);
	}
	WriteTextIntoBuf(draw.x, draw.y, style.color, text.start);
}
draw.x += char_width[' '];
draw.x += get_label_len(text.start);
*/


/*========================================================
=                                                        =
=                         DOM                            =
=                                                        =
========================================================*/
struct _dom
{
	_tag tag;
	_style style;
	_draw draw;
	_buf buf;
	_text text;
	_canvas canvas;
	void init();
	void apply_text();
	void set_style();
	void parse();
};

void _dom::init()
{
	style.clear();
	draw.init();
	buf.init(io.buffer_data, strlen(io.buffer_data));
}

void _dom::set_style()
{
	if (tag.nameis("body")) style.body = tag.opens;
	if (tag.nameis("b")) style.bold = tag.opens;
	if (tag.nameis("i")) style.italic = tag.opens;
	if (tag.nameis("s")) style.strike = tag.opens;
	if (tag.nameis("pre")) style.pre = tag.opens;
	if (tag.nameis("style")) style.ignore = tag.opens;
	if (tag.nameis("a"))  { 
		style.a = tag.opens;
		if (tag.opens) style.color=0x0000FF; else style.color=0;
	}
	
	if (tag.nameis("br")) 
		|| (tag.nameis("p")) 
		|| (tag.nameis("div")) 
		|| (tag.nameis("tr")) {
		draw.line_break();
		return;
	}

	if (tag.nameis("li")) && (tag.opens) {
		draw.line_break();
		return;
	}

	if (tag.nameis("td")) && (tag.opens) {
		draw.line_break();
		return;
	}

	/*
	if (tag.nameis("title")) {
		strcpy(#title, text.start);
		strcat(#title, " - Aelia");
		DrawTitle(#title);
	}

	if 	(tag.nameis("h1")) || (tag.nameis("/h1")) ||
		(tag.nameis("h2")) || (tag.nameis("/h2")) ||
		(tag.nameis("h3")) || (tag.nameis("/h3")) {
		if (tag.nameis("h1")) {
			text.size_pt_change = 8;
		} else if (tag.nameis("/h1")) {
			text.size_pt_change = -8;
		} else if (tag.nameis("h2")) {
			text.size_pt_change = 6;
		} else if (tag.nameis("/h2")) {
			text.size_pt_change = -6;
		} else if (tag.nameis("h3")) {
			text.size_pt_change = 4;
		} else if (tag.nameis("/h3")) {
			text.size_pt_change = -4;
		}
		kfont.size.pt += text.size_pt_change;
		get_label_symbols_size();
		if (text.size_pt_change > 0) {
			draw.y+= list.item_h;//что если будет очень длинная строка в теге?
		} else {//коммент выше и коммент ниже связаны
			draw.y+= list.item_h - text.size_pt_change;//не очень понятна логика этого места
			text.size_pt_change = 0;
		}
		draw.x = HTML_PADDING_X;
		return;					
	}
	*/
}

void _dom::apply_text()
{
	if (kfont.size.height) && (style.body) {
		kfont.bold = style.bold;
		canvas.write_text(draw.x, draw.y, style.color, text.start);
		if (style.a) {
			canvas.draw_hor_line(draw.x, draw.y + list.item_h-2, kfont.get_label_width(text.start), style.color); 
			link.add(draw.x, draw.y, kfont.get_label_width(text.start), list.item_h, text.start, "http://kolibrios.org");
		}
	}
}

void _dom::parse()
{
	dword i;
	init();

	text.start = buf.start;
	tag.start = buf.start;
	for ( i=buf.start; i<buf.end; i++ )
	{
		if (ESBYTE[i]=='<') {
			tag.start = i+1;
			text.end = i-1;
			ESBYTE[i] = '\0';
			apply_text();
		}
		if (ESBYTE[i]=='>') {
			tag.end = i-1;
			text.start = i+1;
			ESBYTE[i] = '\0';
			tag.parse();
			set_style();
		}
	}

	free(buf.start);
	if (!kfont.size.height) {
		list.count = draw.y/list.item_h+3;
		if (list.count < list.visible) list.count = list.visible;
		kfont.size.height = list.count+5*list.item_h;
		kfont.raw_size = 0;
		parse();
	}
}



/*========================================================
=                                                        =
=                       PREPARE                          =
=                                                        =
========================================================*/
void PreparePage() 
{
	_dom dom;

	strcpy(#title, history.current()+strrchr(history.current(),'/'));
	ChangeCharset(charsets[encoding], "CP866", io.buffer_data);
	link.clear();

	kfont.size.height = 0;

	if ( strstri(io.buffer_data, "<html") == -1 ) ParseTxt(); else dom.parse();

	kfont.ApplySmooth();

	DrawPage();
}

void ParseTxt()
{
_canvas canvas;
byte ch, zeroch=0;
dword bufoff, buflen, line_start, srch_pos;
int stroka_y=5, line_length=0;

	line_start=io.buffer_data;
	buflen = strlen(io.buffer_data) + io.buffer_data;
	for (bufoff=io.buffer_data; bufoff<buflen; bufoff++)
	{
		ch = ESBYTE[bufoff];
		line_length += kfont_char_width[ch];
		if (line_length>=list.w-30) || (ch==10) {
			srch_pos = bufoff;
			loop()
			{
				if (__isWhite(ESBYTE[srch_pos])) { bufoff=srch_pos+1; break; } //normal word-break
				if (srch_pos == line_start) break; //no white space found in whole line
				srch_pos--;
			}
			if (kfont.size.height) {
				ESBYTE[bufoff] >< zeroch; //set line end
				canvas.write_text(8, stroka_y, 0x000000, line_start);
				ESBYTE[bufoff] >< zeroch; //restore line
			}
			stroka_y += list.item_h;
			line_start = bufoff;
			line_length = 0;
		}
	}
	if (!kfont.size.height) {
		list.count = stroka_y/list.item_h+3;
		if (list.count < list.visible) list.count = list.visible;
		kfont.size.height = list.count+5*list.item_h;
		kfont.raw_size = 0;
		ParseTxt();
	} 
	else canvas.write_text(8, stroka_y, 0x000000, line_start);
}