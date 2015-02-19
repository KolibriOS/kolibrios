macro cmpe a, b, c
{
    cmp     a, b
    je	    c
}

macro cmpne a, b, c
{
    cmp     a, b
    jne     c
}

macro cmpg a, b, c
{
    cmp     a, b
    jg	    c
}

macro cmpl a, b, c
{
    cmp     a, b
    jl	    c
}

macro cmpnl a, b, c
{
    cmp     a, b
    jnl     c
}

macro cmpng a, b, c
{
    cmp     a, b
    jng     c
}

macro cmpge a, b, c
{
    cmp     a, b
    jge     c
}

macro cmple a, b, c
{
    cmp     a, b
    jle     c
}