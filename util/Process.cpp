#include "Process.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <sys/wait.h>
#include <iostream>

Process::Process(const std::string &name, const std::vector<std::string> &args)
	: m_name(name), m_args(args)
{}

Process::~Process()
{
	delete m_toChildStream;
	//delete m_toParentStream;
#ifdef OS_UNIX
	if (m_toChildPipe != nullptr) {
		fclose(m_toChildPipe);
	}
	if (m_toParentPipe != nullptr) {
		fclose(m_toParentPipe);
	}
#endif
}

void Process::run()
{
	std::vector<const char *> args = {m_name.c_str()};
	std::transform(m_args.begin(), m_args.end(), std::back_inserter(args), [](const std::string &str) { return str.c_str(); });
	args.push_back(nullptr);

	int fdToChild[2];
	int fdToParent[2];
	if (pipe(fdToChild) == -1 || pipe(fdToParent) == -1) {
		throw ProcessError("Unable to open pipes");
	}

	switch (m_pid = fork()) {
	case -1: // error
		close(fdToChild[0]);
		close(fdToChild[1]);
		close(fdToParent[0]);
		close(fdToParent[1]);
		throw ProcessError("Unable to fork");
	case 0: // child
		close(fdToChild[1]);
		close(fdToParent[0]);
		dup2(fdToChild[0], STDIN_FILENO);
		dup2(fdToParent[1], STDOUT_FILENO);
		fflush(stdout);
		write(1, "ffff", 4);
		printf("asdf");
		fflush(stdout);
		execv(m_name.c_str(), (char **)&args[0]);
		_exit(1);
	default: // parent
		close(fdToChild[0]);
		close(fdToParent[1]);
		if (!(m_toChildPipe = fdopen(fdToChild[1], "w"))) {
			throw ProcessError("Unable to open pipe");
		}
		/*if (!(m_toParentPipe = fdopen(fdToParent[0], "r"))) {
			throw ProcessError("Unable to open pipe");
		}*/
		//std::cout << fileno(m_toParentPipe) << " " << fdToParent[0] << std::endl;
		char buf[128];
		const std::size_t s = read(fdToParent[0], buf, 128);
		std::cout << std::string(buf, s) << std::endl;
		m_toChildStream = new boost::iostreams::stream<boost::iostreams::file_descriptor_sink>(fileno(m_toChildPipe), boost::iostreams::never_close_handle);
		//m_toParentStream = new iostreams::stream<iostreams::file_descriptor_source>(fileno(m_toParentPipe), iostreams::never_close_handle);
	}
}

int Process::wait()
{
	int status;
	waitpid(m_pid, &status, WUNTRACED | WCONTINUED);
	if (WIFEXITED(status)) {
		return WEXITSTATUS(status);
	} else {
		return -1;
	}
}
