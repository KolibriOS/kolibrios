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

void _dom::apply_text(dword _intext)
{
	int label_w;
	int textlen = strlen(_intext);
	dword curline = malloc(textlen);

	if (!textlen) || (!style.body) return;

	do {
		strlcpy(curline, _intext, textlen);
		label_w = kfont.get_label_width(curline);
		textlen--;
	} while (label_w > list.w - draw.x - 10) && (textlen);

	kfont.bold = style.bold;

	if (kfont.size.height) {
		canvas.write_text(draw.x, draw.y, style.color, curline);
		if (style.a) {
			canvas.draw_hor_line(draw.x, draw.y + list.item_h-2, 
				kfont.get_label_width(curline), style.color); 
			link.add(draw.x, draw.y, kfont.get_label_width(curline), 
				list.item_h, curline, "http://kolibrios.org");
		}		
	}
	draw.x += label_w;
	strcpy(curline, _intext + textlen);

	if (textlen) && (textlen < strlen(_intext)-1) {
		draw.x = 0;
		draw.y += list.item_h;
		apply_text(_intext + textlen);
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
			apply_text(text.start);
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
	ChangeCharset(encoding, "CP866", io.buffer_data);
	link.clear();

	kfont.size.height = 0;

	dom.parse();

	kfont.ApplySmooth();

	DrawPage();
}
