if tup.getconfig("NO_GCC") ~= "" then return end
HELPERDIR = (tup.getconfig("HELPERDIR") == "") and "../../../programs" or tup.getconfig("HELPERDIR")
tup.include(HELPERDIR .. "/use_gcc.lua")
tup.include(HELPERDIR .. "/use_newlib.lua")

compile_gcc{"RAM.c", "icache.c", "MMU.c", "pxa255_PwrClk.c", "pxa255_IC.c", "pxa255_GPIO.c", "callout_RAM.c", "rt.c", "pxa255_RTC.c", "SoC.c", "pxa255_UART.c", "pxa255_TIMR.c", "mem.c", "dcache.c", "pxa255_LCD.c", "cp15.c", "pxa255_DMA.c", "math64.c", "CPU.c", "main_pc.c", "pxa255_DSP.c"}
link_gcc("uARM")
