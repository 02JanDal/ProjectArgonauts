#include "CmdParser.h"

#include <iostream>
#include <tuple>

#include "StringUtil.h"
#include "TermUtil.h"

namespace Argonauts {
namespace Util {
namespace CLI {
void Parser::printOptionsTable(const vector<std::shared_ptr<Option>> &options) const
{
	const int maxWidth = Term::currentWidth() != 0 ? Term::currentWidth() : 120;

	vector<std::tuple<string, string, string>> rows;

	for (const Option::Ptr &option : options)
	{
		string names;
		names += option->isRequired ? "* " : "  ";
		for (std::size_t i = 0; i < option->names.size(); ++i)
		{
			const string name = option->names[i];
			names += (name.size() == 1 ? "-" : "--") + name;
			if (option->argument.size() > 0)
			{
				names += name.size() == 1 ? ' ' : '=';
				if (!option->isArgumentRequired)
				{
					names += '[';
				}
				names += '<' + option->argument + '>';
				if (!option->isArgumentRequired)
				{
					names += ']';
				}
			}
			if (i < (option->names.size() - 1))
			{
				names += ", ";
			}
		}

		string help1 = option->help;

		string help2;
		if (!option->fromSet.empty())
		{
			help2 += "Allowed values: " + String::joinStrings(option->fromSet, ", ");
		}
		rows.push_back(std::make_tuple(names, help1, help2));
	}

	std::size_t longestOptionString = 0;
	for (const auto &row : rows)
	{
		const std::size_t length = std::get<0>(row).size();
		if (length >= std::size_t(maxWidth / 2))
		{
			continue;
		}
		longestOptionString = std::max(longestOptionString, length);
	}

	for (const auto &row : rows)
	{
		const string opts = std::get<0>(row);
		std::cout << "\t" << opts;
		std::size_t helpOffset;
		if (opts.size() >= std::size_t(maxWidth / 2))
		{
			helpOffset = std::size_t(maxWidth / 2);
			std::cout << "\n";
		}
		else
		{
			helpOffset = longestOptionString + 2;
		}

		vector<string> lines = String::splitStrings(std::get<1>(row), "\n");
		if (!std::get<2>(row).empty())
		{
			lines.push_back(std::get<2>(row));
		}
		std::cout << string(helpOffset - opts.size(), ' ') << lines.front() << '\n';
		for (unsigned int i = 1; i < lines.size(); ++i)
		{
			std::cout << '\t' << string(helpOffset, ' ') << lines.at(i) << '\n';
		}
	}
}

Parser::Parser(const Subcommand::Ptr &cmd, const string &name, const string &version)
	: m_rootCommand(cmd), m_name(name), m_version(version)
{
}

struct Item
{
	string option;
	string argument;
	bool hasArgument = false;
	bool isBooleanInverted = false;

