#pragma once
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace mygfx {

using String = std::string;

struct ConfigValue;
using ChildMap = std::multimap<std::string_view, ConfigValue>;
using ChildMapIterator = std::pair<ChildMap::const_iterator, ChildMap::const_iterator>;
using ValueBase = std::variant<std::nullptr_t, bool, int, double, std::string_view, std::vector<ConfigValue>, ChildMap>;

struct ConfigValue : public ValueBase {
    enum Type {
        Null,
        Bool,
        Int,
        Float,
        Source,
        List,
        Object
    };

    using ValueBase::ValueBase;

    Type getType() const { return (Type)index(); }
    bool isNull() const { return getType() == Type::Null; }

    String getString() const;

    size_t getChildCount() const;
    size_t getChildCount(const std::string_view& key) const;
    ChildMapIterator getIterator(const std::string_view& key) const;
    const ConfigValue* find(const std::string_view& key) const;
    std::string_view name;
};

class Config {
public:
    Config();

    bool parse(const std::string& text);
    const ConfigValue* find(const std::string_view& key);

protected:
    std::string mText;
    ConfigValue mRoot;
};

}