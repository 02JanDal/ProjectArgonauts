#include "CmdParser.h"

#include <iostream>

#include "StringUtil.h"
#include "TermUtil.h"

namespace CLI
{
Parser::Parser(const Subcommand &cmd, const string &name, const string &version)
	: m_rootCommand(cmd), m_name(name), m_version(version)
{
}

struct Item
{
	string option;
	string argument;
	bool hasArgument = false;
	bool isBooleanInverted = false;

	explicit Item(const string &option, const string &argument, const bool hasArgument, const bool isBooleanInverted)
		: option(option), argument(argument), hasArgument(hasArgument), isBooleanInverted(isBooleanInverted) {}
	explicit Item() {}
};

int Parser::parse(const int argc, const char **argv)
{
	m_programName = argv[0];
	// clean the program name (remove full path)
	{
		const int lastSlashPosition = m_programName.rfind('/');
		if (lastSlashPosition != string::npos)
		{
			m_programName = m_programName.substr(lastSlashPosition + 1);
		}
	}

	vector<Item> items;
	vector<string> positional;
	vector<Subcommand> commands;
	Subcommand currentCommand = m_rootCommand;
	map<string, Option> options = currentCommand.mappedOptions();
	map<string, PositionalArgument> positionals = currentCommand.mappedPositionals();

	// state
	bool expectedArgument = false;
	bool noMoreOptions = false;
	// parse
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (expectedArgument && arg[0] != '-')
		{
			items.back().hasArgument = true;
		}
		else
		{
			if (arg[0] == '-' && !noMoreOptions)
			{
				if (arg.size() == 2 && arg[1] == '-')
				{
					// -- means no more options. only positional arguments from now on
					noMoreOptions = true;
				}
				else if (arg.size() > 1 && arg[1] == '-')
				{
					// --long-argument
					const size_t pos = arg.find('=');
					const string name = arg.substr(2, pos == string::npos ? string::npos : pos - 2);
					const auto option = options.find(name);
					if (option == options.end())
					{
						throw ParserException(string("Unknown option --") + name);
					}
					else
					{
						const string argument = pos == string::npos ? string() : arg.substr(pos + 1);
						if (name.find("no-") == 0)
						{
							items.emplace_back(name, argument, pos != string::npos, true);
						}
						else
						{
							items.emplace_back(name, argument, pos != string::npos, false);
						}
						expectedArgument = pos == string::npos && (*option).second.argument.size() != 0;
					}
				}
				else if (arg.size() > 1 && arg[1] != '-')
				{
					// -abc short argument
					for (auto it = ++arg.begin(); it != arg.end(); ++it)
					{
						const auto option = options.find(string({*it}));
						if (option == options.end())
						{
							throw ParserException(string("Unknown option -") + *it);
						}
						else
						{
							items.emplace_back(string({*it}), string(), false, false);
							expectedArgument = (*option).second.argument.size() != 0;
						}
					}
				}
				else // arg.size() == 1
				{
					// single - is a positional argument. used to mean stdin and similar
					positional.push_back(arg);
					noMoreOptions = true; // we do not allow any more options after the first positional argument
				}
			}
			else
			{
				auto cmdIt = std::find_if(currentCommand.commands.begin(), currentCommand.commands.end(), [arg](const Subcommand &cmd)
				{
					return std::find(cmd.names.begin(), cmd.names.end(), arg) != cmd.names.end();
				});
				if (!noMoreOptions && cmdIt != currentCommand.commands.end())
				{
					commands.push_back(*cmdIt);
					currentCommand = *cmdIt;
					const auto newOptions = currentCommand.mappedOptions();
					options.insert(newOptions.begin(), newOptions.end());
					const auto newPosArgs = currentCommand.mappedPositionals();
					positionals.insert(newPosArgs.begin(), newPosArgs.end());
				}
				else
				{
					positional.push_back(arg);
				}
			}
		}
	}

	// invert inverted booleans
	for (Item &item : items)
	{
		if (item.isBooleanInverted)
		{
			// it's a no-* option, start by stripping the no-
			item.option = item.option.substr(3);
			if (item.argument == "false" || item.argument == "0")
			{
				item.hasArgument = true;
				item.argument = "true";
			}
			else
			{
				item.hasArgument = true;
				item.argument = "false";
			}
		}
	}

	// populate permantent data structures
	for (const Item &item : items)
	{
		m_options[options[item.option].names.front()].push_back(item.argument);
	}
	for (const Subcommand &cmd : commands)
	{
		m_subcommands.push_back(cmd.names.front());
	}
	auto argIt = positional.begin();
	for (const auto arg : positionals)
	{
		if (argIt == positional.end() && !arg.second.optional)
		{
			throw MissingRequiredPositionalArgumentException(std::string("Missing required positional argument: ") + TermUtil::style(TermUtil::Bold, '<' + arg.second.name + '>'));
		}
		if (argIt != positional.end())
		{
			if (arg.second.repeatable)
			{
				std::copy(argIt, positional.end(), std::back_inserter(m_positionalArgs[arg.second.name]));
			}
			else
			{
				m_positionalArgs[arg.second.name] = {*argIt};
			}
		}
		++argIt;
	}

