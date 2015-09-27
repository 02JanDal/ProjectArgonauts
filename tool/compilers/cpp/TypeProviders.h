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
#include <unordered_map>
#include <memory>

namespace Argonauts {
namespace Tool {
struct Attribute;
struct Type;

class TypeProvider
{
public:
	virtual ~TypeProvider();
	virtual std::string type(const std::string &type) const = 0;
	virtual std::string headerForType(const std::string &type) const = 0;

	virtual std::string listAppendFunction() const = 0;
	virtual std::string listSizeFunction() const = 0;
	virtual std::string listAtFunction() const = 0;
	virtual std::string listIndexType() const = 0;

	virtual std::string function(const std::string &id) const = 0;

	std::string fullType(const Attribute &attribute) const;
	std::string fullType(const std::shared_ptr<Type> &t) const;
	bool isIntegerType(const std::string &type) const;
	bool isObjectType(const std::string &type) const;

protected:
	std::string getFromMapHelper(const std::unordered_map<std::string, std::string> &map, const std::string &key, const std::string &default_) const;
};
class QtTypeProvider : public TypeProvider
{
public:
	std::string type(const std::string &type) const override;
	std::string headerForType(const std::string &type) const override;
	std::string function(const std::string &id) const override;

	std::string listAppendFunction() const override { return "append"; }
	std::string listSizeFunction() const override { return "size"; }
	std::string listAtFunction() const override { return "at"; }
	std::string listIndexType() const override { return "int"; }
};
class STLTypeProvider : public TypeProvider
{
	std::string type(const std::string &type) const override;
	std::string headerForType(const std::string &type) const override;
	std::string function(const std::string &id) const override;

	std::string listAppendFunction() const override { return "push_back"; }
	std::string listSizeFunction() const override { return "size"; }
	std::string listAtFunction() const override { return "at"; }
	std::string listIndexType() const override { return "std::size_t"; }
};
}
}
