struct libimg_image {
	dword image, w, h;
};

int Libimg_LoadImage(dword struct_pointer, file_path)
{
	int image_pointer;
	image_pointer = load_image(file_path);
	if (!image_pointer) notify("Error: Skin not loaded");
	ESDWORD[struct_pointer] = image_pointer;
	ESDWORD[struct_pointer+4] = DSWORD[image_pointer+4];
	ESDWORD[struct_pointer+8] = DSWORD[image_pointer+8];
}