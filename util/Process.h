#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include <cstdio>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "OsUtil.h"

#ifndef OS_UNIX
# error This file is so far only implemented for Unix-like systems
# pragma message("TODO: implement this for non-unixes")
#endif

class ProcessError : public std::runtime_error
{
	using std::runtime_error::runtime_error;
};

class Process
{
	const std::string m_name;
	const std::vector<std::string> m_args;
public:
	explicit Process(const std::string &name, const std::vector<std::string> &args);
	~Process();

	void run();
	int wait();

	std::istream &toParentStream() { return *m_toParentStream; }
	std::ostream &toChildStream() { return *m_toChildStream; }

private:
	boost::iostreams::stream<boost::iostreams::file_descriptor_sink> *m_toChildStream;
	boost::iostreams::stream<boost::iostreams::file_descriptor_source> *m_toParentStream;
#ifdef OS_UNIX
	pid_t m_pid;
	FILE *m_toChildPipe = nullptr;
	FILE *m_toParentPipe = nullptr;
#endif
};
