#include <catch.hpp>

#include <iostream>
#include <sstream>

#include "util/CmdParser.h"

using namespace Argonauts::Util;
using namespace CLI;

class StringRedirection
{
	std::stringbuf m_buffer;
	std::streambuf *m_oldBuffer;
public:
	explicit StringRedirection()
	{
		m_oldBuffer = std::cout.rdbuf(&m_buffer);
	}
	~StringRedirection()
	{
		std::cout.rdbuf(m_oldBuffer);
	}

	std::stringbuf &buffer() { return m_buffer; }
};

TEST_CASE("can build everything", "[CmdParser]") {
	StringRedirection cout;
	std::stringbuf &buf = cout.buffer();

	ParserBuilder::Ptr builder = std::make_shared<ParserBuilder>("argonauts", "0.0.0.0.1", "This is the global help text");
	builder->addSubcommand({"compile", "c"}, "Compiling!")->setup([](Subcommand::Ptr &cmd)
	{
		cmd->withPositionalArgument("file", "Filename!");
		cmd->withPositionalArgument("extra_file", "Another file!", Subcommand::Optional);
		cmd->addOption({"output", "o"}, "Output file!")->beingRequired();
	});
	builder->addSubcommand({"import", "i"}, "Importing!")->setup([](Subcommand::Ptr &cmd)
	{
		cmd->withPositionalArgument("schema", "The JSON-Schema(s) file to import", Subcommand::Repeatable);
		cmd->addOption({"output", "o"}, "If name of an existing directory or ending with / each schema will be put in a separate file, otherwise all schemas will be put in the file with this name")
				->withRequiredArg("file")
				->beingRequired();
		cmd->addOption({"force", "no-force", "f"}, "If a file with the same name already exists force overwrite it");
	});
	builder->addOption({"global"}, "A global option!")->withArg("optionalarg");
	builder->addVersionOption();
	builder->addHelpOption();
	builder->addListCommand();
	builder->addHelpCommand();
	REQUIRE_NOTHROW(builder->build());
	Parser parser = builder->build();

	SECTION("and parse long help") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "--help";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(parser.hasOption("help"));
		REQUIRE(parser.subcommands().empty());
		REQUIRE(buf.str().find("argonauts 0.0.0.0.1\n") == 0);
		REQUIRE(buf.str().find("--version") != std::string::npos);
	}
	SECTION("and parse short help") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "-h";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(parser.hasOption("help"));
		REQUIRE(parser.subcommands().empty());
		REQUIRE(buf.str().find("argonauts 0.0.0.0.1\n") == 0);
		REQUIRE(buf.str().find("--version") != std::string::npos);
	}
	SECTION("and parse help command") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "help";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(!parser.hasOption("help"));
		REQUIRE(parser.subcommands() == std::vector<std::string>({"help"}));
		REQUIRE(buf.str().find("argonauts 0.0.0.0.1\n") == 0);
		REQUIRE(buf.str().find("--version") != std::string::npos);
	}

	SECTION("and parse long version") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "--version";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(parser.hasOption("version"));
		REQUIRE(parser.subcommands().empty());
		REQUIRE(buf.str().find("argonauts 0.0.0.0.1\n") == 0);
	}
	SECTION("and parse long version") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "-v";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(parser.hasOption("version"));
		REQUIRE(parser.subcommands().empty());
		REQUIRE(buf.str().find("argonauts 0.0.0.0.1\n") == 0);
	}

	SECTION("and parse option without it's optional argument") {
		const char *argv[2];
		argv[0] = "argonauts";
		argv[1] = "--global";
		REQUIRE(parser.parse(2, argv) == 0);
		REQUIRE(buf.str() == "");
		REQUIRE(parser.hasOption("global"));
		REQUIRE(parser.option<std::string>("global").empty());
		REQUIRE(parser.subcommands().empty());
	}
	SECTION("and parse option with it's optional argument") {
		const char *argv[3];
		argv[0] = "argonauts";
		argv[1] = "--global";
		argv[2] = "bla";
		REQUIRE(parser.parse(3, argv) == 0);
		REQUIRE(buf.str() == "");
		REQUIRE(parser.hasOption("global"));
		REQUIRE(parser.option<std::string>("global") == "bla");
		REQUIRE(parser.subcommands().empty());
	}

	SECTION("and parse single positional argument") {
		const char *argv[4];
		argv[0] = "argonauts";
		argv[1] = "compile";
		argv[2] = "-o";
		argv[3] = "asdf";
		REQUIRE(parser.parse(4, argv) == 0);
		REQUIRE(buf.str() == "");
		REQUIRE(parser.subcommands() == std::vector<std::string>({"compile"}));
		REQUIRE(parser.hasOption("output"));
		REQUIRE(parser.positionalArgument("file") == "asdf");
		REQUIRE(parser.positionalArguments("file") == std::vector<std::string>({"asdf"}));
		REQUIRE(!parser.hasPositionalArgument("extra_file"));
		REQUIRE(parser.positionalArgument("extra_file").empty());
	}
	SECTION("and parse single positional argument + optional positional argument") {
		const char *argv[5];
		argv[0] = "argonauts";
		argv[1] = "compile";
		argv[2] = "-o";
		argv[3] = "asdf";
		argv[4] = "fdsa";
		REQUIRE(parser.parse(5, argv) == 0);
		REQUIRE(buf.str() == "");
		REQUIRE(parser.subcommands() == std::vector<std::string>({"compile"}));
		REQUIRE(parser.hasOption("output"));
		REQUIRE(parser.positionalArgument("file") == "asdf");
		REQUIRE(parser.positionalArguments("file") == std::vector<std::string>({"asdf"}));
		REQUIRE(parser.hasPositionalArgument("extra_file"));
		REQUIRE(parser.positionalArgument("extra_file") == "fdsa");
		REQUIRE(parser.positionalArguments("extra_file") == std::vector<std::string>({"fdsa"}));
	}
	SECTION("and parse repeatable positional argument") {
		const char *argv[6];
		argv[0] = "argonauts";
		argv[1] = "import";
		argv[2] = "-o";
		argv[3] = "";
		argv[4] = "asdf";
		argv[5] = "fdsa";
		REQUIRE(parser.parse(6, argv) == 0);
		REQUIRE(buf.str() == "");
		REQUIRE(parser.subcommands() == std::vector<std::string>({"import"}));
		REQUIRE(parser.hasOption("output"));
		REQUIRE(parser.positionalArgument("schema") == "asdf");
		REQUIRE(parser.positionalArguments("schema") == std::vector<std::string>({"asdf", "fdsa"}));
	}

	SECTION("and parse multiple short options + short option with value") {
		const char *argv[5];
		argv[0] = "argonauts";
		argv[1] = "import";
		argv[2] = "-fo";
		argv[3] = "asdf";
		argv[4] = "fff";
		REQUIRE(parser.parse(5, argv) == 0);
		REQUIRE(!parser.hasOption("help"));
		REQUIRE(!parser.hasOption("version"));
		REQUIRE(parser.hasOption("output"));
		REQUIRE(parser.option<std::string>("output") == "asdf");
		REQUIRE(parser.positionalArgument("schema") == "fff");
	}
	SECTION("and parse long option with inline argument") {
		const char *argv[4];
		argv[0] = "argonauts";
		argv[1] = "import";
		argv[2] = "--output=asdf";
		argv[3] = "fff";
		REQUIRE(parser.parse(4, argv) == 0);
		REQUIRE(parser.hasOption("output"));
		REQUIRE(parser.option<std::string>("output") == "asdf");
		REQUIRE(parser.positionalArgument("schema") == "fff");
	}
}
