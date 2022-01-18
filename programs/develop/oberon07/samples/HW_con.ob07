MODULE HW_con;

IMPORT
	Out, In, Console, DateTime;


PROCEDURE OutInt2 (n: INTEGER);
BEGIN
	ASSERT((0 <= n) & (n <= 99));
	IF n < 10 THEN
		Out.Char("0")
	END;
	Out.Int(n, 0)
END OutInt2;


PROCEDURE OutMonth (n: INTEGER);
VAR
	str: ARRAY 4 OF CHAR;
BEGIN
	CASE n OF
	| 1: str := "jan"
	| 2: str := "feb"
	| 3: str := "mar"
	| 4: str := "apr"
	| 5: str := "may"
	| 6: str := "jun"
	| 7: str := "jul"
	| 8: str := "aug"
	| 9: str := "sep"
	|10: str := "oct"
	|11: str := "nov"
	|12: str := "dec"
	END;
	Out.String(str)
END OutMonth;


PROCEDURE main;
VAR
	Year, Month, Day,
	Hour, Min, Sec, Msec: INTEGER;
BEGIN
	Out.String("Hello, world!"); Out.Ln;
	Console.SetColor(Console.White, Console.Red);
	DateTime.Now(Year, Month, Day, Hour, Min, Sec, Msec);
	OutInt2(Day); Out.Char("-"); OutMonth(Month); Out.Char("-"); Out.Int(Year, 0); Out.Char(" ");
	OutInt2(Hour); Out.Char(":"); OutInt2(Min); Out.Char(":"); OutInt2(Sec); Out.Ln;
	Console.SetColor(Console.Blue, Console.LightGray);
	Out.Ln; Out.String("press enter...");
	In.Ln
END main;


BEGIN
	Console.open;
	main;
	Console.exit(TRUE)
END HW_con.