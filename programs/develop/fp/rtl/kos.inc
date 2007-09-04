{cp866}

{ User interface }
procedure kos_definewindow(x, y, w, h: Word; style, header, clframe: DWord); assembler; register;
asm
  pushl %ebx
  pushl %ecx
  pushl %edx
  pushl %esi
  pushl %edi
  movl  %eax, %ebx
  xchgl %edx, %ecx
  movl  header, %esi
  shll  $16, %ebx
  shll  $16, %ecx
  movl  clframe, %edi
  movw  %dx, %bx
  movw  h, %cx
  xorl  %eax, %eax
  movl  style, %edx
  decl  %ebx  {уменьшить ширину на 1}
  decl  %ecx  {уменьшить высоту на 1}
  int   $0x40
  popl %edi
  popl %esi
  popl %edx
  popl %ecx
  popl %ebx
end;

procedure kos_movewindow(x, y, w, h: DWord); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  pushl %esi
  movl  %eax, %ebx
  xchgl %ecx, %edx
  movl  $67, %eax
  movl  h, %esi
  decl  %edx  {уменьшить ширину на 1}
  decl  %esi  {уменьшить высоту на 1}
  int   $0x40
  popl  %esi
  popl  %edx
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

function kos_getkey(): DWord; assembler; register;
asm
  movl $2, %eax
  int  $0x40
end;

function kos_getevent(wait: Boolean = True): DWord; assembler; register;
asm
  andl $1, %eax
  xorb $1, %al
  addl $10, %eax
  int  $0x40
end;

function kos_waitevent(timeout: DWord): DWord; assembler; register;
asm
  pushl %ebx
  movl  $23, %ebx
  xchgl %eax, %ebx
  int  $0x40
  popl  %ebx
end;

function kos_getbutton(): DWord; assembler; register;
asm
  movl  $17, %eax
  int   $0x40
  shrl  $8, %eax
  andl  $0xFF, %eax
end;

function kos_getmousepos(): TKosPoint; assembler; register;
{@return: x*65536 + y}
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %eax
  movl  $37, %eax
  xorl  %ebx, %ebx
  int   $0x40
  movswl %ax, %ecx
  popl  %ebx
  shrl  $16, %eax
  movl  %ecx, TKosPoint.Y(%ebx)
  movl  %eax, TKosPoint.X(%ebx)
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

function kos_getmousewinpos(): TKosPoint; assembler; register;
{@return: x*65536 + y}
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %eax
  movl  $37, %eax
  movl  $1, %ebx
  int   $0x40
  movswl %ax, %ecx
  popl  %ebx
  shrl  $16, %eax
  movl  %ecx, TKosPoint.Y(%ebx)
  movl  %eax, TKosPoint.X(%ebx)
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

function kos_getmousebuttons(): DWord; assembler; register;
{@return:
  бит 0 установлен = левая кнопка нажата
  бит 1 установлен = правая кнопка нажата
  бит 2 установлен = средняя кнопка нажата
  бит 3 установлен = 4-я кнопка нажата
  бит 4 установлен = 5-я кнопка нажата}
asm
  pushl %ebx
  movl  $37, %eax
  movl  $2, %ebx
  int   $0x40
  popl  %ebx
end;

procedure kos_maskevents(mask: DWord); assembler; register;
asm
  pushl %ebx
  xchgl %eax, %ebx
  movl  $40, %eax
  int   $0x40
  xchgl %eax, %ebx
  popl  %ebx
end;

procedure kos_setcaption(caption: PChar); assembler; register;
asm
  pushl %ecx
  pushl %ebx
  xchgl %eax, %ecx
  movl $1, %ebx
  movl $71, %eax
  int  $0x40
  xchgl %eax, %ecx
  popl %ebx
  popl %ecx
end;


{ Graphics }

function kos_screensize(): TKosPoint; assembler; register;
asm
  pushl %eax
  pushl %ecx
  pushl %eax
  movl  $14, %eax
  int   $0x40
  movswl %ax, %ecx
  popl  %ebx
  shrl  $16, %eax
  movl  %ecx, TKosPoint.Y(%ebx)
  movl  %eax, TKosPoint.X(%ebx)
  popl  %ecx
  popl  %eax
end;

