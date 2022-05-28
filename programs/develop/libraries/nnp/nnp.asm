;MIT License

;Copyright (c) 2020 Artem Yashin

;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:

;The above copyright notice and this permission notice shall be included in all
;copies or substantial portions of the Software.

;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
;SOFTWARE.

;https://www.youtube.com/watch?v=GNcGPw_Kb_0

;https://www.youtube.com/watch?v=wzIOl4hWP3U
; 1:20 - cats images

format MS COFF
public EXPORTS
section '.flat' code readable align 16

include "..\..\..\KOSfuncs.inc"
include "..\..\..\proc32.inc"
include "..\..\..\macros.inc"
include "..\..\..\dll.inc"
include "..\..\..\bcc32\include\kos_func.inc"
include "..\..\..\bcc32\include\kos_heap.inc"

@@memcpy$qpvpxvui equ @memcpy$qpvpxvui
@@DoubleToStr$qduso equ @DoubleToStr$qduso
@@strlen$qpxc equ @strlen$qpxc
@@strstr$qpxct1 equ @strstr$qpxct1
@@strchr$qpxci equ @strchr$qpxci
@@StrToInt$qpc equ @StrToInt$qpc
@@StrToDouble$qpc equ @StrToDouble$qpc

mem.alloc   dd ? ;функция для выделения памяти
mem.free    dd ? ;функция для освобождения памяти
mem.realloc dd ? ;функция для перераспределения памяти
dll.load    dd ?

PRECISION equ 16
NNP_FF_BIN  equ 0x6e6962
NNP_FF_JSON equ 0x6e6f736a

struct Layer
	c_size  dd ? ;+ 0 curent size - число нейронов в текущем слое
	n_size  dd ? ;+ 4 next size   - число нейронов на следующем слое
	neurons dd ? ;+ 8 []
	biases  dd ? ;+12 []
	weights dd ? ;+16 [][]
ends

struct NeuralNetwork
	learningRate  dq ? ;+ 0 скорость обучения
	layers        dd ? ;+ 8 [] слои
	layers_length dd ? ;+12 число слоев
	activation    dd ? ;+16 указатель на функцию активации
	derivative    dd ? ;+20 указатель на функцию
	errors        dd ? ;+24 массив для вычислений
	errorsNext    dd ? ;+28
	gradients     dd ? ;+32
	deltas        dd ? ;+36
ends

align 4
_rand_x dd 0

txt_QlearningRateQ_ db '"learningRate": ',0
txt_zap_nl   db ",",10,0
txt_Qlayers_lengthQ db '"layers_length": ',0
txt_QlayersQ db '"layers": [',0
txt_nl_t_Qc_sizeQ db 10,9,'{"c_size": ',0
txt_zap_nl_t_Qn_sizeQ db ",",10,9,'"n_size": ',0
txt_t_QneuronsQ db 9,'"neurons": [',0
txt_zap_sp      db ", ",0
txt_sqbr_zap_nl db	"],",10,0
txt_t_QbiasesQ  db 9,'"biases": [',0
txt_sqbr_zap_t_QweightsQ db "],",10,9,'"weights": [',10,9,9,0
txt_zap_nl_t_t  db ",",10,9,9,0
txt_sqbro       db "[",0
txt_sqbr        db "]",0
txt_sqbr_fbr_zap  db "]},",0
txt_nl_t_sqbr     db 10,9,"]",0
txt_learningRate  db '"learningRate"',0
txt_layers_length db '"layers_length"',0
;txt_pri_zap db '%i,',0
txt_c_size  db '"c_size"',0
txt_n_size  db '"n_size"',0
txt_biases  db '"biases"',0
txt_weights db '"weights"',0

txt_err_layers_neq db 'number of layers does not match',0
txt_err_c_size   db "not found value 'c_size'",0
txt_err_sqbrl_b1 db "not found opening '[' for biases",0
txt_err_sqbrl_w1 db "not found opening '[' for weights",0
txt_err_sqbrl_w2 db "not found opening '[[' for weights",0
txt_err_sqbrr_w2 db "not found closing ']]' for weights",0

align 16
proc lib_init 
	mov     [mem.alloc], eax
	mov     [mem.free], ebx
	mov     [mem.realloc], ecx
	mov     [dll.load], edx

	or      edx, edx
	jz      @f
		invoke  dll.load, @IMPORT
@@:
	ret
endp

align 16
Math_random:
	imul      eax,dword[_rand_x],22695477
	inc       eax
	push      ecx
	mov       dword[_rand_x],eax
	and       eax,65535
	mov       dword[esp],eax
	fild      dword[esp]
	fmul      dword[@f]
	pop       edx
	ret 
	align 4
@@:
	db 0,0,128,55 ;dd 1.0/65536.0

align 16
sigmoid:
	push      ebp
	mov       ebp,esp
	add       esp,-8
	fld       qword[ebp+8]
	fchs
	fstp      qword[esp]
	call      dword[_exp]
	add       esp,8
	fadd      dword[f_1_0]
	fdivr     dword[f_1_0]
	pop       ebp
	ret       8

align 16
dsigmoid:
	push      ebp
	mov       ebp,esp
	fld1
	fsub      qword[ebp+8]
	fmul      qword[ebp+8]
	pop       ebp
	ret       8

