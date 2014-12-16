#ifndef RSBITS_H_INCLUDED
#define RSBITS_H_INCLUDED

#define     BIT_SET(var,mask)   { var |= (mask); }
#define     BIT_CLEAR(var,mask) { var &= ~(mask); }
#define     BIT_TOGGLE(var,mask) { var ^= (mask); }

#define     IS_BIT_SET(var,mask)      ( (var) & (mask) )
#define     IS_BIT_CLEARED(var,mask)  (!( (var) & (mask) ))

#endif // RSBITS_H_INCLUDED