procedure kos_begindraw(); assembler; register;
asm
  pushl %ebx
  movl  $12, %eax
  movl  $1, %ebx
  int   $0x40
  popl  %ebx
end;

procedure kos_enddraw(); assembler; register;
asm
  pushl %ebx
  movl  $12, %eax
  movl  $2, %ebx
  int   $0x40
  popl  %ebx
end;

procedure kos_putpixel(x, y: Word; color: DWord); assembler; register;
asm
  pushl %ebx
  movl  %eax, %ebx
  xchgl %edx, %ecx
  movl  $1, %eax
  int   $0x40
  xchgl %edx, %ecx
  popl  %ebx
end;

procedure kos_drawtext(x, y: Word; text: String; flags, bgcolor: DWord); assembler; register;
label nobg;
asm
  pusha
  shll  $16, %eax
  pushl %ecx
  movl  flags, %ecx  {флаги, цвет}
  movl  bgcolor, %edi
  movw  %dx, %ax
  andl   $0x7FFFFFFF, %ecx
  btl   $31, %edi
  jnc   nobg
  orl   $0x40000000, %ecx
nobg:
  popl  %edx
  movl  %eax, %ebx    {координаты}
  movzbl (%edx), %esi {длина строки}
  movl  $4, %eax      {номер функции}
  incl  %edx          {указатель на строку}
  andl  $0xFFFFFF, %edi
  int   $0x40
  popa
end;

procedure kos_drawrect(x, y, w, h: Word; color: DWord); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  movl  %eax, %ebx
  xchgl %edx, %ecx
  shll  $16, %ebx
  shll  $16, %ecx
  movl  $13, %eax
  movw  %dx, %bx
  movw  h, %cx
  movl  color, %edx
  int   $0x40
  popl %edx
  popl %ecx
  popl %ebx
  popl %eax
end;

procedure kos_drawline(x1, y1, x2, y2: Word; color: DWord = $000000); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx

  xchgl %eax, %ecx
  xchgl %ecx, %edx
  movl  color, %ebx
  {eax - x2, ebx - color, ecx - y1, edx - x1}
  shll  $16, %ecx
  shll  $16, %edx
  movw  %ax, %dx
  movw  y2, %cx
  movl  $38, %eax
  xchgl %ebx, %edx
  int   $0x40

  popl  %edx
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

procedure kos_drawimage(x, y, w, h, depth: DWord; image: Pointer; palette: Pointer; xoffset: DWord); assembler; register;
asm
  pusha
  shll  $16, %eax
  shll  $16, %ecx
  orl   %eax, %edx
  orl   h, %ecx
  movl  depth, %esi
  movl  image, %ebx
  movl  palette, %edi
  movl  xoffset, %ebp
  movl  $65, %eax
  int   $0x40
  popa
end;

procedure kos_drawimage24(x, y, w, h: DWord; image: Pointer); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  pushl %edx
  shll  $16, %eax
  shll  $16, %ecx
  orl   %eax, %edx
  orl   h, %ecx
  movl  image, %ebx
  movl  $7, %eax
  int   $0x40
  popl  %edx
  popl  %ecx
  popl  %ebx
  popl  %eax
end;


{ Work with system }

{ Work with system - System services }

function kos_killthread(tid: TThreadID): Boolean; assembler; register;
asm
  pushl %ecx
  pushl %ebx
  movl  $18, %ecx
  movl  $18, %ebx
  xchgl %eax, %ecx
  int   $0x40
  andl  $1, %eax
  popl  %ebx
  popl  %ecx
  xorb  $1, %al
end;

procedure kos_setactivewindow(slot: TThreadSlot); assembler; register;
asm
  pushl %ecx
  pushl %ebx
  movl  $18, %ecx
  movl  $3, %ebx
  xchgl %eax, %ecx
  int   $0x40
  xchgl %eax, %ecx
  popl  %ebx
  popl  %ecx
end;

{$ifdef EMULATOR}
function kos_getthreadslot(tid: TThreadID): TThreadSlot;
var
  ThreadInfo: TKosThreadInfo;
  HighThreadSlot: TThreadSlot;
begin
  Result := 0;
  repeat
    Inc(Result);
    HighThreadSlot := kos_threadinfo(@ThreadInfo, Result);
  until (Result > HighThreadSlot) or (ThreadInfo.ThreadID = tid);
