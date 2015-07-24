#pragma once

#ifdef _cplusplus
extern "C" 
{
#endif // _cplusplus

int print_humanreadable_time(char* result, int bufferSize, double num, const char* suffix = "");
int print_humanreadable_giga(char* result, int bufferSize, double num, const char* suffix = "");
int print_humanreadable_gibi(char* result, int bufferSize, double num, const char* suffix = "");

#ifdef _cplusplus
}
#endif // _cplusplus
