#include "HumanReadable.h"

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

#include <stdio.h>
#include <math.h>

#ifdef _MSC_VER
# define my_sprintf sprintf_s
#elif defined __GNUC__
# define my_sprintf snprintf
#endif

int print_humanreadable_time( char* result, int bufferSize, double num, const char* suffix /*= ""*/ )
{
	if (num < 1e-6)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num*1e9, " ns", suffix);
	else if (num < 1e-3)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num*1e6, " us", suffix);
	else if (num < 1)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num*1e3, " ms", suffix);
	else if (num < 60)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num, "sec", suffix);
	else if (num < 3600)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num / 60, "min", suffix);
	else if (num < 3600*24)
		return my_sprintf(result, bufferSize, "%7.3f%s%s", num / 3600, "  h", suffix);
	else
		return my_sprintf(result, bufferSize, "%.3f%s%s", num / (3600 * 24), "day", suffix);
}

int print_humanreadable_giga( char* result, int bufferSize, double num, const char* suffix /*= ""*/ )
{
	static const char* units[] = {" ","k","M","G","T","P","E","Z", NULL};
	const char** unit;

	for	(unit = units; *unit != NULL; ++unit)
	{
		if (fabs(num) < 1000.0)
		{
			return my_sprintf(result, bufferSize, "%7.3g%s%s", num, *unit, suffix);
		}
		else
			num /= 1000;
	}
	return my_sprintf(result, bufferSize, "%.3g%s%s", num, "Y", suffix);
}

int print_humanreadable_gibi( char* result, int bufferSize, double num, const char* suffix /*= ""*/ )
{
	static const char* units[] = {"  ","ki","Mi","Gi","Ti","Pi","Ei","Zi", NULL};
	const char** unit;

	for	(unit = units; *unit != NULL; ++unit)
	{
		if (fabs(num) < 1024.0)
		{
			return my_sprintf(result, bufferSize, "%8.4f%s%s", num, *unit, suffix);
		}
		else
			num /= 1024;
	}
	return my_sprintf(result, bufferSize, "%.4f%s%s", num, "Yi", suffix);
}

#ifdef __cplusplus
}

#endif // __cplusplus