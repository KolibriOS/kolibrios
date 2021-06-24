@del search
@c-- /D=LANG_RUS search.c
@rename search.com search
@del warning.txt

if exist search (
    @exit
) else (
    @pause
)