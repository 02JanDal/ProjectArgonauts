# ProjectArgonauts

ProjectArgonauts is a tool and a runtime library for generating code and documentation for an <abbr title="Interface Description Language">IDL</abbr>.
It works similarly to how projects like Protobufs, Cap'n'proto or Flatbuffers work, with the difference that it also supports documentation generation
as a first class citizen, and it is also not limited to a single format. ProjectArgonauts also does not include an RPC layer, and will not do so.

## Features

* C-like IDL
* Code generation for C++ (more to come)
* Documentation generation
* Supported formats: JSON (more to come)
* JSON Schema import/export (in progress)

## Roadmap/wishlist

* Code generation for Ruby and Java (and others!)
* More supported formats (MsgPack, BSON, protobufs, Cap'n'proto, Flatbuffers, etc.)
* High-performance parser generator
    * Currently a relatively generic parser is used, it would be beneficial to add a way to generate LALR or similar parsers.

## For users

### The IDL syntax

```cpp
@doc("The gender of someone")
enum Gender {
	Male = 1;
	Female = 2;
	@doc("Wait what?")
	Unknown = 3;
}

@doc(brief="A telephone number", extended="Only digits!")
@verification.regex("[0-9]+")
using PhoneNumber = String;

@doc.brief("A user is a person")
struct User {
	@doc("The full name of the person")
	name String;

	@optional
	phone PhoneNumber;
}
```

Annotations (`@<id>(<arguments>)`) are added before the enum/enum-entry/using/struct/attribute they apply to.

#### Enumerations

A list of values. When used as a type one of the given entries (it's name or it's value) is allowed. If none matches it is an error.
Each entry is required to have an integer index.

#### Type aliases

```cpp
using NewType = Type;
```
From here on, writing NewType is equal to writing Type. Additionally, verifications and similar are also inherited for attributes using the new type.

#### Structures

A struct contains a list of member attributes. Each attribute requires at least a name and a type (inversed order from C/C++). Some formats require adding an index (` = <index>`, where `<index>` is a number) after the type.

##### Inheritance/including

A struct can inherit from another struct, in which case all attributes of the parent struct are prepended to those of this struct.

```cpp
struct Base {
	a UInt32;
}
struct Inherited : Base {
	b UInt64;
}
// is equal to
struct Inherited {
	a UInt32;
	b UInt64;
}
```

In case of name clashes the attribute of the sub-struct is used, thus allowing overriding of attributes.

#### Annotations

```cpp
@doc(brief="alpha", extended="beta")
// is equivalent to
@doc.brief("alpha")
@doc.extended("beta")

// also allowed:
@doc("abc", extended="def")
@verification.oneOf("a", "b", "c")
```

Valid IDs:

| ID                         | Description                                      | Argument     | Valid context\* |
| -------------------------- | ------------------------------------------------ | ------------ | --------------- |
| doc.brief                  | Short summary documentation                      | string       | Anywhere        |
| doc.extended               | Full documentation, excluding brief              | string       | Anywhere        |
| doc                        | Full documentation, including brief              | string       | Anywhere        |
| hidden                     | Don't output this in the documentation           |              | Anywhere        |
| optional                   | This attribute is not required                   |              | Attributes: All |
| variant.selectBy           | How to choose which type to use when parsing\*\* | string\*\*   | A&A: Variant    |
| verification.regex         | Must match this regular expression               | string       | A&A: String     |
| verification.format        | Must be of this format                           | string\*\*\* | A&A: String     |
| verification.minLength     | Must be at least this many characters            | integer      | A&A: String     |
| verification.maxLength     | Must be no more than this many characters        | integer      | A&A: String     |
| verification.fixedLength   | Must be exactly this many characters             | integer      | A&A: String     |
| verification.min           | Must be at least this big                        | integer      | A&A: Integers   |
| verification.max           | Must be no larger than this                      | integer      | A&A: Integers   |
| verification.fixed         | Must be exactly this value                       | integer      | A&A: Integers   |
| verification.oneOf         | Must be one of the these values                  | list<string> | A&A: String     |
| verification.requireEither | Exactly one of these attributes must exist       | list<string> | Structure       |

\* A&A: Attributes and type aliases of this type<br/>
\*\* Some times the parser will not be able to determine the type of a `Variant`. This is the case when using more then one integer type or multiple structures/enumerations. When this is the case, this annotation is required, see below for valid values.<br/>
\*\*\* See below for available formats

##### Values for `variant.selectBy`

* `firstFieldAvailable`
    * Must only contain structs. The name of the first fields must be unique.
    * Looks at the first field of each struct. If the field exists in the input data that struct is choosen.

##### Values for `verification.format`

* `url`

#### Types

These types are available:

* `UInt8`, `UInt16`, `UInt32`, `UInt64`: unsigned integer of the specified number of bits
* `Int8`, `Int16`, `Int32`, `Int64`: signed integer of the specifed number of bits
* `String`
* `Double`
* `Bool`
* `List<Type>`: A list of `Type`. Can be nested.
* `Map<Key, Value>`: A map of `Key`s to `Value`s. Can be nested.
* `Variant<Type1, Type2..., TypeN>`: Can be any of the given types.
* Any enumeration, structure or type alias definied in the same file.

## For contributors

### Code

The follow top-level directories are available:

* embeddedcpptemplate: Simple executable to generates C++ code from ERB-like templates. Used for code and documentation generation.
* example: Simple example usage.
* runtime: The shared runtime library. The library also includes `util`.
* test: Unit tests
* tool: The developer tool (generators, validator, JSON-schema importer etc.)
* util: Library used by most other parts. Contains both utilities for string handling and similar as well as several JSON-related classes, for things like parsing, JSON Reference, JSON Pointer, JSON Schema etc.

### Contributing

1. Fork on Github
2. Clone your fork locally
3. Edit code (preferably in a branch)
4. Commit and push your changes
5. Open a PR (pull request) on Github

Try to keep the same coding style as used in the rest of the project.
