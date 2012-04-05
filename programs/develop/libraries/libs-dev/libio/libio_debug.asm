
macro   debug_FileInfoHeader ptr {
        DEBUGF  1, "FileInfoHeader (0x%x)\n", ptr
        DEBUGF  1, "Version: %u\n", [ptr + FileInfoHeader.Version]:4
        DEBUGF  1, "FilesRead: %u\n", [ptr + FileInfoHeader.FilesRead]:4
        DEBUGF  1, "FilesCount: %u\n\n", [ptr + FileInfoHeader.FilesCount]:4
}

macro   debug_FileInfoA ptr {
        DEBUGF  1, "FileInfoA (0x%x)\n", ptr
        DEBUGF  1, "Attributes: 0x%x\n", [ptr + FileInfoA.Attributes]:8
        DEBUGF  1, "Flags: 0x%x\n", [ptr + FileInfoA.Flags]:8
        DEBUGF  1, "FileSizeLow: 0x%x\n", [ptr + FileInfoA.FileSize]:8
        DEBUGF  1, "FileSizeHigh: 0x%x\n", [ptr + FileInfoA.FileSize+4]:8
        push    eax
        lea     eax, [ptr + FileInfoA.FileName]
        DEBUGF  1, "FileName: %s\n\n", eax
        pop     eax
}

macro   debug_FileInfoBlock ptr {
        DEBUGF  1, "FileInfoBlock (0x%x)\n", ptr
        DEBUGF  1, "Function: %u\n", [ptr + FileInfoBlock.Function]:4
        DEBUGF  1, "Position: %u\n", [ptr + FileInfoBlock.Position]:4
        DEBUGF  1, "Flags: 0x%x\n", [ptr + FileInfoBlock.Flags]:8
        DEBUGF  1, "Count: %u\n", [ptr + FileInfoBlock.Count]:4
        DEBUGF  1, "Buffer: 0x%x\n", [ptr + FileInfoBlock.Buffer]:8
        DEBUGF  1, "FileName: %s\n\n", [ptr + FileInfoBlock.FileName]:8
}

macro   debug_FindOptions ptr {
        DEBUGF  1, "FindOptions (0x%x)\n", ptr
        DEBUGF  1, "Attributes: 0x%x\n", [ptr + FindOptions.Attributes]:8
        DEBUGF  1, "Mask: %s\n\n", [ptr + FindOptions.Mask]
}

macro   debug_FindFileBlock ptr {
        push    eax
        mov     eax, ptr
        DEBUGF  1, "FindFileBlock (0x%x)\n\n", eax
        debug_FileInfoHeader eax
        add     eax, sizeof.FileInfoHeader
        debug_FileInfoA eax
        add     eax, sizeof.FileInfoA
        debug_FileInfoBlock eax
        add     eax, sizeof.FileInfoBlock
        debug_FindOptions eax
        pop     eax
}