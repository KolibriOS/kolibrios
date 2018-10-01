
void Sort_by_Size(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{                                        
	int j;
	int iss = a;
	if (a >= b) return;
	for (j = a; j <= b; j++)
		if (ESDWORD[file_mas[j]*304 + buf+64] <= ESDWORD[file_mas[b]*304 + buf+64])	{ file_mas[iss] >< file_mas[j];   iss++;}
	Sort_by_Size (a, iss-2);
	Sort_by_Size (iss, b);
}


void Sort_by_Name(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{                                        
	int j;
	int isn = a;
	if (a >= b) return;
	for (j = a; j <= b; j++)
		if (strcmp(file_mas[j]*304 + buf+72, file_mas[b]*304 + buf+72)<=0) { file_mas[isn] >< file_mas[j];   isn++;}
	Sort_by_Name(a, isn-2);
	Sort_by_Name(isn, b);
}

	
void Sort_by_Type(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{         
	int j;      
	dword filename1, filename2, ext1, ext2;
	int n, isn = a;
	if (a >= b) return;
	for (j = a; j <= b; j++)
	{
		filename1 = file_mas[j]*304 + buf+72;
		filename2 = file_mas[b]*304 + buf+72;

		n=strlen(filename1)-1;
		WHILE (n>0) && (ESBYTE[filename1+n]!='.') n--;
		if (n) ext1 = filename1+n+1; else ext1=0;
		n=strlen(filename2);
		n--;
		WHILE (n>0) && (ESBYTE[filename2+n]!='.') n--;
		if (n) ext2 = filename2+n+1; else ext2=0;

		n=strcmp(ext1, ext2);
		if (n<0) { file_mas[isn] >< file_mas[j];   isn++;} 
		if (!n) && (strcmp(filename1, filename2)<=0) { file_mas[isn] >< file_mas[j];   isn++;}
	}
	Sort_by_Type(a, isn-2);
	Sort_by_Type(isn, b);
}



