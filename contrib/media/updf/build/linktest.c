/* Minimal MuPDF 1.19 link test for KolibriOS: exercises the whole
   render pipeline so the linker must resolve every core symbol.
   Not meant to run as-is; this validates that the port LINKS. */

#include <mupdf/fitz.h>
#include <stdio.h>

int render_first_page(const char *path)
{
	fz_context *ctx;
	fz_document *doc;
	fz_pixmap *pix;
	fz_matrix ctm;
	int pages;

	ctx = fz_new_context(NULL, NULL, FZ_STORE_UNLIMITED);
	if (!ctx)
		return 1;

	fz_try(ctx)
	{
		fz_register_document_handlers(ctx);
		doc = fz_open_document(ctx, path);
		pages = fz_count_pages(ctx, doc);
		ctm = fz_scale(1.0f, 1.0f);
		pix = fz_new_pixmap_from_page_number(ctx, doc, 0, ctm, fz_device_rgb(ctx), 0);
		fz_save_pixmap_as_png(ctx, pix, "out.png");
		fz_drop_pixmap(ctx, pix);
		fz_drop_document(ctx, doc);
	}
	fz_catch(ctx)
	{
		fprintf(stderr, "mupdf error: %s\n", fz_caught_message(ctx));
		fz_drop_context(ctx);
		return 2;
	}
	fz_drop_context(ctx);
	return pages;
}

int main(int argc, char **argv)
{
	if (argc < 2)
		return 1;
	return render_first_page(argv[1]);
}
