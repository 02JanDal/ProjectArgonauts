#pragma once

#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <map>

#include "ArgonautsException.h"

namespace CLI
{
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

class ParserException : public ArgonautsException
{
	using ArgonautsException::ArgonautsException;
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

/// Specifies an option (--output=OUTPUT, --no-overwrite, -c)
class Option
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

	explicit Option(const vector<string> &names, const string help)
		: names(names), help(help) {}

public:
	explicit Option() {}

	Option &beingRequired()
	{
		isRequired = true;
		return *this;
	}
	Option &withArg(const string &arg);
	Option &withRequiredArg(const string &arg)
	{
		isArgumentRequired = true;
		return withArg(arg);
	}
	template <typename Func>
	Option &then(Func &&func)
	{
		callbacks.push_back(detail::wrapCaller(std::forward<Func>(func)));
		return *this;
	}
	template <typename Obj, typename Type>
	Option &applyTo(Obj *obj, Type Obj::*member)
	{
		return then([this, obj, member](const Parser &parser) { obj->*member = detail::optionValueFromParser<Type>(parser, names.front()); return Execution::Continue; });
	}
	template <typename Obj, typename Type>
	Option &applyTo(Obj &obj, Type Obj::*member)
	{
		return then([this, &obj, member](const Parser &parser) { obj.*member = detail::optionValueFromParser<Type>(parser, names.front()); return Execution::Continue; });
	}
};
/// Specifies a positional argument; a non-option argument of variable content following subcommands
class PositionalArgument
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

protected:
	explicit PositionalArgument(const string &name, const string &help, const bool repeatable, const bool optional)
		: name(name), help(help), repeatable(repeatable), optional(optional) {}

public:
	explicit PositionalArgument() {}

	template <typename Func>
	PositionalArgument &then(Func &&func)
	{
		callbacks.push_back(detail::wrapCaller(std::forward<Func>(func)));
		return *this;
	}
	template <typename Obj, typename Type>
	PositionalArgument &applyTo(Obj *obj, Type Obj::*member)
	{
		const string name = this->name;
		return then([name, obj, member](const Parser &parser)
		{
			obj->*member = detail::posArgValueFromParser<Type>(parser, name);
			return Execution::Continue;
		});
	}
	template <typename Obj, typename Type>
	PositionalArgument &applyTo(Obj &obj, Type Obj::*member)
	{
		return then([this, &obj, member](const Parser &parser)
		{
			obj.*member = detail::posArgValueFromParser<Type>(parser, name);
			return Execution::Continue;
		});
	}
};
/// A subcommand provides additional positional argument, options etc.
class Subcommand
{
	friend class Parser;

	/// A list of synonumous names for this subcommand (like "install" and "i")
	vector<string> names;
	/// A help string for this command
	string help;
	/// A list of options attached to this command
	vector<Option> options;
	/// A list of positional arguments that this command expects
	vector<PositionalArgument> positionalArguments;
	/// A list of subcommands of this command
	vector<Subcommand> commands;
	/// The callback to be executed if this subcommand is given on the CLI
	detail::Caller callback;

protected:
	explicit Subcommand(const vector<string> &names, const string &help)
		: names(names), help(help) {}

public:
	explicit Subcommand() {}

	enum PositionalArgumentFlags
	{
		None = 0x0,
		Repeatable = 0x1,
		Optional = 0x2
	};

	/// Returns a map of all names for all options mapped to the respective option
	map<string, Option> mappedOptions() const;
	/// Returns a map of all names for all positional arguments mapped to the respective argument
	map<string, PositionalArgument> mappedPositionals();
	/// looks up a subcommand by name
	Subcommand &subcommand(const string &name);

	PositionalArgument &withPositionalArgument(const string &arg, const string &help, const int flags = None);
	Option &addOption(const std::initializer_list<string> &names, const string &help);
	Subcommand &addSubcommand(const std::initializer_list<string> &names, const string &help);
	template <typename Func>
	Subcommand &then(Func &&func)
	{
		callback = detail::wrapCaller(std::forward<Func>(func));
		return *this;
	}
	template <typename Func>
	Subcommand &setup(Func &&func)
	{
		func(*this);
		return *this;
	}
};

class Parser
{
public:
	explicit Parser(const Subcommand &cmd, const string &name, const string &version);

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
	vector<string> subcommands() const { return m_subcommands; }
	string positionalArgument(const string &name) const;
	vector<string> positionalArguments(const string &name) const;
	const string &programName() const { return m_programName; }

private:
	const Subcommand m_rootCommand;
	const string m_name;
	const string m_version;

	// result
	map<string, vector<string>> m_options;
	vector<string> m_subcommands;
	map<string, vector<string>> m_positionalArgs;
	string m_programName;

	// internal result accessors
	vector<string> rawValue(const string &name) const { return const_cast<Parser *>(this)->m_options[name]; }

	string castValue(const string &name, const vector<string> &strings, const detail::Identity<string>) const { return strings.front(); }
	int castValue(const string &name, const vector<string> &strings, const detail::Identity<int>) const { return std::stoi(strings.front()); }
	vector<string> castValue(const string &name, const vector<string> &strings, const detail::Identity<vector<string>>) const { return strings; }
	vector<int> castValue(const string &name, const vector<string> &strings, const detail::Identity<vector<int>>) const
	{
		vector<int> result;
		std::transform(strings.begin(), strings.end(), std::back_inserter(result), [](const string &string){ return std::stoi(string); });
		return result;
	}
	bool castValue(const string &name, const vector<string> &, const detail::Identity<bool>) const;
};

class ParserBuilder : public Subcommand
{
	const string m_name;
	const string m_version;
public:
	explicit ParserBuilder(const string &name, const string &version, const string &help)
		: Subcommand({}, help), m_name(name), m_version(version) {}

	// convenience methods for common options and commands
	Option &addHelpOption();
	Option &addVersionOption();
	Subcommand &addListCommand();
	Subcommand &addHelpCommand();

	Parser build() const;
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
