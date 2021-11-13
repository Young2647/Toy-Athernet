#include<iostream>
#include<string>
#include<exception>

#ifndef ArrayBlockingQueueExceptions_H
#define ArrayBlockingQueueExceptions_H

const char* IllegalArgumentException_msg = "Exception : -ve or zero size capacity arguments are not allowed. Please specify valid +ve capacity.";

struct IllegalArgumentException : public std::exception
{
	const char * what () const throw ()
	{
		return(IllegalArgumentException_msg);
	}
};


const char* IllegalStateException_msg = "Exception : The queue is full. Hence the add method failed.";

struct IllegalStateException : public std::exception
{
	const char * what () const throw ()
	{
		return(IllegalStateException_msg);
	}
};

#endif
