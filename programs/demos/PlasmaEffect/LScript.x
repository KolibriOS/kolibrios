PATH_SIZE   = 1024;
PARAMS_SIZE =  256;
STACK_SIZE  = 1024;

SECTIONS
{
  .all : AT(0){
    LONG(0x554e454D);
    LONG(0x31305445);
    LONG(1);
    LONG("@Main");
    LONG(END);
    LONG($END + PATH_SIZE + PARAMS_SIZE + STACK_SIZE);
    LONG($END + PATH_SIZE + PARAMS_SIZE + STACK_SIZE);
    LONG($END + PATH_SIZE);
    LONG($END);
    *(.text)
    *(.rdata)
    *(.data)
  }
END = .;  
  .bss ALIGN(16) : {*(.bss)}
$END = .;
}