align 16
Layer_Create:
	push      ebp
	mov       ebp,esp
	push      ebx esi edi
	mov       esi,[ebp+8]
	mov       edi,[ebp+12]
	mov       ebx,edi
	shl       ebx,3
	mov       dword[esi+Layer.c_size],edi
	mov       eax,[ebp+16]
	mov       dword[esi+Layer.n_size],eax
	push      ebx
	call      @$bnwa$qui
	pop       ecx
	mov       dword[esi+Layer.neurons],eax
	push      ebx
	call      @$bnwa$qui
	pop       ecx
	mov       dword[esi+Layer.biases],eax
	mov       eax,edi
	shl       eax,2
	push      eax
	call      @$bnwa$qui
	pop       ecx
	mov       dword[esi+Layer.weights],eax
	xor       ebx,ebx
	cmp       edi,ebx
	jbe        .end_f
@@:
	mov       eax,[ebp+16]
	shl       eax,3
	push      eax
	call      @$bnwa$qui
	pop       ecx
	mov       edx,[esi+Layer.weights]
	mov       dword[edx+4*ebx],eax
	inc       ebx
	cmp       edi,ebx
	ja         @b
.end_f:
	pop       edi esi ebx ebp
	ret       12

;+ 8 NeuralNetwork* o
;+12 double learningRate
;+20 void* activation
;+24 void* derivative
;+28 unsigned long* sizes
;+32 long sizes_length
align 16
NNP_Create:
	push      ebp
	mov       ebp,esp
	add       esp,-12
	push      ebx esi edi
	mov       eax,[ebp+8] ;o
	mov       edx,[ebp+12] ;learningRate
	mov       dword[eax+NeuralNetwork.learningRate],edx
	mov       edx,[ebp+16] ;learningRate (2)
	mov       dword[eax+NeuralNetwork.learningRate+4],edx
	mov       ecx,[ebp+8] ;o
	mov       eax,[ebp+20] ;activation
	or        eax,eax
	jnz       @f
	mov       eax,sigmoid
@@:
	mov       dword[ecx+NeuralNetwork.activation],eax
	mov       edx,[ebp+8] ;o
	mov       eax,[ebp+24] ;derivative
	or        eax,eax
	jnz       @f
	mov       eax,dsigmoid
@@:
	mov       dword[edx+NeuralNetwork.derivative],eax
	mov       eax,[ebp+32] ;sizes_length
	imul      eax,sizeof.Layer
	stdcall   @$bnwa$qui,eax
	pop       ecx
	mov       edx,[ebp+8] ;o
	mov       dword[edx+NeuralNetwork.layers],eax
	mov       ecx,[ebp+8] ;o
	mov       eax,[ebp+32] ;sizes_length
	mov       dword[ecx+NeuralNetwork.layers_length],eax
	xor       edi,edi ;i=0
	mov       eax,[ebp+28] ;sizes
	lea       edx,[eax+4]
	mov       dword[ebp-8],edx ;save &sizes[i+1]
	jmp       .150
.cycle_0: ;for (i=0; i < sizes_length; i++)
	xor       ecx,ecx
	mov       dword[ebp-4],ecx ;nextSize = 0
	mov       eax,[ebp+32] ;sizes_length
	dec       eax
	cmp       edi,eax
	jae       .152
	mov       edx,[ebp-8]
	mov       ecx,[edx]
	mov       dword[ebp-4],ecx ;nextSize = sizes[i+1]
.152:
	mov       eax,[ebp-4]
	push      eax
	mov       edx,[ebp-8]
	mov       ecx,[edx-4]
	push      ecx
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	mov       eax,[ebp+8] ;o
	mov       edx,[eax+NeuralNetwork.layers]
	add       edx,ecx
	stdcall   Layer_Create,edx
	xor       esi,esi ;j=0
	mov       eax,[ebp-8]
	lea       edx,[eax-4]
	mov       dword[ebp-12],edx ;save &sizes[i]
	jmp       .154
.cycle_1: ;for (j=0; j < sizes[i]; j++)
	call      Math_random
	fmul      dword[f_2_0]
	fsub      dword[f_1_0]
	mov       eax,[ebp+8] ;o
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	add       ecx,[eax+NeuralNetwork.layers]
	mov       ecx,[ecx+Layer.biases]
	fstp      qword[ecx+8*esi]
	xor       ebx,ebx ;k=0
	cmp       ebx,[ebp-4]
	jae       .157
@@: ;for (k=0; k < nextSize; k++)
	call      Math_random
	fmul      dword[f_2_0]
	fsub      dword[f_1_0]
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8] ;o
	mov       ecx,[edx+NeuralNetwork.layers]
	mov       eax,[ecx+eax+Layer.weights]
	mov       edx,[eax+4*esi]
	fstp      qword[edx+8*ebx]
	inc       ebx
	cmp       ebx,[ebp-4]
	jb        @b
.157:
	inc       esi
.154:
	mov       ecx,[ebp-12]
	cmp       esi,[ecx]
	jb        .cycle_1
	inc       edi
	add       dword[ebp-8],4
.150:
	cmp       edi,[ebp+32] ;sizes_length
	jb        .cycle_0
;create errors array
	push      dword[ebp+8]
	call      NNP_GetMaxLLen
	mov       esi,eax
	shl       esi,4
	stdcall   @$bnwa$qui,esi
	pop       ecx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.errors],eax
	shr       esi,1
	add       eax,esi
	mov       dword[edx+NeuralNetwork.errorsNext],eax
;create gradients array
	stdcall   @$bnwa$qui,esi
	pop       ecx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.gradients],eax
