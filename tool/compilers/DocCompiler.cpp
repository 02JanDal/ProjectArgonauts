#include "DocCompiler.h"

#include "util/CmdParser.h"
#include "util/FSUtil.h"
#include "tool/DataTypes.h"

#include <boost/filesystem.hpp>

#include "templates/Doc.ect.h"

namespace Argonauts {
namespace Tool {
void DocCompiler::setup(std::shared_ptr<Util::CLI::Subcommand> &builder)
{
	builder->addOption({"output", "o"}, "The directory to which to write the resulting HTML file")
			->withRequiredArg("DIR")
			->beingRequired()
			->applyTo(this, &DocCompiler::m_outdir);
}

bool DocCompiler::run(const Util::CLI::Parser &parser, const File &file)
{
	const boost::filesystem::path outFile = boost::filesystem::path(m_outdir) / "index.html";

	if (!boost::filesystem::exists(m_outdir)) {
		boost::filesystem::create_directories(m_outdir);
	} else if (!boost::filesystem::is_directory(m_outdir)) {
		throw ArgonautsException(std::string("Path to ") + m_outdir + " contains a non-directory");
	} else if (boost::filesystem::exists(outFile) && !boost::filesystem::is_regular_file(outFile)) {
		throw ArgonautsException(std::string("Existing item ") + outFile.string() + " is not a regular file");
	}
	FSUtil::writeFile(outFile.string(), generateDoc(boost::filesystem::path(m_outdir).stem().string(), file));
	return true;
}
}
}
