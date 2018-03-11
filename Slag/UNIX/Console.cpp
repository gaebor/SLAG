#include "../OS_dependent.h"

#include <wordexp.h>

int GetConsoleWidth()
{
    return 80;
}

static int cursor_pos;

void RememberCursorPosition()
{
}
void RestoreCursorPosition()
{
}

std::vector<std::string> split_to_argv(const std::string& line)
{
	wordexp_t wordexp_result;
	std::vector<std::string> argv;

	if (wordexp(line.c_str(), &wordexp_result, 0) == 0)
	{
		for (size_t i = 0; i < wordexp_result.we_wordc; ++i)
			argv.emplace_back(wordexp_result.we_wordv[i]);

		wordfree(&wordexp_result);
	}
	return argv;
}
