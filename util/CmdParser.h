/*
 * Copyright 2015 Jan Dalheimer <jan@dalheimer.de>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <map>
#include <memory>

#include "ArgonautsException.h"

namespace Argonauts {
namespace Util {
namespace CLI {

using std::string;
using std::vector;
using std::map;

enum class Execution
{
	ExitSuccess,
	ExitFailure,
	Continue
};

class Parser;
namespace detail
{
template <typename T>
struct Identity { using type = T; };

template <typename Func>
struct toFunction { using type = Func; };
template <typename Ret, typename Arg>
struct toFunction<Ret(Arg)> : public toFunction<std::function<Ret(Arg)>> {};
template <typename Ret, typename Arg>
struct toFunction<Ret(*)(Arg)> : public toFunction<std::function<Ret(Arg)>> {};
template <typename Ret>
struct toFunction<Ret()> : public toFunction<std::function<Ret()>> {};
template <typename Ret>
struct toFunction<Ret(*)()> : public toFunction<std::function<Ret()>> {};

using Caller = std::function<Execution(const Parser &parser)>;

inline Caller wrapCallerHelper(std::function<Execution(const Parser &parser)> &&func) { return std::forward<Caller>(func); }
inline Caller wrapCallerHelper(std::function<Execution()> &&func) { return [func](const Parser &) { return func(); }; }

template <typename Func>
Caller wrapCaller(Func &&func) { return wrapCallerHelper(typename toFunction<Func>::type(std::forward<Func>(func))); }

/*
template <typename Func>
typename std::enable_if<std::is_same<typename std::result_of<Func>::type, Execution>::value && has_argument<Func>::value, Caller>::type
wrapCaller(Func &&func) { return Caller(std::forward<Func>(func)); }
template <typename Func>
typename std::enable_if<std::is_same<typename std::result_of<Func>::type, Execution>::value && !has_argument<Func>::value, Caller>::type
wrapCaller(Func &&func) { return [func](const Parser &) { return func(); }; }
template <typename Func>
typename std::enable_if<std::is_same<typename std::result_of<Func>::type, void>::value && has_argument<Func>::value, Caller>::type
wrapCaller(std::function<void(const Parser &)> &&func) { return [func](const Parser &p) { func(p); return Execution::Continue; }; }
template <typename Func>
typename std::enable_if<std::is_same<typename std::result_of<Func>::type, void>::value && !has_argument<Func>::value, Caller>::type
wrapCaller(std::function<void()> &&func) { return [func](const Parser &) { func(); return Execution::Continue; }; }
*/

template <typename T>
inline T optionValueFromParser(const Parser &parser, const string &name);
template <typename T>
inline T posArgValueFromParser(const Parser &parser, const string &name);
}

class ParserException : public Exception
{
	using Exception::Exception;
};
class ParserBuildException : public ParserException
{
	using ParserException::ParserException;
};
class MissingRequiredOptionException : public ParserException
{
	using ParserException::ParserException;
};
class MissingRequiredPositionalArgumentException : public ParserException
{
	using ParserException::ParserException;
};
class MissingRequiredOptionArgument : public ParserException
{
	using ParserException::ParserException;
};
class ArgumentNotInSetException : public ParserException
{
	using ParserException::ParserException;
};

/// Specifies an option (--output=OUTPUT, --no-overwrite, -c)
class Option : public std::enable_shared_from_this<Option>
{
	friend class Subcommand;
	friend class Parser;

	/// a list of possible names for this option, like {"output", "out", "o"}
	/// @note special handling is applied to names beginning with no-
	vector<string> names;
	/// the help text for this option
	string help;
	/// if not empty, the name of the argument of this option
	string argument;
	/// true if we require a non-empty argument
	bool isArgumentRequired = false;
	/// the callbacks that will be executed if this option is given on the CLI
	vector<detail::Caller> callbacks;
	/// true if this option is required
	bool isRequired = false;
	/// true if this option should exclusivly be invoked before checking constraints
	bool earlyExit = false;
	/// if non-empty, the argument must be contained in this list
	vector<string> fromSet;
	/// if non-empty, the default if no explicit argument is given
	string defaultArgument;

public:
	using Ptr = std::shared_ptr<Option>;

	explicit Option(const vector<string> &names_, const string &help_)
		: names(names_), help(help_) {}
	explicit Option() {}