	explicit Item(const string &option_, const string &argument_, const bool hasArgument_, const bool isBooleanInverted_)
		: option(option_), argument(argument_), hasArgument(hasArgument_), isBooleanInverted(isBooleanInverted_) {}
	explicit Item() {}
};

int Parser::parse(const int argc, const char **argv)
{
	m_programName = argv[0];
	// clean the program name (remove full path)
	{
		const std::size_t lastSlashPosition = m_programName.rfind('/');
		if (lastSlashPosition != string::npos)
		{
			m_programName = m_programName.substr(lastSlashPosition + 1);
		}
	}

	vector<Item> items;
	vector<string> positional;
	vector<Subcommand::Ptr> commands = {m_rootCommand};
	Subcommand::Ptr currentCommand = m_rootCommand;
	map<string, Option::Ptr> options = currentCommand->mappedOptions();
	map<string, PositionalArgument::Ptr> positionals = currentCommand->mappedPositionals();
	vector<string> positionalsOrder;
	std::transform(currentCommand->positionalArguments.begin(), currentCommand->positionalArguments.end(), std::back_inserter(positionalsOrder), [](const PositionalArgument::Ptr &arg) { return arg->name; });

	// state
	bool expectedArgument = false;
	bool noMoreOptions = false;
	// parse
	for (int i = 1; i < argc; ++i)
	{
		const string arg = argv[i];
		if (expectedArgument && (arg[0] != '-' || arg.size() == 1))
		{
			items.back().hasArgument = true;
			items.back().argument = arg;
			expectedArgument = false;
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
						expectedArgument = pos == string::npos && (*option).second->argument.size() != 0;
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
							expectedArgument = (*option).second->argument.size() != 0;
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
				auto cmdIt = std::find_if(currentCommand->commands.begin(), currentCommand->commands.end(), [arg](const Subcommand::Ptr &cmd)
				{
					return std::find(cmd->names.begin(), cmd->names.end(), arg) != cmd->names.end();
				});
				if (!noMoreOptions && cmdIt != currentCommand->commands.end())
				{
					commands.push_back(*cmdIt);
					currentCommand = *cmdIt;
					const auto newOptions = currentCommand->mappedOptions();
					options.insert(newOptions.begin(), newOptions.end());
					const auto newPosArgs = currentCommand->mappedPositionals();
					positionals.insert(newPosArgs.begin(), newPosArgs.end());
					std::transform(currentCommand->positionalArguments.begin(), currentCommand->positionalArguments.end(), std::back_inserter(positionalsOrder), [](const PositionalArgument::Ptr &a) { return a->name; });
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
		m_options[options[item.option]->names.front()].push_back(item.argument);
	}
	for (const Subcommand::Ptr &cmd : commands)
	{
		if (!cmd->names.empty()) {
			m_subcommands.push_back(cmd->names.front());
		}
	}

	// special handling for early exit
	for (const auto &pair : options) {
		if (m_options.count(pair.first) != 0 && pair.second->earlyExit) {
			for (const auto callback : pair.second->callbacks)
			{
				switch (callback(*this))
				{
				case Execution::ExitSuccess: return 0;
				case Execution::ExitFailure: return 1;
				case Execution::Continue: break;
				}
			}
		}
	}

	// populate permanent data structure for positional arguments
	auto argIt = positional.begin();
	for (const auto name : positionalsOrder)
	{
		const PositionalArgument::Ptr arg = positionals[name];
		if (argIt == positional.end() && !arg->optional)
		{
			throw MissingRequiredPositionalArgumentException(std::string("Missing required positional argument: ") + Term::style(Term::Bold, '<' + arg->name + '>'));
		}
		if (argIt != positional.end())
		{
			if (arg->repeatable)
			{
				std::copy(argIt, positional.end(), std::back_inserter(m_positionalArgs[arg->name]));
			}
			else
			{
				m_positionalArgs[arg->name] = {*argIt};
			}
		}
		++argIt;
	}

	// ensure that we have all required options and arguments
	for (const auto &pair : options)
	{
		const bool wasNotGiven = m_options.count(pair.second->names.front()) == 0;
		if (pair.second->isRequired && wasNotGiven)
		{
			throw MissingRequiredOptionException(string("Missing required option: ") + Term::style(Term::Bold, pair.second->names.front()));
		}
		if (pair.second->isArgumentRequired && !wasNotGiven && m_options[pair.second->names.front()].empty())
		{
			throw MissingRequiredOptionArgument(string("Missing required argument to option: ") + Term::style(Term::Bold, pair.second->names.front()));
		}
		if (!wasNotGiven && !pair.second->fromSet.empty())
		{
			vector<string> allowed = pair.second->fromSet;
			vector<string> opts = m_options[pair.second->names.front()];
			for (auto it = opts.begin(); it != opts.end(); ++it)
			{
				if (std::find(allowed.begin(), allowed.end(), *it) == allowed.end())
				{
					throw ArgumentNotInSetException(string("The given argument ") + *it + "is not in the set of allowed arguments (" + String::joinStrings(allowed, ", ") + ")");
				}
			}
		}
	}
	for (const Subcommand::Ptr &cmd : commands)
	{
		for (const PositionalArgument::Ptr &arg : cmd->positionalArguments)
		{
			if (!arg->optional && m_positionalArgs.count(arg->name) == 0)
			{
				throw MissingRequiredPositionalArgumentException(string("Missing required positional argument: ") + Term::style(Term::Bold, arg->name));
			}
		}
	}

	// begin by calling options since they usually only populate further higher-level structures
	for (const auto &pair : m_options)
	{
		for (const auto &callback : options[pair.first]->callbacks)
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
		for (const auto &callback : positionals[pair.first]->callbacks)
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
	for (const Subcommand::Ptr &cmd : commands)
	{
		if (cmd->callback) {
			switch (cmd->callback(*this))
			{
			case Execution::ExitSuccess: return 0;
			case Execution::ExitFailure: return 1;
			case Execution::Continue: break;
			}
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
	if (!hasPositionalArgument(name))
	{
		return {};
	}
	return m_positionalArgs.at(name);
}
bool Parser::hasPositionalArgument(const std::string &name) const
{
	return m_positionalArgs.find(name) != m_positionalArgs.end();
}

vector<std::string> Parser::rawValue(const std::string name) const
{
	return m_options.at(name);
}

bool Parser::hasOption(const std::string &name) const
{
	return m_options.find(name) != m_options.end();
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
	using namespace Term;

	vector<Option::Ptr> options = m_rootCommand->options;
	vector<PositionalArgument::Ptr> posArgs = m_rootCommand->positionalArguments;
	vector<Subcommand::Ptr> commands = m_rootCommand->commands;
	Subcommand::Ptr command = m_rootCommand;
	for (const string &cmd : subcommands)
	{
		command = command->subcommand(cmd);
		// collect all options and positional arguments in the chain of subcommands
		options.insert(options.end(), command->options.begin(), command->options.end());
		posArgs.insert(posArgs.end(), command->positionalArguments.begin(), command->positionalArguments.end());
		commands = command->commands; // intentionally overwrite, we don't want to show sibling commands
	}

	auto printUsageForCommand = [this](const Subcommand::Ptr &cmd)
	{
		string str = "\t" + m_programName;
		if (!cmd->names.empty())
		{
			vector<string> names;
			Subcommand::Ptr parent = cmd;
			while (parent)
			{
				if (!parent->names.empty())
				{
					names.push_back(parent->names.front());
				}
				parent = parent->parent;
			}

			std::reverse(names.begin(), names.end());
			str += ' ' + String::joinStrings(names, " ");
		}
		str += " [OPTIONS]";
		vector<PositionalArgument::Ptr> positionals;
		Subcommand::Ptr parent = cmd;
		while (parent)
		{
			std::reverse_copy(parent->positionalArguments.begin(), parent->positionalArguments.end(), std::back_inserter(positionals));
			parent = parent->parent;
		}
		std::reverse(positionals.begin(), positionals.end());
		for (const PositionalArgument::Ptr &arg : positionals)
		{
			str += ' ';
			if (arg->optional)
			{
				str += '[';
			}
			str += '<' + arg->name;
			if (arg->repeatable)
			{
				str += "...";
			}
			str += '>';
			if (arg->optional)
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
	for (const Subcommand::Ptr &cmd : commands)
	{
		printUsageForCommand(cmd);
	}

	if (options.size() > 0)
	{
		std::cout << std::endl
				  << style(Bold, "Options:") << std::endl;
		printOptionsTable(options);
	}

	if (!command->help.empty())
	{
		std::cout << std::endl
				  << style(Bold, "Help:") << std::endl
				  << '\t' << command->help << std::endl;
	}

	return Execution::ExitSuccess;
}
Execution Parser::showList(const vector<string> &subcommands) const
{
	using namespace Term;

	std::cout << style(Bold, "Available commands:") << std::endl;
	Subcommand::Ptr command = m_rootCommand;
	for (const string &cmd : subcommands)
	{
		command = command->subcommand(cmd);
	}

	for (const Subcommand::Ptr &cmd : command->commands)
	{
		std::cout << ' ' << style(Bold, fg(Green, cmd->names.front())) << '\t' << String::firstLine(cmd->help) << std::endl;
	}
	return Execution::ExitSuccess;
}

Option::Ptr ParserBuilder::addHelpOption()
{
	return addOption({"help", "h"}, "Shows this help and exits")->then([](const Parser &parser)
	{
		return parser.showHelp(parser.subcommands());
	})->makeEarlyExit();
}
Option::Ptr ParserBuilder::addVersionOption()
{
	return addOption({"version", "v"}, "Shows the version of this program and exits")->then([](const Parser &p)
	{
		std::cout << p.name() << " " << p.version() << std::endl;
		return Execution::ExitSuccess;
	})->makeEarlyExit();
}
Subcommand::Ptr ParserBuilder::addListCommand()
{
	Subcommand::Ptr cmd = addSubcommand({"list", "l"}, "Shows a list of subcommands for the given command (or globally if none given")
			->then([](const Parser &parser)
	{
		if (parser.hasPositionalArgument("command"))
		{
			return parser.showList(parser.positionalArguments("command"));
		}
		else
		{
			return parser.showList({});
		}
	});
	cmd->withPositionalArgument("command", "The command to show children for", Subcommand::Optional | Subcommand::Repeatable);
	return cmd;
}
Subcommand::Ptr ParserBuilder::addHelpCommand()
{
	Subcommand::Ptr cmd = addSubcommand({"help", "h"}, "Shows the help for the given command (or global help if none is given)")
			->then([](const Parser &parser)
	{
		if (parser.hasPositionalArgument("command"))
		{
			return parser.showHelp(parser.positionalArguments("command"));
		}
		else
		{
			return parser.showHelp({});
		}
	});
	cmd->withPositionalArgument("command", "The command to show help for", Subcommand::Optional | Subcommand::Repeatable);
	return cmd;
}
Parser ParserBuilder::build()
{
	return Parser(shared_from_this(), m_name, m_version);
}

map<string, Option::Ptr> Subcommand::mappedOptions() const
{
	map<string, Option::Ptr> output;
	for (const Option::Ptr &option : options)
	{
		for (const string &name : option->names)
		{
			output[name] = option;
		}
	}
	return output;
}
map<string, PositionalArgument::Ptr> Subcommand::mappedPositionals()
{
	map<string, PositionalArgument::Ptr> output;
	for (const PositionalArgument::Ptr &arg : positionalArguments)
	{
		output[arg->name] = arg;
	}
	return output;
}
Subcommand::Ptr Subcommand::subcommand(const string &name)
{
	auto it = std::find_if(commands.begin(), commands.end(), [name](const Subcommand::Ptr &cmd)
	{
		return std::find(cmd->names.begin(), cmd->names.end(), name) != cmd->names.end();
	});
	if (it == commands.end())
	{
		throw ParserException(string("Unknown subcommand: ") + name);
	}
	return *it;
}
PositionalArgument::Ptr Subcommand::withPositionalArgument(const string &arg, const string &help_, const int flags)
{
	if (positionalArguments.size() > 0 && positionalArguments.back()->repeatable)
	{
		throw ParserBuildException("Cannot add further positional arguments to command after adding a repeatable argument");
	}
	if (positionalArguments.size() > 0 && positionalArguments.back()->optional)
	{
		throw ParserBuildException("Cannot add further positional arguments to command after adding an optional argument");
	}
	positionalArguments.push_back(std::make_shared<PositionalArgument>(arg, help_, flags & Repeatable, flags & Optional));
	return positionalArguments.back();
}
Option::Ptr Subcommand::addOption(const std::initializer_list<string> &names_, const string &help_)
{
	for (auto it = options.begin(); it != options.end(); ++it)
	{
		for (const std::string &name : (*it)->names)
		{
			for (const std::string &newName : names_)
			{
				if (name == newName)
				{
					throw ParserBuildException(std::string("Attempted to add option '") + newName + "' to subcommand '" + this->names[0] + "', which already exists.");
				}
			}
		}
	}
	options.push_back(std::make_shared<Option>(names_, help_));
	return options.back();
}
Subcommand::Ptr Subcommand::addSubcommand(const std::initializer_list<string> &names_, const string &help_)
{
	commands.push_back(std::make_shared<Subcommand>(names_, help_, shared_from_this()));
	return commands.back();
}
}
}
}
