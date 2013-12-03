db 'datapack'
count dd 5
dd wav_file1, wav_file1_end - wav_file1
dd wav_file2, wav_file2_end - wav_file2
dd wav_file3, wav_file3_end - wav_file3
dd wav_file4, wav_file4_end - wav_file4
dd wav_file5, wav_file5_end - wav_file5
dd 0

align 4


wav_file1:
file 'bombfly16_2_11.wav':0x36
wav_file1_end:

wav_file2:
file 'bombbang16_2_11.wav':0x36
wav_file2_end:

wav_file3:
file 'zenitka16_2_11.wav':0x36
wav_file3_end:

wav_file4:
file 'plane16_2_11.wav':0x36
wav_file4_end:

wav_file5:
file 'airradewarning16_2_11.wav':0x36
wav_file5_end:
