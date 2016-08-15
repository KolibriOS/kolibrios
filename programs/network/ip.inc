
struct  IPv4_header

        VersionAndIHL           db ?    ; Version[0-3 bits] and IHL(header length)[4-7 bits]
        TypeOfService           db ?    ; precedence [7-5] minimize delay [4], maximize throughput [3], maximize riliability [2] minimize momentary cost [1] and zero [0]
        TotalLength             dw ?
        Identification          dw ?
        FlagsAndFragmentOffset  dw ?    ; Flags[0-2] and FragmentOffset[3-15]
        TimeToLive              db ?    ;
        Protocol                db ?
        HeaderChecksum          dw ?
        SourceAddress           dd ?
        DestinationAddress      dd ?

ends