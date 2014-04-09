1. INSTALL
  Copy "@notify" and "notify3.png" to "/sys/". Launch "test.sh" for testing.

2. FORMAT
 2.1 RUN
  a) @notify <TEXT>
  b) @notify "<TEXT>" [-<KEYS>]
  c) @notify '<TEXT>' [-<KEYS>]

 2.2 <TEXT>
  All charactes. If you won`t to write character of quote (" or '), you must
  enter \ before it (\" or \'). New-line character is "\n" or char 10.

 2.3 <KEYS>
  d - disable auto-closing
  t - title

  2.3.1 ICONS:
   A - application
   E - error
   W - warning
   O - ok
   N - network
   I - info
   F - folder
   C - component
   M - mail
   D - download
   H - hard disk
   P - audio player
