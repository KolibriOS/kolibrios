1. INSTALL
  Copy "@notify" and "notify3.png" to "/sys/". Launch "test.sh" for testing.

2. FORMAT
 2.1 RUN
  a) @notify <TEXT>
  b) @notify "<TEXT>" [-<KEYS>]
  c) @notify '<TEXT>' [-<KEYS>]

 2.2 <TEXT>
  All charactes. If you won`t to write character of quote (" or '), you must
  enter \ before it (\" or \'). New-line character is "\n".

 2.3 <KEYS>
  d - disable auto-closing
  t - first line is title
  A - application icon
  E - error icon
  W - warning icon
  O - ok icon
  N - network icon
  I - info icon
  F - folder icon
  C - component icon
