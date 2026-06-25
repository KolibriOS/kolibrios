; -----------------------------
; Draws a button with a text label
; input: pLabel - pointer to the LABEL structure
;        id - button identifier
;        xPosition - button position X
;        yPosition - button position Y
;        bWidth - button width
;        bHeight - button height
; -----------------------------
proc draw.Button pLabel, id, xPosition, yPosition, bWidth, bHeight
    mcall   SF_DEFINE_BUTTON, <[xPosition], [bWidth]>, <[yPosition], [bHeight]>, [id], [button_color]
    ; position X for text positioning
    mov     eax, [bHeight]
    shr     eax, 1
    add     [yPosition], eax
    ; position Y for text positioning
    mov     eax, [pLabel]
    mov     eax, [eax+LABEL.size]
    neg     eax
    mul     [configFont.width]
    add     eax, [bWidth]
    add     eax, 4
    shr     eax, 1
    add     [xPosition], eax
    ; draw text on the button
    stdcall draw.Label, [pLabel], [xPosition], [yPosition]
    ret
endp
; -----------------------------
; Draws text on position yPosition and centered on the width of the screen
; considering the length of the text and the offset in symbols
; input: pLabel - pointer to the LABEL structure
;        yPosition - text position Y
;        countOffset - number of symbols for offset
; -----------------------------
proc draw.Navigation pLabel, yPosition, countOffset
    mov     eax, [pLabel]
    mov     eax, [eax + LABEL.size]
    sub     eax, [countOffset]
    stdcall draw.GetNavigationX, eax
    stdcall draw.Label, [pLabel], eax, [yPosition], 0
    ret
endp
; -----------------------------
; Draws a number on position yPosition and centered on the width of the screen
; considering the length of the number and the offset in symbols
; input: pLabel - pointer to the LABEL structure
;        yPosition - text position Y
;        countOffset - number of symbols for offset
; -----------------------------
proc draw.NavigationNumber pLabel, yPosition, countOffset
    mov     eax, [pLabel]
    mov     eax, [eax + LABEL.size]
    sub     eax, [countOffset]
    stdcall draw.GetNavigationX, eax
    stdcall draw.Number, [pLabel], eax, [yPosition], 0
    ret
endp
; -----------------------------
; Returns the X coordinate for the text positioning on the center of the screen
; considering the offset in symbols
; input: countOffset - number of symbols for offset
; output: eax - X coordinate
; -----------------------------
proc draw.GetNavigationX countOffset
    mov     eax, [countOffset]
    neg     eax
    mul     [configFont.width]
    add     eax, [window_width]
    sub     eax, 10
    shr     eax, 1
    ret
endp
; -----------------------------
; Builds the coordinates of the text label adding the offset in symbols along X
; input: xPosition - X coordinate
;        yPosition - Y coordinate
;        countOffset - number of symbols for offset
; output: eax - X coordinate in format X*65536+Y
; -----------------------------
proc draw._prepareCoord xPosition, yPosition, countOffset
    mov     eax, [countOffset]
    mul     [configFont.width]
    add     eax, [xPosition]
    shl     eax, 16

    mov     ebx, [configFont.height]
    shr     ebx, 1
    sub     [yPosition], ebx
    add     eax, [yPosition]
    ret
endp
; -----------------------------
; Draws the text at the X and Y positions and shifts the offset in symbols along X
; input: pLabel - pointer to the LABEL structure
;        xPosition - text position X
;        yPosition - text position Y
;        countOffset - number of symbols for offset
; -----------------------------
proc draw.Label pLabel, xPosition, yPosition, countOffset
    stdcall draw._prepareCoord, [xPosition], [yPosition], [countOffset]
    mov     ebx, eax
    mov     eax, [pLabel]
    mov     ecx, [configFont.mask]
    or      ecx, [eax + LABEL.color]
    mov     edx, [eax + LABEL.value]
    mcall   SF_DRAW_TEXT
    ret
endp
; -----------------------------
; Draws the number at the X and Y positions and shifts the offset in symbols along X
; input: pLabel - pointer to the LABEL structure
;        xPosition - text position X
;        yPosition - text position Y
;        countOffset - number of symbols for offset
; -----------------------------
proc draw.Number pLabel, xPosition, yPosition, countOffset
    stdcall draw._prepareCoord, [xPosition], [yPosition], [countOffset]
    mov     edx, eax
    mov     eax, [pLabel]
    mov     ebx, [eax + LABEL.size]
    shl     ebx, 16
    mov     ecx, [eax + LABEL.value]
    mov     esi, [configFont.maskNumber]
    or      esi, [eax + LABEL.color]
    mcall   SF_DRAW_NUMBER, , , , ,[background_color]
    ret
