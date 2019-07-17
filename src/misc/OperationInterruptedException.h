#ifndef OPERATIONINTERRUPTEDEXCEPTION_H
#define OPERATIONINTERRUPTEDEXCEPTION_H

#include <exception>

class OperationInterruptedException : public std::exception
{
	const char * what()
	{
		return "A socket I/O operation was aborted";
	}
};

#endif
