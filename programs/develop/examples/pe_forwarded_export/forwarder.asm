format PE console 0.8 DLL at 410000h
include '../../../export.inc'

align 4
data export
export 'forwarder.dll', \
        func1, 'func1', \
        func2, 'func2'
func1   db 'forwarded.forward_by_name',0
func2   db 'forwarded.#2',0
end data
