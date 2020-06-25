/* TIMERS PaulCodeman */
/**
	void Timers::revise(void) -> This function revising all timers.
	void Timers::getTime(void) -> This function updating current time for timers.
	dword set(dword,dword,byte); -> This function seting timer for function Timers::revise.
	dword clear(dword); -> This function clearning anything timer.
	---------
	The functions setTimeout,setInterval,clearInterval,clearTimeout implementing functional JavaScript.
*/
#define offsetSizeTimers 4*3+1
#define defaultMaxTimers 1000
:struct Timers
{
	dword time;
	dword alloc;
	dword count;
	dword size;
	void revise(void);
	void getTime(void);
	dword set(dword,dword,byte);
	dword clear(dword);
};
void Timers::getTime(void)
{
	EAX = 26;
	EBX = 9;
	$int 0x40
	time = EAX;
}
void Timers::revise(void)
{
	dword position = 0;
	dword i = 0;
	IF (!alloc) RETURN;
	getTime();
	i = count;
	position = alloc;
	WHILE(i)
	{
		IF (DSDWORD[position])
		{
			IF (DSDWORD[position+4]<=time)
			{
				$call DSDWORD[position];
				IF (DSBYTE[position+12]) DSDWORD[position+4] = time+DSDWORD[position+8];
				ELSE
				{
					DSDWORD[position] = 0;
					count--;
				}
			}
			i--;
		}
		position+=offsetSizeTimers;
	}
}
dword Timers::set(dword function, newTime, byte repeat)
{
	dword position = 0;
	dword i = 0;
	IF (!alloc)
	{
		size = defaultMaxTimers*offsetSizeTimers;
		alloc = malloc(size);
	}
	i = count;
	position = alloc;
	WHILE(i)
	{
		IF (!DSDWORD[position]) BREAK;
		position+=offsetSizeTimers;
		i--;
	}
	count++;
	getTime();
	DSDWORD[position] = function;
	DSDWORD[position+4] = time+newTime;
	DSBYTE[position+8] = newTime;
	DSBYTE[position+12] = repeat;
	RETURN position;
}
dword Timers::clear(dword id)
{
	IF (!alloc) || (!id) || (!DSDWORD[id]) RETURN 0;
	count--;
	DSDWORD[id] = 0;
	RETURN id;
}

// Analogs JS Functions
:Timers Timer = {0};
inline dword setTimeout(dword function, time)
{
	RETURN Timer.set(function, time, 0);
}
inline dword setInterval(dword function, time)
{
	RETURN Timer.set(function, time, 1);
}
inline dword clearTimeout(dword id)
{
	RETURN Timer.clear(id);
}
inline dword clearInterval(dword id)
{
	RETURN Timer.clear(id);
}
