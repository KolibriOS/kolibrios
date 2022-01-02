#include <stdlib.h>
#include <ctype.h>

double strtod (const char* str, char** endptr)
{
	double 	res = 0.0;
	int		pwr = 0, pwr1, esign = 1, sign = 1;

	while (isspace(*str)) str++;

	if (*str == '-') { sign = -1; str++; }
		else
	if (*str == '+') str++;


	while (isdigit(*str))
	{
		res = 10 * res + (*str - '0');
		str++;
	}

	if (*str =='.')
	{
		str++;
		double div = 10.0;
		while (isdigit(*str))
		{
			res += (*str - '0') / div;
			str++;
			div *= 10;
		}
	}

	if (*str =='e' || *str =='E')
	{
		str++;
		if (*str == '-') { esign = -1; str++; }
			else
		if (*str == '+') str++;

		while (isdigit(*str))
		{
			pwr = 10.0 * pwr + (*str - '0');
			str++;
		}

		// fck, i've no pow() yet
		//  res = res * pow(10, pwr);
		for (pwr1 = pwr; pwr1 !=0; pwr1--)
			if (esign == 1)
				res *= 10;
			else
			    res /= 10;

	}
	if (endptr)
		*endptr = (char*)str;

	return res * sign;
}

long double strtold (const char* str, char** endptr)
{
    return (long double)strtod(str, endptr);
}

float strtof (const char* str, char** endptr)
{
    return (float)strtod(str, endptr);
}
