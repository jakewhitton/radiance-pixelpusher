#ifndef LOG_H
#define LOG_H

#include <sstream>
#include <string>
#include <mutex>

// Lock on output is defined in Log.cpp
extern std::mutex outputLock;

/*
 * buildString was adapted from:
 *
 * https://stackoverflow.com/questions/21806561/concatenating-strings-and-numbers-in-variadic-template-function 
 */

template <typename... Args>
std::string buildString(Args const & ... args)
{
	std::ostringstream stream;

	using List = int[];
	(void)List{0, ( (void)(stream << args), 0) ...};

	return stream.str();
}

/*
 * SOURCE_PATH_SIZE is defined by cmake and just represents the name of the cpp file
 * this header file is imported into, and it was adapted from:
 *
 * https://stackoverflow.com/questions/8487986/file-macro-shows-full-path
 */
#define __FILENAME__ __FILE__ + SOURCE_PATH_SIZE

#define INFO(...) logInfo(__FILENAME__, __VA_ARGS__)

template <typename... Args>
void logInfo(const char * filename, const char * fmt, Args... args)
{
	std::string revisedFmt = buildString(filename, " [INFO]: ", fmt, "\n");

	outputLock.lock();
	fprintf(stdout, revisedFmt.c_str(), args...);
	outputLock.unlock();
}

#define ERR(...) logError(__FILENAME__, __LINE__, __VA_ARGS__)

template <typename... Args>
void logError(const char * filename, const int lineNumber, const char * fmt, Args... args)
{
	std::string revisedFmt = buildString(filename, " [ERR, line ", lineNumber, "]: ", fmt, "\n");

	outputLock.lock();
	fprintf(stderr, revisedFmt.c_str(), args...);
	outputLock.unlock();
}

#endif
