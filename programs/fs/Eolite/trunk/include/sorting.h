//Быстрая сортировка. Leency 2008.

void Sort_by_Size(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{                                        
	int iss = a;
	IF (a >= b) return;
	FOR (j = a; j <= b; j++)
		IF (ESDWORD[file_mas[j]*304 + buf+64] <= ESDWORD[file_mas[b]*304 + buf+64])	{ file_mas[iss] >< file_mas[j];   iss++;}
	Sort_by_Size (a, iss-2);
	Sort_by_Size (iss, b);
}


void Sort_by_Name(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{                                        
	int isn = a;
	IF (a >= b) return;
	FOR (j = a; j <= b; j++)
		IF (strcmp(file_mas[j]*304 + buf+72, file_mas[b]*304 + buf+72)<=0) { file_mas[isn] >< file_mas[j];   isn++;}
	Sort_by_Name(a, isn-2);
	Sort_by_Name(isn, b);
}

 
void Sort_by_Type(int a, b)   // для первого вызова: a = 0, b = <элементов в массиве> - 1
{                                        
	int jst, ist=a;
	int n;
	unsigned char filename1[256], filename2[256];
	unsigned char ext1[256], ext2[256];
	
	IF (a >= b) return;
	for (jst = a; jst <= b; jst++)
	{
		ext1[0]=ext2[0]=filename1[0]=filename2[0]=NULL;
		copystr(file_mas[jst]*304 + buf+72, #filename1);
		copystr(file_mas[b]*304 + buf+72, #filename2);
		n=strlen(#filename1)-1;
		WHILE (filename1[n]<>'.') && (n>0) n--;
		IF (n) copystr(#filename1+n+1, #ext1);
		n=strlen(#filename2)-1;
		WHILE (filename2[n]<>'.') && (n>0) n--;
		IF (n) copystr(#filename2+n+1, #ext2);

		n=strcmp(#ext1, #ext2);
		IF (n<0) { file_mas[ist] >< file_mas[jst];   ist++;} 
		IF (n==0) && (strcmp(#filename1, #filename2)<=0) { file_mas[ist] >< file_mas[jst];   ist++;}
	}
	Sort_by_Type(a, ist-2);
	Sort_by_Type(ist, b);
}