;create deltas array
	shr       esi,1
	stdcall   @$bnwa$qui,esi
	pop       ecx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.deltas],eax
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       28
align 4        
f_2_0:
	dd 2.0
f_1_0:
	dd 1.0

;заполнение случайными числами
;+ 8 NeuralNetwork* o
align 16
NNP_Reset:
	push      ebp
	mov       ebp,esp
	add       esp,-8
	push      ebx esi edi

	xor       edi,edi ;i=0
	jmp       .3
.cycle_0: ;for (i=0; i < o->layers_length; i++)
	xor       esi,esi ;j=0
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	add       eax,[edx+NeuralNetwork.layers]
	mov       edx,[eax+Layer.n_size]
	mov       [ebp-4],edx
	mov       edx,[eax+Layer.c_size]
	mov       [ebp-8],edx
	jmp       .2
.cycle_1: ;for (j=0; j < o->layers[i].c_size; j++)
	call      Math_random
	fmul      dword[f_2_0]
	fsub      dword[f_1_0]
	mov       eax,[ebp+8] ;o
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	add       ecx,[eax+NeuralNetwork.layers]
	mov       ecx,[ecx+Layer.biases]
	fstp      qword[ecx+8*esi]
	xor       ebx,ebx ;k=0
	cmp       ebx,[ebp-4]
	jae       .1
@@: ;for (k=0; k < o->layers[i].n_size; k++)
	call      Math_random
	fmul      dword[f_2_0]
	fsub      dword[f_1_0]
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8] ;o
	add       eax,[edx+NeuralNetwork.layers]
	mov       eax,[eax+Layer.weights]
	mov       edx,[eax+4*esi] ;edx = &o->layers[i].weights[j]
	fstp      qword[edx+8*ebx] ;o->layers[i].weights[j][k] = Math_random()*2.0-1.0;
	inc       ebx ;k++
	cmp       ebx,[ebp-4]
	jb        @b
.1:
	inc       esi ;j++
.2:
	cmp       esi,[ebp-8]
	jb        .cycle_1
	inc       edi ;i++
.3:
	mov       ecx,[ebp+8] ;o
	cmp       edi,[ecx+NeuralNetwork.layers_length]
	jb        .cycle_0
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       4

;расчет входных и выходных нейронов
;+ 8 NeuralNetwork* o
;+12 double* inputs
align 16
NNP_FeedForward:
	push      ebp
	mov       ebp,esp
	add       esp,-4
	push      ebx esi edi
	mov       eax,[ebp+8]
	mov       eax,[eax+NeuralNetwork.layers]
	mov       ecx,[eax+Layer.c_size]
	shl       ecx,1
	mov       edi,[eax+Layer.neurons]
	mov       esi,[ebp+12]
	rep   movsd
	mov       edi,[ebp+8]
	mov       esi,[edi+NeuralNetwork.layers]
	sub       esi,sizeof.Layer
	mov       dword[ebp-4],1 ;i = 1
	jmp       .3
align 4
.cycle_0:
	add       esi,sizeof.Layer
	xor       ebx,ebx ;j=0
	jmp       .2
align 4
.cycle_1:
	mov       edx,[esi+sizeof.Layer+Layer.neurons]
	xor       eax,eax
	mov       dword[edx+8*ebx],eax
	mov       dword[edx+8*ebx+4],eax ;l1.neurons[j] = 0
	;;xor       eax,eax ;k=0
	jmp        .1
align 4
.cycle_2:
	;l1.neurons[j] += l.neurons[k] * l.weights[k][j]
	mov       ecx,[esi+Layer.neurons] ;l.neurons
	fld       qword[ecx+8*eax] ;st0 = l.neurons[k]
	mov       ecx,[esi+Layer.weights]
	mov       ecx,[ecx+4*eax]
	fmul      qword[ecx+8*ebx] ;st0 *= l.weights[k][j]
	fadd      qword[edx+8*ebx] ;st0 += l1.neurons[j]
	fstp      qword[edx+8*ebx]
	inc       eax ;k++
.1:
	cmp       eax,[esi+Layer.c_size] ;k < l.c_size
	jb         .cycle_2
	mov       eax,[esi+sizeof.Layer+Layer.biases]
	fld       qword[eax+8*ebx]
	fadd      qword[edx+8*ebx]
	sub       esp,8
	fstp      qword[esp] ;push l1.neurons[j]
	call      dword[edi+NeuralNetwork.activation]
	fstp      qword[edx+8*ebx]
	inc       ebx ;j++
.2:
	cmp       ebx,[esi+sizeof.Layer+Layer.c_size] ;j < l1.c_size
	jb        .cycle_1
	inc       dword[ebp-4] ;i++
.3:
	mov       eax,[edi+NeuralNetwork.layers_length]
	cmp       eax,[ebp-4]
	ja        .cycle_0
	mov       eax,[esi+sizeof.Layer+Layer.neurons]
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       8

