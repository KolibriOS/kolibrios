format PE console 0.8
include '../../../proc32.inc'
include '../../../import.inc'
        invoke  func1
        invoke  func2
        invoke  con_exit, 0
        xor     eax, eax
        ret

align 4
data import
library forwarder, 'forwarder.dll', console, 'console.dll'
import forwarder, func1, 'func1', func2, 'func2'
import console, con_exit, 'con_exit'
end data