end;

{$else}

function kos_getthreadslot(tid: TThreadID): TThreadSlot; assembler; register;
asm
  pushl %ecx
  pushl %ebx
  movl  $18, %ecx
  movl  $21, %ebx
  xchgl %eax, %ecx
  int   $0x40
  popl  %ebx
  popl  %ecx
end;
{$endif}

{ Work with system - Set system parameters }

procedure kos_enablepci(); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  movl  $21, %eax
  movl  $12, %ebx
  movl  $1, %ecx
  int   $0x40
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

{ Work with system - Internal system services }

procedure kos_switchthread(); assembler; register;
asm
  pushl %eax
  pushl %ebx
  movl  $68, %eax
  movl  $1, %ebx
  int   $0x40
  popl  %ebx
  popl  %eax
end;

function kos_initheap(): DWord; assembler; register;
asm
  pushl %ebx
  movl  $68, %eax
  movl  $11, %ebx
  int   $0x40
  popl  %ebx
end;

function kos_alloc(size: DWord): Pointer; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  %eax, %ecx
  movl  $68, %eax
  movl  $12, %ebx
  int   $0x40
  popl  %ecx
  popl  %ebx
end;

function kos_free(ptr: Pointer): Boolean; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  %eax, %ecx
  movl  $68, %eax
  movl  $13, %ebx
  int   $0x40
  popl  %ecx
  popl  %ebx
end;

function kos_loaddriver(name: PChar): THandle; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  %eax, %ecx
  movl  $68, %eax
  movl  $16, %ebx
  int   $0x40
  popl  %ecx
  popl  %ebx
end;


{ Processes and threads }

function kos_threadinfo(info: PKosThreadInfo; slot: TThreadSlot): DWord; assembler; register;
asm
  pushl %ebx
  movl  %eax, %ebx
  xchgl %edx, %ecx
  movl  $9, %eax
  int   $0x40
  xchgl %edx, %ecx
  popl  %ebx
end;

function kos_newthread(entry, stack: Pointer): TThreadID; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  $1, %ebx
  movl  %eax, %ecx
  movl  $51, %eax
  int   $0x40
  popl  %ecx
  popl  %ebx
end;

procedure kos_initipc(ipc: PKosIPC; size: DWord); assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  $60, %ecx
  movl  $1, %ebx
  xchgl %eax, %ecx
  int   $0x40
  popl  %ecx
  popl  %ebx
end;

function kos_sendmsg(tid: TThreadID; msg: Pointer; size: DWord): DWord; assembler; register;
{@return:
  0 - успешно
  1 - приёмник не определил буфер для IPC-сообщений
      (может быть, ещё не успел, а может быть, это не тот поток, который нужен)
  2 - приёмник заблокировал IPC-буфер; попробуйте немного подождать
  3 - переполнение IPC-буфера приёмника
  4 - процесса/потока с таким PID не существует}
asm
  pushl %esi
  pushl %ebx
  movl  $60, %esi
  movl  $2, %ebx
  xchgl %ecx, %esi
  xchgl %eax, %ecx
  int   $0x40
  xchgl %ecx, %esi
  popl  %ebx
  popl  %esi
end;

function kos_resizemem(size: DWord): Boolean; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  movl  %eax, %ecx
  movl  $64, %eax
  movl  $1, %ebx
  int   $0x40
  xorb  $1, %al
  popl  %ecx
  popl  %ebx
end;


{ File system }
{ File system - Work with the current folder }

procedure kos_setdir(path: PChar); assembler; register;
asm
  pushl %ecx
  pushl %ebx
  movl  $30, %ecx
  movl  $1, %ebx
  xchgl %eax, %ecx
  int   $0x40
  popl  %ebx
  popl  %ecx
end;

function kos_getdir(path: PChar; size: DWord): DWord; assembler; register;
asm
  pushl %ecx
  pushl %ebx
  movl  $30, %ecx
  movl  $2, %ebx
  xchgl %eax, %ecx
  int   $0x40
  popl  %ebx
  popl  %ecx
end;

{ File system - Work with file system with long names support }

function kos_readfile(kosfile: PKosFile; var readed: Longint): DWord; assembler; register;
asm
  pushl %ebx
  movl  $70, %ebx
  xchgl %eax, %ebx
  movl  $0, (%ebx)
  int   $0x40
  movl  %ebx, (%edx)
  popl  %ebx
