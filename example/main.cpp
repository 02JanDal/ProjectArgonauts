#include "grammar/Site.arg.h"

#include <iostream>

#include <util/json/JsonSaxWriter.h>
#include <runtime/Serializer.h>

int main(int argc, char **argv)
{
	User::Builder userBuilder;
	userBuilder.set_name("Arthur Philip Dent");
	userBuilder.set_age(12);
	userBuilder.set_metadata({{"meta1", false}, {"meta2", std::string("asdf")}});

	Site::Builder builder;
	builder.set_name("asdf");
	builder.set_type(SiteType::Governmental);
	builder.add_users(userBuilder.build().rebuild());
	userBuilder.set_name("Blarg");
	builder.add_users(userBuilder);
	const Site site = builder.build();

	Argonauts::Util::StringOutputStream stream;
	Argonauts::Util::Json::SaxWriter writer(&stream);
	Argonauts::Runtime::SaxSinkSerializer serializer(&writer);
	site.serialize(&serializer);
	std::cout << stream.result() << std::endl;

	return 0;
}
