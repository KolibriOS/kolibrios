#include "math/init.c"
#include "random/init.c"
#include "re/init.c"
#include "ksys/init.c"
#include "pygame/init.c"
#include "bitwise/init.c"
#include "file/init.c"
//#include "socket/socket.c" 

void init_std_modules(TP){
      math_init(tp);
      random_init(tp);
      re_init(tp);
      ksys_init(tp);
      file_init(tp);
      //socket_init(tp);
      pygame_init(tp);
      bitwise_init(tp);
}
