#ifndef INCLUDE_HUMAN_READABLE_H
#define INCLUDE_HUMAN_READABLE_H

#ifdef __cplusplus
extern "C" 
{
#endif // __cplusplus

int print_humanreadable_time(char* result, int bufferSize, double num, const char* suffix = "");
int print_humanreadable_giga(char* result, int bufferSize, double num, const char* suffix = "");
int print_humanreadable_gibi(char* result, int bufferSize, double num, const char* suffix = "");

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //INCLUDE_HUMAN_READABLE_H