;+ 8 NeuralNetwork* o
;+12 double* targets
align 16
NNP_BackPropagation:
;cycle_0	for (i = 0; i < l.c_size; i++)
;cycle_1	for (; k >= 0; k--)
;cycle_2		for (i = 0; i < l1.c_size; i++)
;cycle_3		for (i = 0; i < l1.c_size; i++)
;cycle_4			for (j = 0; j < l.c_size; j++)
;cycle_5		for (i = 0; i < l.c_size; i++)
;cycle_6			for (j = 0; j < l1.c_size; j++)
;cycle_7		for (i = 0; i < l1.c_size; i++)
;cycle_8			for (j = 0; j < l.c_size; j++)
	push      ebp
	mov       ebp,esp
	add       esp,-12
	push      ebx esi edi
	mov       esi,[ebp+8]
	mov       edi,[esi+NeuralNetwork.layers_length]
	dec       edi
	mov       dword[ebp-4],edi ;k = o->layers_length - 1
	imul      edi,sizeof.Layer
	add       edi,[esi+NeuralNetwork.layers]
	xor       ebx,ebx ;i=0
	mov       eax,[ebp+12] ;eax = targets[]
	jmp       .180
align 4
.cycle_0:
	mov       edx,[edi+Layer.neurons]
	mov       ecx,[esi+NeuralNetwork.errors]
	fld       qword[eax]
	fsub      qword[edx+8*ebx]
	fstp      qword[ecx+8*ebx]
	inc       ebx
	add       eax,8
.180:
	cmp       ebx,[edi+Layer.c_size]
	jb        .cycle_0
	dec       dword[ebp-4] ;k--
	cmp       dword[ebp-4],0
	jl        .end_f
align 4
.cycle_1:
	sub       edi,sizeof.Layer
	xor       ebx,ebx ;i=0
	jmp       .186
align 4
.cycle_2:
	mov       eax,[edi+sizeof.Layer+Layer.neurons]
	push      dword[eax+8*ebx+4]
	push      dword[eax+8*ebx]
	call      dword[esi+NeuralNetwork.derivative]
	mov       ecx,[esi+NeuralNetwork.errors]
	fmul      qword[ecx+8*ebx]
	fmul      qword[esi+NeuralNetwork.learningRate]
	mov       edx,[esi+NeuralNetwork.gradients]	
	fstp      qword[edx+8*ebx]
	inc       ebx
.186:
	cmp       ebx,[edi+sizeof.Layer+Layer.c_size]
	jb        .cycle_2
	mov       edx,[esi+NeuralNetwork.deltas]
	xor       ebx,ebx
	jmp       .189
align 4
.cycle_3:
	mov       eax,[edi+Layer.c_size]
	shl       eax,3
	push      eax
	call      @$bnwa$qui
	pop       ecx
	mov       dword[edx],eax
	xor       eax,eax ;j=0
	jmp       .191
align 4
.cycle_4:
	mov       ecx,[esi+NeuralNetwork.gradients]
	fld       qword[ecx+8*ebx]
	mov       ecx,[edi+Layer.neurons]
	fmul      qword[ecx+8*eax]
	mov       ecx,[edx]
	fstp      qword[ecx+8*eax]
	inc       eax
.191:
	cmp       eax,[edi+Layer.c_size]
	jb        .cycle_4
	inc       ebx
	add       edx,4
.189:
	cmp       ebx,[edi+sizeof.Layer+Layer.c_size]
	jb        .cycle_3
	xor       ebx,ebx
	jmp       .195
align 4
.cycle_5:
	mov       eax,[esi+NeuralNetwork.errorsNext]
	xor       edx,edx
	mov       dword[eax+8*ebx],edx
	mov       dword[eax+8*ebx+4],edx
	xor       eax,eax ;j=0
	jmp       .197
align 4
.cycle_6:
	mov       edx,[edi+Layer.weights]
	mov       ecx,[edx+4*ebx]
	mov       edx,[esi+NeuralNetwork.errors]
	fld       qword[ecx+8*eax]
	fmul      qword[edx+8*eax]
	mov       ecx,[esi+NeuralNetwork.errorsNext]
	fadd      qword[ecx+8*ebx]
	fstp      qword[ecx+8*ebx]
	inc       eax
.197:
	cmp       eax,[edi+sizeof.Layer+Layer.c_size]
	jb        .cycle_6
	inc       ebx
.195:
	cmp       ebx,[edi]
	jb        .cycle_5
;copy errors to next level
	mov       eax,[esi+NeuralNetwork.errors]
	mov       edx,[esi+NeuralNetwork.errorsNext]
	mov       dword[esi+NeuralNetwork.errors],edx
	mov       dword[esi+NeuralNetwork.errorsNext],eax
	mov       eax,[esi+NeuralNetwork.deltas]
	mov       dword[ebp-12],eax
	xor       ebx,ebx ;i=0
	jmp       .201
align 4
.cycle_7:
	mov       ecx,[esi+NeuralNetwork.gradients]
	mov       eax,[edi+sizeof.Layer+Layer.biases]
	fld       qword[ecx+8*ebx]
	fadd      qword[eax+8*ebx]
	fstp      qword[eax+8*ebx]
	xor       eax,eax ;j=0
	mov       edx,[ebp-12] ;edx = deltas[i]
	jmp       .203
align 4
.cycle_8:
;	mov       ecx,[edx]
;	mov       ecx,[ecx+8*eax+4]
;	and       ecx,0x7ff80000
;	cmp       ecx,0x7ff80000
;	jne       @f
;	push      ebx
;	mov       ecx,[edx]
;	xor       ebx,ebx
;	mov       dword[ecx+8*eax],ebx
;	mov       dword[ecx+8*eax+4],ebx
;	pop       ebx
;@@:
	mov       ecx,[edx]
	fld       qword[ecx+8*eax]
	mov       ecx,[edi+Layer.weights]
	mov       ecx,[ecx+4*eax]
	fadd      qword[ecx+8*ebx]
	fstp      qword[ecx+8*ebx]