endp
; -----------------------------
; Set configuration of the font depending on the size of the square side
; setting the font size and the font mask text and number
; input: squareSideLength - size of the square side
; -----------------------------
proc draw.setConfigFont squareSideLength
    cmp     [squareSideLength], MIN_SQUARE_SIDE_LENGTH_FONT
    jg      @f
    cmp     [configFont.flag], FONT_SMALL
    je      .return
    mov     [configFont.flag], FONT_SMALL
    jmp     .set
  @@:
    cmp     [configFont.flag], FONT_LARGE
    je      .return
    mov     [configFont.flag], FONT_LARGE
  .set:
    mov     eax, [configFont.flag]
    lea     ebx, [eax + 8]
    shl     ebx, 28
    mov     [configFont.mask], ebx
    lea     ebx, [eax*2 + 6]
    mov     [configFont.width], ebx
    lea     ebx, [7 + eax*4]
    lea     ebx, [ebx + eax*2]
    mov     [configFont.height], ebx
    lea     ebx, [eax + 4]
    shl     ebx, 28
    mov     [configFont.maskNumber], ebx
  .return:
    ret
endp
; -----------------------------
; Initializes an array of packed coordinate values for a grid of cells
; ((sizeCell * index + baseOffset + 1) << 16) | (sizeCell - 1)
; input: baseOffset - base offset coordinate for the grid
;        countCell - number of cells to proccess
;        sizeCell - size of cell
;        addressSave - pointer to destination array
; -----------------------------
proc draw.InitCoordSquare baseOffset, countCell, sizeCell, addressSave
    ; load arguments into registers
    mov     ebx, [sizeCell]
    mov     ecx, [countCell]
    mov     esi, [baseOffset]
    mov     edi, [addressSave]
    ; check if count is zero
    test    ecx, ecx
    jz      .return
  .loop:
    ; decrement index (loop backwards)
    dec     ecx
    ; calculate start coordinate: (index * size) + base + 1
    mov     eax, ecx
    mul     ebx
    add     eax, esi
    inc     eax
    ; pack coordinates: (start << 16) | (size - 1)
    shl     eax, 16
    add     eax, ebx
    dec     eax
    ; save result in array
    mov     [edi + ecx*4], eax
    test    ecx, ecx
    jnz     .loop
  .return:
    ret
endp
; -----------------------------
; Draws a square on the cell position and fills it with color
; input: yxIndex - x and y position of cells
;        color - color to fill
; -----------------------------
proc draw.Square uses ebx ecx, yxIndex, color
    movzx   ebx, byte [yxIndex]
    movzx   ecx, byte [yxIndex + 1]
    mcall   SF_DRAW_RECT, [COORD_SQUARE.x + ebx*4], [COORD_SQUARE.y + ecx*4], [color]
    ret
endp
; -----------------------------
; Draws an picture using a bitmap in the specified grid positions
; input: x - horizontal position
;        width - horizontal number of cells
;        y - vertical position
;        height - vertical number of cells
;        color - color to fill
;        picture - pointer to the picture bitmap
; -----------------------------
proc draw.Picture uses ebx ecx, x, width, y, height, color, picture
    ; cl this is X limit = x + width
    mov     cl, byte [x]
    add     cl, byte [width]
    ; ch this is Y limit = y + height
    mov     ch, byte [y]
    add     ch, byte [height]
    mov     edx, [color]
    mov     esi, [picture]
    ; set begin coordinate bh = y, bl = x
    movzx   ebx, byte [y]
    shl     ebx, 8
  .y_loop:
    mov     bl, byte [x]
  .x_loop:
    ; index bit
    mov     edi, 7
  .bit_loop:
    bt      [esi], edi
    jnc     .skip_bit
    stdcall draw.Square, ebx, edx
  .skip_bit:
    ; increment x
    inc     bl
    ; decrement index bit
    dec     edi
    jns     .bit_loop
    ; next byte picture
    inc     esi
    cmpl    bl, cl, .x_loop
    inc     bh
    cmpl    bh, ch, .y_loop
    ret
endp