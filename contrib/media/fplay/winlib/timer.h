

static uint32_t update_timers(uint32_t realtime)
{
    ostimer_t  *timer;
    uint32_t   exp_time = -1;

    timer = (ostimer_t*)timers.next;
    while( &timer->link != &timers)
    {
        ostimer_t  *tmp;

        tmp   = timer;
        timer = (ostimer_t*)timer->link.next;

        if( tmp->exp_time < realtime)
        {
            list_remove(&tmp->link);
            send_message(tmp->ctrl, MSG_TIMER, tmp->tmr_arg, tmp);
        }
    };

    timer = (ostimer_t*)timers.next;
    while( &timer->link != &timers)
    {
        if( exp_time > timer->exp_time)
            exp_time = timer->exp_time;
        timer = (ostimer_t*)timer->link.next;
    }
    return exp_time;
};

int set_timer(ctrl_t *ctrl, ostimer_t *timer, uint32_t delay)
{
    if( ctrl && timer &&delay)
    {
        timer->ctrl = ctrl;
        timer->exp_time = realtime + delay;

        if( exp_time > timer->exp_time)
            exp_time = timer->exp_time;

        list_append(&timer->link, &timers);
    };
    return 0;
}