;	mov       ecx,[edi+Layer.weights]
;	mov       ecx,[ecx+4*eax]
;	mov       ecx,[ecx+8*ebx+4]
;	and       ecx,0x7ff80000
;	cmp       ecx,0x7ff80000
;	jne       @f
;	mov       ecx,[edi+Layer.weights]
;	mov       ecx,[ecx+4*eax]
;	push      edx
;	xor       edx,edx
;	mov       dword[ecx+8*ebx],edx
;	mov       dword[ecx+8*ebx+4],edx
;	pop       edx
;@@:
	inc       eax
.203:
	cmp       eax,[edi+Layer.c_size]
	jb        .cycle_8
	mov       eax,[ebp-12]
	stdcall   @$bdele$qpv, [eax]
	pop       ecx
	inc       ebx
	add       dword[ebp-12],4
.201:
	cmp       ebx,[edi+sizeof.Layer+Layer.c_size]
	jb        .cycle_7
	dec       dword[ebp-4]
	cmp       dword[ebp-4],0
	jge       .cycle_1
.end_f:
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       8


;NNP_GetMemSize:
; todo

;+ 8 NeuralNetwork* o
;+12 unsigned long fmt
;+16 char* m_data
align 16
NNP_GetMemData:
	push      ebp
	mov       ebp,esp
	add       esp,-12
	push      ebx esi edi
	cmp       dword[ebp+12],NNP_FF_JSON
	jne       .end_f
	mov       esi,[ebp+16]
	mov       byte[esi],0
	stdcall   [_strcat], esi,txt_QlearningRateQ_
	add       esp,8
	push      1
	push      PRECISION
	mov       eax,[ebp+8]
	push      dword[eax+NeuralNetwork.learningRate+4]
	push      dword[eax+NeuralNetwork.learningRate]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	stdcall   [_strcat], esi,txt_zap_nl
	add       esp,8
	stdcall   [_strcat], esi,txt_Qlayers_lengthQ
	add       esp,8
	push      1
	push      0
	mov       ecx,[ebp+8]
	fild      dword[ecx+NeuralNetwork.layers_length]
	add       esp,-8
	fstp      qword[esp]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	stdcall   [_strcat], esi,txt_zap_nl
	add       esp,8
.230:
	stdcall   [_strcat], esi,txt_QlayersQ
	add       esp,8
	xor       edi,edi ;i=0
	jmp       .232
align 4
.cycle_0:
	push      esi
	call      @@strlen$qpxc
	pop       ecx
	add       esi,eax
	stdcall   [_strcat], esi,txt_nl_t_Qc_sizeQ
	add       esp,8
	mov       ebx,edi
	imul      ebx,sizeof.Layer
	push      1
	push      0
	mov       edx,[ebp+8]
	mov       edx,[edx+NeuralNetwork.layers]
	xor       eax,eax
	add       esp,-8
	mov       ecx,[edx+ebx+Layer.c_size]
	mov       dword[ebp-12],ecx
	mov       dword[ebp-8],eax
	fild      qword[ebp-12]
	fstp      qword[esp]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	stdcall   [_strcat], esi,txt_zap_nl_t_Qn_sizeQ
	add       esp,8
	push      1
	push      0
	mov       ecx,[ebp+8]
	mov       ecx,[ecx+NeuralNetwork.layers]
	xor       edx,edx
	add       esp,-8
	mov       eax,[ecx+ebx+Layer.n_size]
	mov       dword[ebp-12],eax
	mov       dword[ebp-8],edx
	fild      qword[ebp-12]
	fstp      qword[esp]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	stdcall   [_strcat], esi,txt_zap_nl
	add       esp,8
	stdcall   [_strcat], esi,txt_t_QneuronsQ
	add       esp,8
	xor       ebx,ebx ;j=0
	jmp       .234
align 4
.cycle_1:
	test      ebx,ebx
	je        .235
	stdcall   [_strcat], esi,txt_zap_sp
	add       esp,8
.235:
	push      1
	push      PRECISION
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	mov       ecx,[edx+NeuralNetwork.layers]
	mov       eax,[ecx+eax+Layer.neurons]
	push      dword[eax+8*ebx+4]
	push      dword[eax+8*ebx]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	inc       ebx
.234:
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	mov       eax,[ebp+8]
	add       ecx,[eax+NeuralNetwork.layers]
	cmp       ebx,[ecx+Layer.c_size]
	jb        .cycle_1
	stdcall   [_strcat], esi,txt_sqbr_zap_nl
	add       esp,8
	stdcall   [_strcat], esi,txt_t_QbiasesQ
	add       esp,8
	xor       ebx,ebx ;j=0
	jmp       .238
align 4
.cycle_2:
	test      ebx,ebx
	je        .239
	stdcall   [_strcat], esi,txt_zap_sp
	add       esp,8
.239:
	push      1
	push      PRECISION
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	add       eax,[edx+NeuralNetwork.layers]
	mov       eax,[eax+Layer.biases]
	push      dword[eax+8*ebx+4]
	push      dword[eax+8*ebx]
	call      @@DoubleToStr$qduso
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	inc       ebx
.238:
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	mov       eax,[ebp+8]
	add       ecx,[eax+NeuralNetwork.layers]
	cmp       ebx,[ecx+Layer.c_size]
	jb        .cycle_2
	stdcall   [_strcat], esi,txt_sqbr_zap_t_QweightsQ
	add       esp,8
	mov       eax,[ebp+8]
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	add       ecx,[eax+NeuralNetwork.layers]
	cmp       dword[ecx+Layer.n_size],0
	je        .241
	xor       ebx,ebx
	jmp       .243
