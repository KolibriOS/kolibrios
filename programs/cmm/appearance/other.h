void Sort_by_Name(int a, b)
{
	int j, i = a;
	if (a >= b) return;
	for (j = a; j <= b; j++)
		if (strcmp(files_mas[j]*304 + buf+72, files_mas[b]*304 + buf+72)<=0) { files_mas[i] >< files_mas[j];   i++;}
	Sort_by_Name(a, i-2);
	Sort_by_Name(i, b);
}