#include <setjmp.h>
#define __USE_GNU
#include <signal.h>
#include <stddef.h>

sigset_t mask;

void reset_procmask(void) {
    sigemptyset (&mask);
	sigaddset (&mask, SIGPROF);
    sigprocmask(SIG_UNBLOCK, &mask, NULL);
}

int get_fake_if(ucontext_t *ctx) {
    // we fake IF with id flag
    return !(ctx->uc_mcontext.__gregs[REG_EFL] & (1 << 21));
}