.242:
	test      ebx,ebx
	je        .244
	stdcall   [_strcat], esi,txt_zap_nl_t_t
	add       esp,8
.244:
	stdcall   [_strcat], esi,txt_sqbro
	add       esp,8
	xor       eax,eax
	mov       dword[ebp-4],eax
	jmp       .246
.245:
	cmp       dword[ebp-4],0
	je        .247
	stdcall   [_strcat], esi,txt_zap_sp
	add       esp,8
.247:
	push      1
	push      PRECISION
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       eax,[ebp+8]
	add       edx,[eax+NeuralNetwork.layers]
	mov       edx,[edx+Layer.weights]
	mov       ecx,[edx+4*ebx]
	mov       eax,[ebp-4]
	push      dword[ecx+8*eax+4]
	push      dword[ecx+8*eax]
@@:
	call      @@DoubleToStr$qduso
	dec       dword[esp+8] ;уменьшаем PRECISION
	jz        @f           ;для избежания зацикливания
	cmp       word[eax],'#'
	je        @b           ;если число не поместилось пробуем перевести с меньшей точностью
@@:
	add       esp,16
	stdcall   [_strcat], esi,eax
	add       esp,8
	inc       dword[ebp-4]
.246:
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	mov       eax,[ebp+8]
	add       ecx,[eax+NeuralNetwork.layers]
	mov       ecx,[ecx+Layer.n_size]
	cmp       ecx,[ebp-4]
	ja        .245
	stdcall   [_strcat], esi,txt_sqbr
	add       esp,8
	inc       ebx
.243:
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       ecx,[ebp+8]
	add       eax,[ecx+NeuralNetwork.layers]
	cmp       ebx,[eax+Layer.c_size]
	jb        .242
.241:
	stdcall   [_strcat], esi,txt_sqbr_fbr_zap
	add       esp,8
	inc       edi
.232:
	mov       eax,[ebp+8]
	cmp       edi,[eax+NeuralNetwork.layers_length]
	jb        .cycle_0
	stdcall   [_strcat], esi,txt_nl_t_sqbr
	add       esp,8
.end_f:
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       12

;+ 8 NeuralNetwork* o
align 16
NNP_GetMaxLLen:
	push      ebp
	mov       ebp,esp
	push      ebx esi
	mov       ebx,[ebp+8]
	cmp       dword[ebx+NeuralNetwork.layers_length],1
	jge       .1
	xor       eax,eax
	jmp       .end_f
.1:
	mov       edx,[ebx+NeuralNetwork.layers]
	lea       eax,[edx+sizeof.Layer]
	mov       ecx,[edx]
	mov       edx,1 ;i=1
	jmp       .3
.cycle_0: ;for (i=1; i < o->layers_length; i++)
	mov       esi,[eax]
	cmp       esi,ecx
	jbe       .2
	mov       ecx,esi
.2:
	inc       edx
	add       eax,sizeof.Layer
.3:
	cmp       edx,[ebx+NeuralNetwork.layers_length]
	jl        .cycle_0
	mov       eax,ecx
.end_f:
	pop       esi ebx ebp
	ret       4

;+ 8 NeuralNetwork* o
;+12 unsigned long fmt
;+16 void* m_data
align 16
NNP_SetMemData:
	push      ebp
	mov       ebp,esp
	add       esp,-12
	push      ebx esi edi
	mov       eax,[ebp+16]
	mov       edx,[ebp+12]
;	cmp       edx,NNP_FF_BIN
;	jne       .191
;...
;.191:
	cmp       edx,NNP_FF_JSON
	jne       .198
.199:
	stdcall   @@strstr$qpxct1, eax,txt_learningRate
	add       esp,8
	mov       esi,eax
	stdcall   @@strstr$qpxct1, esi,txt_layers_length
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .200
	mov       eax,1
	jmp       .193
.200:
	stdcall   @@strchr$qpxci, esi,':'
	add       esp,8
	mov       ebx,eax
	test      ebx,ebx
	jne       .201
	mov       eax,2
	jmp       .193
.201:
	inc       ebx
	stdcall   @@strchr$qpxci, esi,','
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .202
	mov       eax,3
	jmp       .193
.202:
	mov       byte[esi],0
	inc       esi
	stdcall   @@StrToInt$qpc, ebx
	pop       ecx
	mov       dword[ebp-4],eax
	;lea       eax,[ebp-4]
	;stdcall   [_scanf], ebx,txt_pri_zap,eax
	;add       esp,12
	mov       eax,[ebp+8]
	mov       edx,[eax+12]
	cmp       edx,[ebp-4]
	je        .203
	mov       eax,txt_err_layers_neq
	jmp       .193
.203:
	xor       edi,edi ;i=0
	jmp       .205
.204: ;for(i=0;i<o->layers_length;i++)
	stdcall   @@strstr$qpxct1, esi,txt_c_size
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne        .206
	mov       eax,txt_err_c_size
	jmp       .193
.206:
	stdcall   @@strchr$qpxci, esi,':'
	add       esp,8
	mov       ebx,eax
	test      ebx,ebx
	jne        .207
	mov       eax,6
	jmp       .193