end;

function kos_rewritefile(kosfile: PKosFile; var writed: Longint): DWord; assembler; register;
asm
  pushl %ebx
  movl  $70, %ebx
  xchgl %eax, %ebx
  movl  $2, (%ebx)
  int   $0x40
  movl  %ebx, (%edx)
  popl  %ebx
end;

function kos_writefile(kosfile: PKosFile; var writed: Longint): DWord; assembler; register;
asm
  pushl %ebx
  movl  $70, %ebx
  xchgl %eax, %ebx
  movl  $3, (%ebx)
  int   $0x40
  movl  %ebx, (%edx)
  popl  %ebx
end;

function kos_fileinfo(kosfile: PKosFile): DWord; assembler; register;
asm
  pushl %ebx
  movl  $70, %ebx
  xchgl %eax, %ebx
  movl  $5, (%ebx)
  int   $0x40
  popl  %ebx
end;


{ Work with hardware }

function kos_readport(index: DWord): DWord; assembler; register;
label ok, exit;
asm
  pushl %ecx
  pushl %ebx
  xchgl %eax, %ecx         {index}
  movl  $43, %eax
  orl   $0x80000000, %ecx  {index}
  int   $0x40
  orl   %eax, %eax
  jzl   ok
  movl  $-1, %eax
  jmp   exit
ok:
  movl  %ebx, %eax
exit:
  popl  %ebx
  popl  %ecx
end;

procedure kos_writeport(index, value: DWord); assembler; register;
asm
  pushl %eax
  pushl %ebx
  pushl %ecx
  xchgl %edx, %ebx  {value}
  xchgl %eax, %ecx  {index}
  movl  $43, %eax
  int   $0x40
  xchgl %edx, %ebx
  popl  %ecx
  popl  %ebx
  popl  %eax
end;

function kos_reserveport(port: DWord): Boolean; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  pushl %edx
  movl  %eax, %ecx  {port}
  movl  $46, %eax
  movl  %ecx, %edx  {port}
  xorl  %ebx, %ebx
  int   $0x40
  xorb  $1, %al
  popl  %edx
  popl  %ecx
  popl  %ebx
end;

{ Work with hardware - Low-level access to PCI}

function kos_lastpcibus(): Byte; assembler; register;
asm
  pushl %ebx
  movl  $62, %eax
  movl  $1, %ebx
  int   $0x40
  popl  %ebx
end;

function kos_readpcib(bus, dev, func, reg: Byte): Byte; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  pushl %edx
  shlb  $3, %dl   {dev}
  movb  %al, %bh  {bus}
  shlw  $8, %cx   {func}
  movb  $4, %bl
  movb  reg, %cl  {func}
  andb  $7, %ch   {func}
  movl  $62, %eax
  orb   %dl, %ch  {dev/func}
  int   $0x40
  popl  %edx
  popl  %ecx
  popl  %ebx
end;

function kos_readpciw(bus, dev, func, reg: Byte): Word; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  pushl %edx
  shlb  $3, %dl   {dev}
  movb  %al, %bh  {bus}
  shlw  $8, %cx   {func}
  movb  $5, %bl
  movb  reg, %cl  {reg}
  andb  $7, %ch   {func}
  movl  $62, %eax
  orb   %dl, %ch  {dev/func}
  int   $0x40
  popl  %edx
  popl  %ecx
  popl  %ebx
end;

function kos_readpcid(bus, dev, func, reg: Byte): DWord; assembler; register;
asm
  pushl %ebx
  pushl %ecx
  pushl %edx
  shlb  $3, %dl   {dev}
  movb  %al, %bh  {bus}
  shlw  $8, %cx   {func}
  movb  $6, %bl
  movb  reg, %cl  {reg}
  andb  $7, %ch   {func}
  movl  $62, %eax
  orb   %dl, %ch  {dev/func}
  int   $0x40
  popl  %edx
  popl  %ecx
  popl  %ebx
end;


{ Other }
procedure kos_delay(ms: DWord); assembler; register;
asm
  pushl %ebx
  movl  %eax, %ebx
  movl  $5, %eax
  int   $0x40
  popl  %ebx
end;
