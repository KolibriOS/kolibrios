#ifndef _ASM_GENERIC_BUG_H
#define _ASM_GENERIC_BUG_H



#define WARN(condition, format...) ({                               \
    int __ret_warn_on = !!(condition);                              \
    unlikely(__ret_warn_on);                                        \
})


#endif
