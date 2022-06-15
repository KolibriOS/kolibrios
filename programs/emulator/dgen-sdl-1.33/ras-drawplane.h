/*
 * Looks up a vertical scroll value and sets some related variables.
 */
#undef LOOKUP_YSCROLL_REC
#define LOOKUP_YSCROLL_REC(rec_no)					\
	do {								\
		yscroll_amount = get_word(vsram + rec_no * 2) & 0x7ff;	\
									\
		/* interlace ? */					\
		if (reg[12] & 2)					\
			yscroll_amount >>= 1;				\
									\
		/* Offset for the line */				\
		yscroll_amount += line;					\
									\
		yoff = ((yscroll_amount >> 3) & (ysize - 1));		\
		tile_line = (tiles + ((xsize * yoff) & 0x1fff));	\
		scan = (yscroll_amount & 7);				\
	}								\
	while (0)

{
	int xsize, ysize;
	int x, scan = 0, w, xstart;
	static int sizes[4] = { 32, 64, 64, 128 };
	unsigned which;
	unsigned char *where, *hscroll_rec_ptr, *tiles, *tile_line = NULL;
	int xoff, yoff, xoff_mask;
	int hscroll_amount, yscroll_amount = 0;
	uint8_t two_cell_vscroll = 0;

	/*
	 * when VSCR bit is set in register 11, this is 'per 2-cell'
	 * vertical scrolling as opposed to full screen vscrolling.
	 */
	two_cell_vscroll = ((reg[11] >> 2) & 0x1);

#if PLANE == 0
	// Plane 0 is only where the window isn't
	// This should make Herzog Zwei split screen work perfectly, and clean
	// up those little glitches on Sonic 3's level select.
	if (reg[18] & 0x80) {
		// Window goes down, plane 0 goes up! :)
		if ((line >> 3) >= (reg[18] & 0x1f))
			return;
	}
	else {
		// Window goes up, plane 0 goes down
		if ((line >> 3) < (reg[18] & 0x1f))
			return;
	}
#endif

	/*
	 * Get the vertical/horizontal scroll plane sizes
	 *
	 * 0b00: 32 cell
	 * 0b01: 64 cell
	 * 0b10: prohibited, but unlicensed games use this
	 *       turns out to be 64.
	 * 0b11: 128 cell
	 */
	xsize = (sizes[(reg[16] & 3)] << 1);
	ysize = sizes[((reg[16] >> 4) & 3)];

	/*
	 * Here we compute pointer to the beginning of the  hscroll table.
	 * The base address of the table is stored in reg[13] << 10.
	 */
#if PLANE == 0
	hscroll_rec_ptr = (vram + ((reg[13] << 10) & 0xfc00));
	tiles = (vram + (reg[2] << 10));
#else // PLANE == 1
	hscroll_rec_ptr = (vram + ((reg[13] << 10) & 0xfc00) + 2);
	tiles = (vram + (reg[4] << 13));
#endif

	// Wide or narrow?
	if (reg[12] & 1) {
		w = 40;
		xstart = -8;
	}
	else {
		w = 32;
		xstart = 24;
	}

	/*
	 * Lookup the horizontal offset.
	 * See Charles MacDonald's genvdp.txt for explanation.
	 */
	switch (reg[11] & 3) {
	case 0:
		// full screen
		// NOP - pointer in the right place
		break;
	case 1:
		// invalid, but populous uses it
		hscroll_rec_ptr += ((line & 7) << 2);
		break;
	case 2:
		// per tile
		hscroll_rec_ptr += ((line & ~7) << 2);
		break;
	case 3:
		// per line
		hscroll_rec_ptr += (line << 2);
		break;
	}

	hscroll_amount = get_word(hscroll_rec_ptr);
	xoff_mask = xsize - 1;
	xoff = ((-(hscroll_amount>>3) - 1)<<1) & xoff_mask;
	where = dest + (xstart + (hscroll_amount & 7)) * (int) Bpp;

	/*
	 * If this is not column vscroll mode, we look up the
	 * whole screen vertical scroll value once and once only.
	 */
	if (two_cell_vscroll == 0)
		LOOKUP_YSCROLL_REC(PLANE);

	/*
	 * Loop cells, we draw 2 more cells than expected (-1 and w) because
	 * previously off-screen cells can be horizontally scrolled on-screen.
	 */
	for (x = -1; (x <= w); x++) {
		/*
		 * If we are in 2-cell vscroll mode then lookup the amount by
		 * which we should scroll this tile.
		 *
		 * If we are not in 2-cell vscroll then we looked up the value
		 * for the whole screen vscroll earlier.
		 *
		 * We lookup vscroll values on even x values and this is the
		 * vscroll value for the next two cells. Note that cell -1 is
		 * a special case as we never looked up the vscroll value for
		 * cell -2.
		 */
		if ((two_cell_vscroll) && ((x % 2 == 0) || (x == -1))) {

			/*
			 * Note that the underflow and overflow of the table
			 * for cell -1 and cell w is intentional.
			 *
			 * http://gendev.spritesmind.net/forum/viewtopic.php?t=737&postdays=0&postorder=asc&start=30
			 */
			uint8_t cell_index = (uint8_t) x % w;
			int vscroll_rec_no = 2 * (cell_index / 2);

			/*
			 * The records alternate, PLANE A, PLANE B, PLANE A,
			 * ...
			 */
#if PLANE == 1
			vscroll_rec_no++;
#endif
			LOOKUP_YSCROLL_REC(vscroll_rec_no);
		}

#if PLANE == 0
		if (reg[17] & 0x80) {
			// Don't draw where the window will be
			if (x >= ((reg[17] & 0x1f) << 1))
				goto skip;
		}
		else {
			// + 1 so scroll layers in Sonic look right
			if ((x + 1) < ((reg[17] & 0x1f) << 1))
				goto skip;
		}
#endif
		which = get_word(tile_line + xoff);

#if (FRONT == 0) && (PLANE == 1)
		draw_tile_solid(which, scan, where);
#elif FRONT == 1
		if (which >> 15)
			draw_tile(which, scan, where);
#else
		if (!(which >> 15))
			draw_tile(which, scan, where);
#endif

#if PLANE == 0
	skip:
#endif
		where += Bpp_times8;
		xoff = ((xoff + 2) & xoff_mask);
	}
}
