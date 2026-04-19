#ifndef _UNCONST_H_
#define _UNCONST_H_

#ifndef unconst
#define unconst(__v, __t) __extension__ ({union { const __t __cp; __t __p; } __q; __q.__cp = __v; __q.__p;})
#endif

#endif // _UNCONST_H_
