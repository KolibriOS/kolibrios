umka_init
disk_add ../img/xfs_v5_files_s05k_b4k_n8k.img hd0 -c 0
# hole begin
# zero length
read70 /hd0/1/hole_begin 0 0 -b
read70 /hd0/1/hole_begin 1 0 -b
read70 /hd0/1/hole_begin 0x3ffe 0 -b
read70 /hd0/1/hole_begin 0x3fff 0 -b
read70 /hd0/1/hole_begin 0x4000 0 -b
read70 /hd0/1/hole_begin 0x4001 0 -b
# one-byte length
read70 /hd0/1/hole_begin 0 1 -b
read70 /hd0/1/hole_begin 1 1 -b
read70 /hd0/1/hole_begin 0x3ffe 1 -b
read70 /hd0/1/hole_begin 0x3fff 1 -b
read70 /hd0/1/hole_begin 0x4000 1 -b
read70 /hd0/1/hole_begin 0x4001 1 -b
# fixed-size block, different begin/end positions
read70 /hd0/1/hole_begin 0 11 -b
read70 /hd0/1/hole_begin 1 11 -b
read70 /hd0/1/hole_begin 0x3ff4 11 -b
read70 /hd0/1/hole_begin 0x3ff5 11 -b
read70 /hd0/1/hole_begin 0x3ff6 11 -b
read70 /hd0/1/hole_begin 0x3ff7 11 -b
read70 /hd0/1/hole_begin 0x3ffe 11 -b
read70 /hd0/1/hole_begin 0x3fff 11 -b
read70 /hd0/1/hole_begin 0x4000 11 -b
read70 /hd0/1/hole_begin 0x4001 11 -b

# hole middle
# zero length
read70 /hd0/1/hole_middle 0x7ffe 0 -b
read70 /hd0/1/hole_middle 0x7fff 0 -b
read70 /hd0/1/hole_middle 0x8000 0 -b
read70 /hd0/1/hole_middle 0x8001 0 -b
read70 /hd0/1/hole_middle 0xbffe 0 -b
read70 /hd0/1/hole_middle 0xbfff 0 -b
read70 /hd0/1/hole_middle 0xc000 0 -b
read70 /hd0/1/hole_middle 0xc001 0 -b
# one-byte length
read70 /hd0/1/hole_middle 0x7ffe 1 -b
read70 /hd0/1/hole_middle 0x7fff 1 -b
read70 /hd0/1/hole_middle 0x8000 1 -b
read70 /hd0/1/hole_middle 0x8001 1 -b
read70 /hd0/1/hole_middle 0xbffe 1 -b
read70 /hd0/1/hole_middle 0xbfff 1 -b
read70 /hd0/1/hole_middle 0xc000 1 -b
read70 /hd0/1/hole_middle 0xc001 1 -b
# fixed-size block, different begin/end positions
read70 /hd0/1/hole_middle 0x7ff4 11 -b
read70 /hd0/1/hole_middle 0x7ff5 11 -b
read70 /hd0/1/hole_middle 0x7ff6 11 -b
read70 /hd0/1/hole_middle 0x7ff7 11 -b
read70 /hd0/1/hole_middle 0x7ffe 11 -b
read70 /hd0/1/hole_middle 0x7fff 11 -b
read70 /hd0/1/hole_middle 0x8000 11 -b
read70 /hd0/1/hole_middle 0x8001 11 -b
read70 /hd0/1/hole_middle 0xbff4 11 -b
read70 /hd0/1/hole_middle 0xbff5 11 -b
read70 /hd0/1/hole_middle 0xbff6 11 -b
read70 /hd0/1/hole_middle 0xbff7 11 -b
read70 /hd0/1/hole_middle 0xbffe 11 -b
read70 /hd0/1/hole_middle 0xbfff 11 -b
read70 /hd0/1/hole_middle 0xc000 11 -b
read70 /hd0/1/hole_middle 0xc001 11 -b

# hole end
# zero length
read70 /hd0/1/hole_end 0xbffe 0 -b
read70 /hd0/1/hole_end 0xbfff 0 -b
read70 /hd0/1/hole_end 0xc000 0 -b
read70 /hd0/1/hole_end 0xc001 0 -b
read70 /hd0/1/hole_end 0xfffe 0 -b
read70 /hd0/1/hole_end 0xffff 0 -b
read70 /hd0/1/hole_end 0x10000 0 -b
read70 /hd0/1/hole_end 0x10001 0 -b
# one-byte length
read70 /hd0/1/hole_end 0xbffe 1 -b
read70 /hd0/1/hole_end 0xbfff 1 -b
read70 /hd0/1/hole_end 0xc000 1 -b
read70 /hd0/1/hole_end 0xc001 1 -b
read70 /hd0/1/hole_end 0xfffe 1 -b
read70 /hd0/1/hole_end 0xffff 1 -b
read70 /hd0/1/hole_end 0x10000 1 -b
read70 /hd0/1/hole_end 0x10001 1 -b
# fixed-size block, different begin/end positions
read70 /hd0/1/hole_end 0xbff4 11 -b
read70 /hd0/1/hole_end 0xbff5 11 -b
read70 /hd0/1/hole_end 0xbff6 11 -b
read70 /hd0/1/hole_end 0xbff7 11 -b
read70 /hd0/1/hole_end 0xbffe 11 -b
read70 /hd0/1/hole_end 0xbfff 11 -b
read70 /hd0/1/hole_end 0xc000 11 -b
read70 /hd0/1/hole_end 0xc001 11 -b
read70 /hd0/1/hole_end 0xfff4 11 -b
read70 /hd0/1/hole_end 0xfff5 11 -b
read70 /hd0/1/hole_end 0xfff6 11 -b
read70 /hd0/1/hole_end 0xfff7 11 -b
read70 /hd0/1/hole_end 0xfffe 11 -b
read70 /hd0/1/hole_end 0xffff 11 -b
read70 /hd0/1/hole_end 0x10000 11 -b
read70 /hd0/1/hole_end 0x10001 11 -b