	// special handling for early exit (--help and --version)
	if (m_options.count("help") != 0)
	{
		for (const auto callback : options["help"].callbacks)
		{
			switch (callback(*this))
			{
			case Execution::ExitSuccess: return 0;
			case Execution::ExitFailure: return 1;
			case Execution::Continue: break;
			}
		}
	}
	if (m_options.count("version") != 0)
	{
		for (const auto callback : options["version"].callbacks)
		{
			switch (callback(*this))
			{
			case Execution::ExitSuccess: return 0;
			case Execution::ExitFailure: return 1;
			case Execution::Continue: break;
			}
		}
	}

	// ensure that we have all required options and arguments
	for (const auto &pair : options)
	{
		if (pair.second.isRequired && m_options.count(pair.second.names.front()) == 0)
		{
			throw MissingRequiredOptionException(string("Missing required option ") + pair.second.names.front());
		}
		if (pair.second.isArgumentRequired && m_options[pair.second.names.front()].empty())
		{
			throw MissingRequiredOptionArgument(string("Missing required argument to option ") + pair.second.names.front());
		}
	}
	for (const Subcommand &cmd : commands)
	{
		for (const PositionalArgument &arg : cmd.positionalArguments)
		{
			if (!arg.optional && m_positionalArgs.count(arg.name) == 0)
			{
				throw MissingRequiredPositionalArgumentException(string("Missing required positional argument ") + arg.name);
			}
		}
	}

	// begin by calling options since they usually only populate further higher-level structures
	for (const auto &pair : m_options)
	{
		for (const auto &callback : options[pair.first].callbacks)
		{
			switch (callback(*this))
			{
			case Execution::ExitSuccess: return 0;
			case Execution::ExitFailure: return 1;
			case Execution::Continue: break;
			}
		}
	}
	// next the positional arguments
	for (const auto &pair : m_positionalArgs)
	{
		for (const auto &callback : positionals[pair.first].callbacks)
		{
			switch (callback(*this))
			{
			case Execution::ExitSuccess: return 0;
			case Execution::ExitFailure: return 1;
			case Execution::Continue: break;
			}
		}
	}
	// finally, the subcommands
	for (const Subcommand &cmd : commands)
	{
		switch (cmd.callback(*this))
		{
		case Execution::ExitSuccess: return 0;
		case Execution::ExitFailure: return 1;
		case Execution::Continue: break;
		}
	}

	// if everyone said Execution::Continue it means success
	return 0;
}

string Parser::positionalArgument(const string &name) const
{
	if (positionalArguments(name).size() > 0)
	{
		return positionalArguments(name).front();
	}
	else
	{
		return string();
	}
}
vector<string> Parser::positionalArguments(const string &name) const
{
	return const_cast<Parser *>(this)->m_positionalArgs[name];
}

bool Parser::castValue(const std::string &name, const vector<std::string> &, const detail::Identity<bool>) const
{
	auto it = m_options.find(name);
	const std::string value = it == m_options.end() ? std::string() : (*it).second.back();
	if (it != m_options.end() && (value != "true" || value != "1" || value == ""))
	{
		return true;
	}
	else
	{
		return false;
	}
}

