OUTPUT_FORMAT("pe-i386")
OUTPUT_ARCH(i386)

SECTIONS
{
    .text : {
        *(.text)
        . = ALIGN(4);
    }

    .rodata : {
        *(.rodata)
        *(.rodata.*)
        
        . = ALIGN(4);

        __start_mutex_init_array = .;
        KEEP(*(.mutex_init_array))
        __stop_mutex_init_array = .;
        
        . = ALIGN(4);
    }

    .data : {
        *(.data)
        . = ALIGN(4);
    }

    .bss : {
        *(.bss)
        *(COMMON)
        . = ALIGN(4);
    }
}