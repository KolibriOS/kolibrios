SECTIONS
{
    . = 0x00000;
    .text :
    {
        LONG(0x554e454D);
        LONG(0x31305445);
        LONG(1);
        LONG(go.example.Main);
        LONG(__end);
        LONG(0x10000);
        LONG(0x10000);
        LONG(0);
        LONG(0);
        *(.text)
    }
    .eh_frame : {
        *(.eh_frame)
    }
    .group : {
        *(.group)
    }
    .data : {
        *(.data)
    }
    .rodata : {
        *(.rodata)
        *(.rodata.*)
    }
    __end = .;
}
