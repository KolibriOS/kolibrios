## DBGBOARD - a console-based debug board 
Main advantages over the old board:
* Bigger font
* Scrolling (like in other console apps)
* Messages highligting
    * K : - kernel messages (K: also supported because some code in kernel prints such)
    * L: - launcher messages
    * I: - information
    * W: - warning
    * E: - error
    * S: - success
* Three display modes (You can switch modes using `Tab` key)
    * User messages
    * Kernel messages
    * Both kernel and user messages

Also, like the old board it writes log to /tmp0/1/boardlog.txt (or you can pass another path in args like `/sys/develop/dbgboard /tmp0/1/hgfdhgfh.txt`), you can view log file in cedit by hitting `F2` key
