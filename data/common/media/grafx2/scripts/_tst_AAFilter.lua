-- Apply a kind of AA filter on picture

-- Get the picture size
w, h = getpicturesize();

-- Here is the filtering matrix
 matrix = {
	 {  0, -1,  0 },
	 { -1,  5, -1 },
	 {  0, -1,  0 }};

-- Loop trough all the pixels
-- To make this script simpler we don't handle the picture borders
-- (the matrix would get pixels outside the picture space)
-- for var = start_value, end_value, step do ...
for y = 1, h - 2, 1 do
	for x = 1, w - 2, 1 do
		filtered =
			matrix[1][1] * getbackuppixel(x - 1, y - 1) +
			matrix[1][2] * getbackuppixel(x    , y - 1) +
			matrix[1][3] * getbackuppixel(x + 1, y - 1) +
			matrix[2][1] * getbackuppixel(x - 1, y    ) +
			matrix[2][2] * getbackuppixel(x    , y    ) +
			matrix[2][3] * getbackuppixel(x + 1, y    ) +
			matrix[3][1] * getbackuppixel(x - 1, y + 1) +
			matrix[3][2] * getbackuppixel(x    , y + 1) +
			matrix[3][3] * getbackuppixel(x + 1, y + 1);
		putpicturepixel(x,y,filtered);
	end
end
