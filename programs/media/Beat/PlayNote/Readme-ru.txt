PlayNote (дата выпуска 2020.05.17)

PlayNote - простая программа для проигрывания ноты. Звук проигрывается через звуковой драйвер.

Использование: PlayNote <path>
                path - путь к файлу, который будет проигран.

Примеры:
 PlayNote note.raw
 PlayNote /tmp0/1/note.raw

===========================
Для генерирования ноты в формате .wav при помощи sox (для прослушивания результата:
 sox -n -L -c 1 -b 16 -r 48000 Note_C6.wav synth 1 sine 1046.4
Для генерирования ноты в формате .raw при помощи sox (для программы PlayNote):
 sox -n -L -c 1 -b 16 -r 48000 Note_C6.raw synth 1 sine 1046.4

Для установки программы sox в Ubuntu:
 sudo apt install sox
===========================

//--------------------------------------//
  The programme: 
   - Compiled with KTCC compiler.
   - Written in KolibriOS NB svn7768.
   - Designed and written by JohnXenox
     aka Aleksandr Igorevich.
//--------------------------------------//
