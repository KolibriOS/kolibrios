; Matrix & Vertex

align 4
an180f dd 180.0 ;константа для вычисления углов

struct M4
	m rd 4*4 ;float[4][4]
ends

; макрос облегчающий доступ к элементам матрицы M4
; для доступа к элементам 1-й индекс множится на 16, 2-й на 4:
; m[i1*16][i2*4], потом все пишется в регистр reg:
; reg <-- m[i1][i2]
macro M4_reg reg,m,i1,i2
{
	mov reg,i1 ;1-й индекс
	if i2 eq 0
		shl reg,4
	else
		shl reg,2
		add reg,i2 ;2-й индекс
		shl reg,2
	end if
	add reg,m ;указатель на начало матрицы
}

struct M3
	m rd 3*3 ;float[3][3]
ends

struct M34
	m rd 3*4 ;float[3][4]
ends

struct V3
	v rd 3 ;float[3]
ends

struct V4
	v rd 4 ;float[4]
ends
	