	Option::Ptr beingRequired()
	{
		isRequired = true;
		return shared_from_this();
	}
	Option::Ptr withArg(const string &arg, const string &def = string())
	{
		argument = arg;
		defaultArgument = def;
		return shared_from_this();
	}
	Option::Ptr withRequiredArg(const string &arg, const string &def = string())
	{
		isArgumentRequired = true;
		return withArg(arg, def);
	}
	Option::Ptr requireFromSet(const vector<string> &list)
	{
		fromSet = vector<string>(list);
		return shared_from_this();
	}
	Option::Ptr makeEarlyExit()
	{
		earlyExit = true;
		return shared_from_this();
	}

	template <typename Func>
	Option::Ptr then(Func &&func)
	{
		callbacks.push_back(detail::wrapCaller(std::forward<Func>(func)));
		return shared_from_this();
	}
	template <typename Obj, typename Func>
	Option::Ptr then(Obj &obj, Func func)
	{
		return then([&obj, func](const Parser &parser) { return (obj.*func)(parser); });
	}
	template <typename Obj, typename Func>
	Option::Ptr then(Obj *obj, Func func)
	{
		return then([obj, func](const Parser &parser) { return (obj->*func)(parser); });
	}

	template <typename Obj, typename Type>
	Option::Ptr applyTo(Obj *obj, Type Obj::*member)
	{
		const std::string name = names.front();
		return then([name, obj, member](const Parser &parser) { obj->*member = detail::optionValueFromParser<Type>(parser, name); return Execution::Continue; });
	}
	template <typename Obj, typename Type>
	Option::Ptr applyTo(Obj &obj, Type Obj::*member)
	{
		const std::string name = names.front();
		return then([name, &obj, member](const Parser &parser) { obj.*member = detail::optionValueFromParser<Type>(parser, name); return Execution::Continue; });
	}
};
/// Specifies a positional argument; a non-option argument of variable content following subcommands
class PositionalArgument : public std::enable_shared_from_this<PositionalArgument>
{
	friend class Subcommand;
	friend class Parser;

	/// The name of this argument
	string name;
	/// The help text for this argument
	string help;
	/// Can this argument be given multiple times?
	bool repeatable = false;
	/// Is this argument optional?
	bool optional = false;
	/// The callbacks that will be executed
	vector<detail::Caller> callbacks;

public:
	using Ptr = std::shared_ptr<PositionalArgument>;

	explicit PositionalArgument(const string &name_, const string &help_, const bool repeatable_, const bool optional_)
		: name(name_), help(help_), repeatable(repeatable_), optional(optional_) {}

	explicit PositionalArgument() {}

	template <typename Func>
	PositionalArgument::Ptr then(Func &&func)
	{
		callbacks.push_back(detail::wrapCaller(std::forward<Func>(func)));
		return shared_from_this();
	}
	template <typename Obj, typename Type>
	PositionalArgument::Ptr applyTo(Obj *obj, Type Obj::*member)
	{
		const string n = name;
		return then([n, obj, member](const Parser &parser)
		{
			obj->*member = detail::posArgValueFromParser<Type>(parser, n);
			return Execution::Continue;
		});
	}
	template <typename Obj, typename Type>
	PositionalArgument::Ptr applyTo(Obj &obj, Type Obj::*member)
	{
		const std::string n = name;
		return then([n, &obj, member](const Parser &parser)
		{
			obj.*member = detail::posArgValueFromParser<Type>(parser, n);
			return Execution::Continue;
		});
	}
};
/// A subcommand provides additional positional argument, options etc.
class Subcommand : public std::enable_shared_from_this<Subcommand>
{
	friend class Parser;

public:
	using Ptr = std::shared_ptr<Subcommand>;

private:
	/// A list of synonumous names for this subcommand (like "install" and "i")
	vector<string> names;
	/// A help string for this command
	string help;
	/// A list of options attached to this command
	vector<Option::Ptr> options;
	/// A list of positional arguments that this command expects
	vector<PositionalArgument::Ptr> positionalArguments;
	/// A list of subcommands of this command
	vector<Subcommand::Ptr> commands;
	/// The callback to be executed if this subcommand is given on the CLI
	detail::Caller callback;
	/// The parent command of this command
	Subcommand::Ptr parent;

public:
	explicit Subcommand(const vector<string> &names_, const string &help_, const Subcommand::Ptr &parent_)
		: names(names_), help(help_), parent(parent_) {}