.207:
	inc       ebx
	stdcall   @@strchr$qpxci, esi,','
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne        .208
	mov       eax,7
	jmp       .193
.208:
	mov       byte[esi],0
	inc       esi
	stdcall   @@StrToInt$qpc, ebx
	pop       ecx
	mov       dword[ebp-4],eax
	stdcall   @@strstr$qpxct1, esi,txt_n_size
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .209
	mov       eax,8
	jmp       .193
.209:
	stdcall   @@strchr$qpxci, esi,':'
	add       esp,8
	mov       ebx,eax
	test      ebx,ebx
	jne       .210
	mov       eax,9
	jmp       .193
.210:
	inc       ebx
	stdcall   @@strchr$qpxci, esi,','
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .211
	mov       eax,10
	jmp       .193
.211:
	mov       byte[esi],0
	inc       esi
	stdcall   @@StrToInt$qpc,ebx
	pop       ecx
	mov       dword[ebp-8],eax
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	add       eax,[edx+NeuralNetwork.layers]
	mov       edx,[eax+Layer.c_size]
	cmp       edx,[ebp-4]
	jne       .213
	mov       edx,[eax+Layer.n_size]
	cmp       edx,[ebp-8]
	je        .214
.213:
	mov       ecx,[ebp+8]
	stdcall   NNP_GetMaxLLen,ecx
	mov       ecx,edi
	imul      ecx,sizeof.Layer
	mov       ebx,eax
	mov       eax,[ebp+8]
	mov       edx,[eax+NeuralNetwork.layers]
	add       edx,ecx
	stdcall   Layer_Destroy,edx
	mov       eax,[ebp-8]
	push      eax
	mov       edx,[ebp-4]
	push      edx
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       ecx,[ebp+8]
	mov       eax,[ecx+NeuralNetwork.layers]
	add       eax,edx
	stdcall   Layer_Create,eax
	cmp       ebx,[ebp-4] ;if(n>s || k>s)
	jb        .215
	cmp       ebx,[ebp-8]
	jae       .214
.215:
	mov       edx,[ebp+8]
	mov       ecx,[edx+NeuralNetwork.errors]
	cmp       ecx,[edx+NeuralNetwork.errorsNext]
	jl        @f
	mov       ecx,[edx+NeuralNetwork.errorsNext]
@@:
	mov       ebx,[ebp-4]
	cmp       ebx,[ebp-8]
	jge       @f
	mov       ebx,[ebp-8]
@@:
	shl       ebx,4
	stdcall   [mem.realloc], ecx,ebx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.errors],eax
	shr       ebx,1
	add       eax,ebx
	mov       dword[edx+NeuralNetwork.errorsNext],eax
	stdcall   [mem.realloc], [edx+NeuralNetwork.gradients],ebx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.gradients],eax
	shr       ebx,1
	stdcall   [mem.realloc], [edx+NeuralNetwork.deltas],ebx
	mov       edx,[ebp+8]
	mov       dword[edx+NeuralNetwork.deltas],eax
.214:
	stdcall   @@strstr$qpxct1, esi,txt_biases
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .216
	mov       eax,11
	jmp       .193
.216:
	stdcall   @@strchr$qpxci, esi,'['
	add       esp,8
	mov       ebx,eax
	test      ebx,ebx
	jne        .217
	mov       eax,txt_err_sqbrl_b1
	jmp       .193
.217:
	inc       ebx
	xor       edx,edx
	mov       dword[ebp-8],edx
	jmp        .219
.218:
	dec       edx
	cmp       eax,edx
	jae        .220
	stdcall   @@strchr$qpxci, ebx,','
	add       esp,8
	mov       esi,eax
	jmp        .221
.220:
	stdcall   @@strchr$qpxci, ebx,']'
	add       esp,8
	mov       esi,eax
.221:
	test      esi,esi
	jne        .222
	mov       eax,13
	jmp       .193
.222:
	mov       byte[esi],0
	stdcall   @@StrToDouble$qpc,ebx
	pop       ecx
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       ecx,[ebp+8]
	lea       ebx,[esi+1]
	mov       eax,[ecx+NeuralNetwork.layers]
	mov       ecx,[ebp-8]
	mov       edx,[eax+edx+Layer.biases]
	fstp      qword[edx+8*ecx]
	inc       dword[ebp-8]
.219:
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       ecx,[ebp+8]
	add       edx,[ecx+NeuralNetwork.layers]
	mov       edx,[edx+Layer.c_size]
	mov       eax,[ebp-8]
	cmp       edx,eax
	ja        .218
	mov       esi,ebx
	stdcall   @@strstr$qpxct1, esi,txt_weights
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .224
	mov       eax,14
	jmp       .193
.224:
	stdcall   @@strchr$qpxci, esi,'['
	add       esp,8
	mov       esi,eax
	test      esi,esi
	jne       .225
	mov       eax,txt_err_sqbrl_w1
	jmp       .193
.225:
	inc       esi
	xor       edx,edx
	mov       dword[ebp-8],edx ;k=0
	jmp       .227
.226: ;for(k=0;k<o->layers[i].c_size;k++)

	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	add       eax,[edx+NeuralNetwork.layers]
	mov       eax,[eax+Layer.n_size]
	or        eax,eax
	jnz       .end_null_we
	inc       dword[ebp-8] ;k++
	jmp       .227 ;if 'weights' is null array
