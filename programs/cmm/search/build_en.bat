@del search
@c-- /D=LANG_ENG search.c
@rename search.com search
@del warning.txt

if exist search (
    @exit
) else (
    @pause
)