	explicit Subcommand() {}

	enum PositionalArgumentFlags
	{
		None = 0x0,
		Repeatable = 0x1,
		Optional = 0x2
	};

	/// Returns a map of all names for all options mapped to the respective option
	map<string, Option::Ptr> mappedOptions() const;
	/// Returns a map of all names for all positional arguments mapped to the respective argument
	map<string, PositionalArgument::Ptr> mappedPositionals();
	/// looks up a subcommand by name
	Subcommand::Ptr subcommand(const string &name);

	PositionalArgument::Ptr withPositionalArgument(const string &arg, const string &help_, const int flags = None);
	Option::Ptr addOption(const std::initializer_list<string> &names_, const string &help_);
	Subcommand::Ptr addSubcommand(const std::initializer_list<string> &names_, const string &help_);
	template <typename Func>
	Subcommand::Ptr then(Func &&func)
	{
		callback = detail::wrapCaller(std::forward<Func>(func));
		return shared_from_this();
	}
	template <typename Func>
	Subcommand::Ptr setup(Func &&func)
	{
		auto ptr = shared_from_this();
		func(ptr);
		return ptr;
	}
};

class Parser
{
public:
	explicit Parser(const Subcommand::Ptr &cmd, const string &name, const string &version);

	int parse(const int argc, const char **argv);

	// helpers
	Execution showHelp(const vector<string> &subcommands) const;
	Execution showList(const vector<string> &subcommands) const;
	const string &name() const { return m_name; }
	const string &version() const { return m_version; }

	// result accessors
	template <typename T>
	T option(const string &name) const
	{
		return castValue(name, rawValue(name), detail::Identity<T>());
	}
	bool hasOption(const string &name) const;
	vector<string> subcommands() const { return m_subcommands; }
	string positionalArgument(const string &name) const;
	vector<string> positionalArguments(const string &name) const;
	bool hasPositionalArgument(const string &name) const;
	const string &programName() const { return m_programName; }

private:
	const Subcommand::Ptr m_rootCommand;
	const string m_name;
	const string m_version;

	// result
	map<string, vector<string>> m_options;
	vector<string> m_subcommands;
	map<string, vector<string>> m_positionalArgs;
	string m_programName;

	// internal result accessors
	vector<string> rawValue(const std::string name) const;

	string castValue(const string &, const vector<string> &strings, const detail::Identity<string>) const { return strings.front(); }
	int castValue(const string &, const vector<string> &strings, const detail::Identity<int>) const { return std::stoi(strings.front()); }
	vector<string> castValue(const string &, const vector<string> &strings, const detail::Identity<vector<string>>) const { return strings; }
	vector<int> castValue(const string &, const vector<string> &strings, const detail::Identity<vector<int>>) const
	{
		vector<int> result;
		std::transform(strings.begin(), strings.end(), std::back_inserter(result), [](const string &string){ return std::stoi(string); });
		return result;
	}
	bool castValue(const string &name, const vector<string> &, const detail::Identity<bool>) const;

	// showHelp helper
	void printOptionsTable(const vector<Option::Ptr> &ptr) const;
};

class ParserBuilder : public Subcommand
{
	const string m_name;
	const string m_version;
public:
	using Ptr = std::shared_ptr<ParserBuilder>;

	explicit ParserBuilder(const string &name, const string &version, const string &help)
		: Subcommand({}, help, {}), m_name(name), m_version(version) {}

	// convenience methods for common options and commands
	Option::Ptr addHelpOption();
	Option::Ptr addVersionOption();
	Subcommand::Ptr addListCommand();
	Subcommand::Ptr addHelpCommand();

	Parser build();
};

namespace detail
{
// these need to come after Parser since we need to have it defined to be able to access it
template <typename T>
inline T optionValueFromParser(const Parser &parser, const string &name)
{
	return parser.option<T>(name);
}
template <typename T>
inline T posArgValueFromParser(const Parser &parser, const string &name)
{
	return parser.positionalArgument(name);
}
template<>
inline vector<string> posArgValueFromParser<vector<string>>(const Parser &parser, const string &name)
{
	return parser.positionalArguments(name);
}
}

}
}
}