.end_null_we:

	stdcall   @@strchr$qpxci, esi,'['
	add       esp,8
	mov       ebx,eax
	test      ebx,ebx
	jne       .228
	mov       eax,txt_err_sqbrl_w2
	jmp       .193
.228:
	inc       ebx
	xor       edx,edx
	mov       dword[ebp-12],edx ;j=0
	jmp       .230
.229: ;for(j=0;j<o->layers[i].n_size;j++)
	dec       edx
	cmp       eax,edx ;eax = j, edx = n_size-1
	jae       .231
	stdcall   @@strchr$qpxci, ebx,','
	add       esp,8
	mov       esi,eax
	jmp       .232
.231:
	stdcall   @@strchr$qpxci, ebx,']'
	add       esp,8
	mov       esi,eax
.232:
	test      esi,esi
	jne       .233
	mov       eax,txt_err_sqbrr_w2
	jmp       .193
.233:
	mov       byte[esi],0
	stdcall   @@StrToDouble$qpc,ebx
	pop       ecx
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       ecx,[ebp+8]
	lea       ebx,[esi+1]
	mov       eax,[ecx+NeuralNetwork.layers]
	mov       ecx,[ebp-8]
	mov       edx,[eax+edx+Layer.weights]
	mov       eax,[edx+4*ecx]
	mov       edx,[ebp-12]
	fstp      qword[eax+8*edx]
	inc       dword[ebp-12]
.230:
	mov       edx,edi
	imul      edx,sizeof.Layer
	mov       ecx,[ebp+8]
	add       edx,[ecx+NeuralNetwork.layers]
	mov       edx,[edx+Layer.n_size]
	mov       eax,[ebp-12]
	cmp       edx,eax
	ja        .229
	mov       esi,ebx
	inc       dword[ebp-8]
.227:
	mov       eax,edi
	imul      eax,sizeof.Layer
	mov       edx,[ebp+8]
	add       eax,[edx+NeuralNetwork.layers]
	mov       eax,[eax+Layer.c_size]
	cmp       eax,[ebp-8]
	ja        .226
	inc       edi
.205:
	mov       edx,[ebp+8]
	cmp       edi,[edx+NeuralNetwork.layers_length]
	jb        .204
	xor       eax,eax
	jmp        .193
.198:
	mov       eax,1000
.193:
	pop       edi esi ebx
	mov       esp,ebp
	pop       ebp
	ret       12

align 16
Layer_Destroy:
	push      ebp
	mov       ebp,esp
	push      ebx esi
	mov       esi,[ebp+8]
	push      dword[esi+Layer.neurons]
	call      @$bdele$qpv
	pop       ecx
	push      dword[esi+Layer.biases]
	call      @$bdele$qpv
	pop       ecx
	xor       ebx,ebx
	jmp       .143
.142:
	mov       eax,[esi+Layer.weights]
	push      dword[eax+4*ebx]
	call      @$bdele$qpv
	pop       ecx
	inc       ebx
.143:
	cmp       ebx,[esi+Layer.c_size]
	jb        .142
	push      dword[esi+Layer.weights]
	call      @$bdele$qpv
	pop       ecx
.145:
	pop       esi ebx ebp
	ret       4

align 16
NNP_Destroy:
	push      ebp
	mov       ebp,esp
	push      ebx esi
	mov       esi,[ebp+8]
	xor       ebx,ebx
	jmp       .232
.231:
	mov       eax,ebx
	imul      eax,sizeof.Layer
	add       eax,[esi+NeuralNetwork.layers]
	push      eax
	call      Layer_Destroy
	inc       ebx
.232:
	cmp       ebx,[esi+NeuralNetwork.layers_length]
	jb        .231
	push      dword[esi+NeuralNetwork.layers]
	call      @$bdele$qpv
	pop       ecx
;
	mov       ecx,[esi+NeuralNetwork.errors]
	cmp       ecx,[esi+NeuralNetwork.errorsNext]
	jl        @f
	mov       ecx,[esi+NeuralNetwork.errorsNext]
@@:
	push      ecx
	call      @$bdele$qpv
	pop       ecx
;
	push      dword[esi+NeuralNetwork.gradients]
	call      @$bdele$qpv
	pop       ecx
;
	push      dword[esi+NeuralNetwork.deltas]
	call      @$bdele$qpv
	pop       ecx
	pop       esi ebx ebp
	ret       4


align 16
EXPORTS:
	dd sz_lib_init, lib_init
	dd sz_create, NNP_Create
	dd sz_reset, NNP_Reset
	dd sz_feedforward, NNP_FeedForward
	dd sz_backpropagation, NNP_BackPropagation
	dd sz_getmemdata, NNP_GetMemData
	dd sz_setmemdata, NNP_SetMemData
	dd sz_destroy, NNP_Destroy
	dd 0,0
	sz_lib_init db 'lib_init',0
	sz_create db 'NNP_Create',0
	sz_reset db 'NNP_Reset',0
	sz_feedforward db 'NNP_FeedForward',0
	sz_backpropagation db 'NNP_BackPropagation',0
	sz_getmemdata db 'NNP_GetMemData',0
	sz_setmemdata db 'NNP_SetMemData',0
	sz_destroy db 'NNP_Destroy',0

align 16
@IMPORT:

library \
	libc, 'libc.obj'

import libc, \
_strcat, 'strcat',\
_exp,    'exp'
;_scanf,  'scanf',\ ;???
