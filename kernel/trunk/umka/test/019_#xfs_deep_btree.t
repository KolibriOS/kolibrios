umka_init
disk_add ../img/xfs_v4_btrees_l2.img hd0 -c 0

ls80 /hd0/1/dir_btree_l2 -f 0 -c 1
ls80 /hd0/1/dir_btree_l2 -f 0 -c 99
ls80 /hd0/1/dir_btree_l2 -f 77777 -c 99
ls80 /hd0/1/dir_btree_l2 -f 150000 -c 99
ls80 /hd0/1/dir_btree_l2 -f 193100 -c 99

read70 /hd0/1/file_btree_l2 0 16388096 -h
read70 /hd0/1/file_btree_l2 77777 7777777 -h
read70 /hd0/1/file_btree_l2 1 0x1001 -h
read70 /hd0/1/file_btree_l2 0x1000 0x1000 -h
read70 /hd0/1/file_btree_l2 0x1000 0x1001 -h

disk_del hd0
