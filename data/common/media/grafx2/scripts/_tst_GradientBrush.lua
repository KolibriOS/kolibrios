w, h = getbrushsize()

for x = 0, w - 1, 1 do
	for y = 0, h - 1, 1 do
		putbrushpixel(x, y, (x+y)%256);
	end
end
