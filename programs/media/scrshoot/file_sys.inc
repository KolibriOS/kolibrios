macro file_sys struc_ptr
{
	mov	eax,58
	mov	ebx,struc_ptr
	int	0x40
}

struc fs_struc function_number,block_number,size,data_ptr,mem_for_sys_ptr
{
.function_number: dd function_number+0	;+0
.block_number: dd block_number+0	;+4
.size: dd size+0			;+8
.data_ptr: dd data_ptr+0		;+12
.mem_for_sys_ptr: dd mem_for_sys_ptr+0	;+16
}
fs_struc_size=20

fs_read_file=0
fs_rewrite_file=1
fs_delete_file=2
fs_write_to_file=3
fs_create_dir=4
fs_rename_file=5
fs_lba_read=9
fs_get_file_size=12
fs_get_file_atr=13
fs_get_file_date_time=14
fs_get_fs_info=15
fs_run_app=16

fse_successful=0
fse_not_def_base=1
fse_bad_fs=2
fse_bad_fs=3
fse_part_not_def=4
fse_file_not_found=5
fse_end_of_file=6
fse_bad_ptr=7
fse_disk_filled=8
fse_bad_fat_table=9
fse_acces_denied=10