Execution Parser::showHelp(const vector<string> &subcommands) const
{
	using namespace TermUtil;

	vector<Option> options = m_rootCommand.options;
	vector<PositionalArgument> posArgs = m_rootCommand.positionalArguments;
	vector<Subcommand> commands = m_rootCommand.commands;
	Subcommand command = m_rootCommand;
	for (const string &cmd : subcommands)
	{
		command = command.subcommand(cmd);
		// collect all options and positional arguments in the chain of subcommands
		options.insert(options.end(), command.options.begin(), command.options.end());
		posArgs.insert(posArgs.end(), command.positionalArguments.begin(), command.positionalArguments.end());
		commands = command.commands; // intentionally overwrite, we don't want to show sibling commands
	}

	auto printUsageForCommand = [this](const Subcommand &command)
	{
		string str = "\t" + m_programName;
		if (!command.names.empty())
		{
			str += " " + command.names.front();
		}
		str += " [OPTIONS]";
		for (const PositionalArgument &arg : command.positionalArguments)
		{
			str += ' ';
			if (arg.optional)
			{
				str += '[';
			}
			str += '<' + arg.name;
			if (arg.repeatable)
			{
				str += "...";
			}
			str += '>';
			if (arg.optional)
			{
				str += ']';
			}
		}
		std::cout << str << std::endl;
	};

	std::cout << m_name << " " << m_version << std::endl
			  << std::endl
			  << style(Bold, "Usage:") << std::endl;
	printUsageForCommand(command);
	for (const Subcommand &cmd : commands)
	{
		printUsageForCommand(cmd);
	}

	if (options.size() > 0)
	{
		std::cout << std::endl
				  << style(Bold, "Options:") << std::endl;
		for (const Option &option : options)
		{
			string str;
			str += option.isRequired ? "* " : "  ";
			for (int i = 0; i < option.names.size(); ++i)
			{
				const string name = option.names[i];
				str += (name.size() == 1 ? "-" : "--") + name;
				if (option.argument.size() > 0)
				{
					str += name.size() == 1 ? ' ' : '=';
					if (!option.isArgumentRequired)
					{
						str += '[';
					}
					str += '<' + option.argument + '>';
					if (!option.isArgumentRequired)
					{
						str += ']';
					}
				}
				if (i < (option.names.size() - 1))
				{
					str += ", ";
				}
			}
			str += '\t' + option.help;
			std::cout << str << std::endl;
		}
	}

	if (!command.help.empty())
	{
		std::cout << std::endl
				  << style(Bold, "Help:") << std::endl
				  << '\t' << command.help << std::endl;
	}

	return Execution::ExitSuccess;
}
Execution Parser::showList(const vector<string> &subcommands) const
{
	using namespace TermUtil;

	std::cout << style(Bold, "Available commands:") << std::endl;
	Subcommand command = m_rootCommand;
	for (const string &cmd : subcommands)
	{
		command = command.subcommand(cmd);
	}

	for (const Subcommand &cmd : command.commands)
	{
		std::cout << ' ' << style(Bold, fg(Green, cmd.names.front())) << '\t' << StringUtil::firstLine(cmd.help) << std::endl;
	}
	return Execution::ExitSuccess;
}

Option &ParserBuilder::addHelpOption()
{
	return addOption({"help", "h"}, "Shows this help and exits").then([](const Parser &parser)
	{
		return parser.showHelp(parser.subcommands());
	});
}
Option &ParserBuilder::addVersionOption()
{
	return addOption({"version", "v"}, "Shows the version of this program and exits").then([](const Parser &p)
	{
		std::cout << p.name() << " " << p.version() << std::endl;
		return Execution::ExitSuccess;
	});
}
Subcommand &ParserBuilder::addListCommand()
{
	Subcommand &cmd = addSubcommand({"list", "l"}, "Shows a list of subcommands for the given command (or globally if none given")
			.then([](const Parser &parser)
	{
		return parser.showList(parser.positionalArguments("command"));
	});
	cmd.withPositionalArgument("command", "The command to show children for", Subcommand::Optional | Subcommand::Repeatable);
	return cmd;
}
Subcommand &ParserBuilder::addHelpCommand()
{
	Subcommand &cmd = addSubcommand({"help", "h"}, "Shows the help for the given command (or global help if none is given)")
			.then([](const Parser &parser)
	{
		return parser.showHelp(parser.positionalArguments("command"));
	});
	cmd.withPositionalArgument("command", "The command to show help for", Subcommand::Optional | Subcommand::Repeatable);
	return cmd;
}
Parser ParserBuilder::build() const
{
	return Parser(*this, m_name, m_version);
}

map<string, Option> Subcommand::mappedOptions() const
{
	map<string, Option> output;
	for (const Option &option : options)
	{
		for (const string &name : option.names)
		{
			output[name] = option;
		}
	}
	return output;
}
map<string, PositionalArgument> Subcommand::mappedPositionals()
{
	map<string, PositionalArgument> output;
	for (const PositionalArgument &arg : positionalArguments)
	{
		output[arg.name] = arg;
	}
	return output;
}
Subcommand &Subcommand::subcommand(const string &name)
{
	auto it = std::find_if(commands.begin(), commands.end(), [name](const Subcommand &cmd)
	{
		return std::find(cmd.names.begin(), cmd.names.end(), name) != cmd.names.end();
	});
	if (it == commands.end())
	{
		throw ParserException(string("Unknown subcommand: ") + name);
	}
	return *it;
}
PositionalArgument &Subcommand::withPositionalArgument(const string &arg, const string &help, const int flags)
{
	if (positionalArguments.size() > 0 && positionalArguments.back().repeatable)
	{
		throw ParserBuildException("Cannot add further positional arguments to command after adding a repeatable argument");
	}
	if (positionalArguments.size() > 0 && positionalArguments.back().optional)
	{
		throw ParserBuildException("Cannot add further positional arguments to command after adding an optional argument");
	}
	positionalArguments.push_back(PositionalArgument(arg, help, flags & Repeatable, flags & Optional));
	return positionalArguments.back();
}
Option &Subcommand::addOption(const std::initializer_list<string> &names, const string &help)
{
	options.push_back(Option{names, help});
	return options.back();
}
Subcommand &Subcommand::addSubcommand(const std::initializer_list<string> &names, const string &help)
{
	commands.push_back(Subcommand{names, help});
	return commands.back();
}

Option &Option::withArg(const string &arg)
{
	argument = arg;
	return *this;
}
}
