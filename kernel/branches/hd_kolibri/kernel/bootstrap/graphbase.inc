;Set base of graphic segment to linear address of LFB
        mov     eax,[LFBAddress]          ; set for gs
        mov     [graph_data_l+2],ax
        shr     eax,16
        mov     [graph_data_l+4],al
        mov     [graph_data_l+7],ah

