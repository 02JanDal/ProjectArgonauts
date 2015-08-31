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

#include "DocCompiler.h"

#include <iostream>
#include <boost/filesystem.hpp>

#include "util/CmdParser.h"
#include "util/FSUtil.h"
#include "tool/DataTypes.h"

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
		throw Util::Exception(std::string("Path to ") + m_outdir + " contains a non-directory");
	} else if (boost::filesystem::exists(outFile) && !boost::filesystem::is_regular_file(outFile)) {
		throw Util::Exception(std::string("Existing item ") + outFile.string() + " is not a regular file");
	}

	Util::FS::writeFile(outFile.string(), generateDoc(boost::filesystem::path(parser.positionalArgument("file")).stem().string(), file));
	std::cout << "Generated documentation for " << parser.positionalArgument("file") << " in " << outFile.string() << std::endl;
	return true;
}

int DocCompiler::resolverFlags() const
{
	return ResolveIncludes;
}
}
}
