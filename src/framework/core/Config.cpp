#include "Config.h"
#include "utils/Log.h"
#include <array>
#include <format>

namespace mygfx {

class ParseError : public std::exception {
public:
    explicit ParseError(const std::string& _Message)
        : mMessage(_Message)
    {
    }

    explicit ParseError(const char* _Message)
        : mMessage(_Message)
    {
    }
    
    char const* what() const noexcept override
    {
        return mMessage.c_str();
    }

private:
    String mMessage;
};

static ParseError parseError(const std::string_view& source, size_t at, const std::string_view& expected)
{
    auto token = source[at];
    auto near = source.substr(at, 25);
    auto line = 1;
    for (auto i = 0; i < at; ++i)
        if (source[i] == 10)
            ++line;
    return ParseError(std::format("Unexpected token {}, expected {} on line {} near {}", token, expected, line, near));
}

static ParseError parseError(const std::string_view& source, size_t at, char expected)
{
    auto token = source[at];
    auto near = source.substr(at, 25);
    auto line = 1;
    for (auto i = 0; i < at; ++i)
        if (source[i] == 10)
            ++line;

    return ParseError(std::format("Unexpected token {}, expected {} on line {} near {}", token, expected, line, near));
}

inline static std::array<uint32_t, 4> characterMask(const std::string_view& str)
{
    std::array<uint32_t, 4> mask { 0 };
    for (auto i = 0; i < str.length(); ++i) {
        auto c = str[i];
        mask[(c >> 5)] |= (1 << (c % 32));
    }
    return mask;
}

inline static bool hasChar(std::array<uint32_t, 4> mask, char c)
{
    return (mask[(c >> 5)] & (1 << (c % 32))) != 0;
}

const std::array<uint32_t, 4> NUMBER = characterMask("-+0123456789");
const std::array<uint32_t, 4> NUMBER_EXP = characterMask(".eE");
const std::array<uint32_t, 4> ID_TERM = characterMask(" \t\n\r=:,[]");
const std::array<uint32_t, 4> WHITESPACE = characterMask(" \n\r\t,");

class SJSON {
public:
    auto parse(std::string_view str)
    {
        s = str;
        i = 0;

        try {
            return proot();
        } catch (const ParseError& e) {
            LOG_ERROR(e.what());
            return ConfigValue {};
        }
    }

private:
    size_t i = 0;
    std::string_view s;

    static const char QUOTE = '\"', SLASH = '/', BACKSLASH = '\\', OPENBRACE = '{', CLOSEBRACE = '}',
                      COLON = ':', STAR = '*', CR = '\r', LF = '\n',
                      OPENSQUAREBRAC = '[', CLOSESQUAREBRAC = ']';

    void ws()
    {
        while (i < s.length()) {
            if (s[i] == SLASH) { // "/"
                ++i;
                if (s[i] == SLASH)
                    while (s[++i] != 10)
                        ; // "\n"
                else if (s[i] == 42) // "*"
                    while (s[++i] != 42)
                        ;
            } else if (!hasChar(WHITESPACE, s[i])) {
                break;
            }
            ++i;
        }
    }

    void consume(char c)
    {
        ws();
        if (s[i++] != c)
            throw parseError(s, i - 1, c);
    }

    void consumeKeyword(const std::string_view& kw)
    {
        ws();
        for (auto c : kw) {
            if (s[i++] != c)
                throw parseError(s, i - 1, { c });
        }
    }

    bool tryParse(const std::string_view& kw)
    {
        ws();
        auto start = i;
        for (auto c : kw) {
            if (s[i++] != c) {
                i = start;
                return false;
            }
        }

        if (!hasChar(ID_TERM, s[i])) {
            i = start;
            return false;
        }

        return true;
    }

    ConfigValue pvalue()
    {
        ws();
        const auto c = s[i];
        if (hasChar(NUMBER, c))
            return pnumber();
        if (c == OPENBRACE)
            return pobject(); // "{"
        if (c == OPENSQUAREBRAC)
            return parray(); // "["
        if (c == QUOTE)
            return pstring(); // "

        if (c == 't') {
            if (tryParse("true")) {
                return true;
            }
        }

        if (c == 'f') {
            if (tryParse("false")) {
                return false;
            }
        }

        if (c == 'n') {
            if (tryParse("null")) {
                return false;
            }
        }

        return pidentifier();

        // throw parseError(s, i, "number, {, [, \", true, false or null");
    }

    ConfigValue pnumber()
    {
        ws();
        const auto start = i;
        bool isFloat = false;
        for (; i < s.length(); ++i) {
            char c = s[i];
            bool expc = hasChar(NUMBER_EXP, c);
            isFloat |= expc;
            if (!expc && !hasChar(NUMBER, c))
                break;
        }
        auto n = s.substr(start, i - start);

        if (isFloat) {
            double dv;
            char* end;
            dv = std::strtod(n.data(), &end);
            return dv;
        } else {
            int64_t iv;
            char* end;
            iv = std::strtoll(n.data(), &end, 0);
            return iv;
        }
    }

    std::string_view pstring()
    {
        // Literal string
        if (s[i] == QUOTE && s[i + 1] == QUOTE && s[i + 2] == QUOTE) {
            i += 3;
            auto start = i;
            for (; s[i] != QUOTE || s[i + 1] != QUOTE || s[i + 2] != QUOTE; ++i)
                ;
            i += 3;
            return s.substr(start, i - 3 - start);
        }

        const auto start = i;
        bool escape = false;
        consume(QUOTE);
        for (; s[i] != QUOTE; ++i) { // unescaped "
            // if (s[i] == 92) {
            //     ++i;
            //    escape = true;
            //}
        }
        consume(QUOTE);
        // if (!escape)
        return s.substr(start + 1, i - 1 - (start + 1));
        /*
                i = start;
                std::string_view octets;
                consume(34);
                for (; s[i] != 34; ++i) { // unescaped "

                    if (s[i] == 92) {
                        ++i;
                        if (s[i] == 98)
                            octets.push_back(8); // \b
                        else if (s[i] == 102)
                            octets.push_back(12); // \f
                        else if (s[i] == 110)
                            octets.push_back(10); // \n
                        else if (s[i] == 114)
                            octets.push_back(13); // \r
                        else if (s[i] == 116)
                            octets.push_back(9); // \t
                        else if (s[i] == 117) { // \u
                            ++i;
                            octets.push_back(16 * (s[i] - 48) + s[i + 1] - 48);
                            i += 2;
                            octets.push_back(16 * (s[i] - 48) + s[i + 1] - 48);
                            i += 2;
                        } else
                            octets.push_back(s[i]); // \" \\ \/
                    } else
                        octets.push_back(s[i]);
                }
                consume(34);
                return octets;*/
    }

    std::string_view psource()
    {
        consume(OPENBRACE); // "{"
        size_t start = i;
        int depth = 1;
        for (; i < s.length(); ++i) {
            if (s[i] == OPENBRACE) {
                depth++;
            }
            if (s[i] == CLOSEBRACE) {
                depth--;

                if (depth == 0) {
                    break;
                }
            }
        }

        return s.substr(start, i - 1 - start);
    }

    std::vector<ConfigValue> parray()
    {
        std::vector<ConfigValue> ar;
        ws();
        consume(OPENSQUAREBRAC); // "["
        ws();
        for (; s[i] != CLOSESQUAREBRAC; ws()) // "]"
            ar.push_back(pvalue());

        consume(CLOSESQUAREBRAC);
        return ar;
    }

    std::string_view pidentifier()
    {
        ws();
        if (i == s.length()) // Catch whitespace EOF
            return {};
        if (s[i] == QUOTE)
            return pstring();

        auto start = i;
        for (; !hasChar(ID_TERM, s[i]); ++i)
            ;
        return s.substr(start, i - start);
    }

    void pelements(ElementMap& object)
    {
        auto key = pidentifier();
        if (key.empty()) {
            return;
        }

        ws();
        std::string_view name;
        if (s[i] == ':') {
            consume(':');
        } else if (s[i] == '=') {
            consume('=');
        } else if (s[i] == '"') {
            auto str = pstring();
            ws();
            if (s[i] != OPENBRACE) {
                object.emplace(key, str);
                return;
            }
            name = str;
        }

        if (s[i] == OPENBRACE) {
            if (key.starts_with('@')) {
                auto e = object.emplace(key, psource());
                e->second.name = name;
                return;
            }
        }

        auto e = object.emplace(key, pvalue());
        e->second.name = name;
    }

    ConfigValue pobject()
    {
        ElementMap object;
        consume(OPENBRACE); // "{"
        ws();
        for (; s[i] != CLOSEBRACE; ws()) { // "}"
            pelements(object);
        }
        consume(CLOSEBRACE); // "}"
        return object;
    }

    ConfigValue proot()
    {
        ws();
        if (s[i] == OPENBRACE)
            return pobject();

        ElementMap object;
        while (i < s.length()) {
            pelements(object);
        }

        ws();
        if (i != s.length())
            throw parseError(s, i, "end-of-string");

        return object;
    }
};

String ConfigValue::getString() const
{
    if (getType() == Str) {
        auto& str = std::get<std::string_view>(*this);
        return String { str.data(), str.length() };
    }
    return "";
}

size_t ConfigValue::getArrayCount() const
{
    if (getType() != ConfigValue::List) {
        return 0;
    }

    return std::get<ElementList>(*this).size();
}

const ConfigValue& ConfigValue::getAt(size_t index) const
{
    return std::get<ElementList>(*this).at(index);
}

size_t ConfigValue::getElementCount() const
{
    if (getType() != ConfigValue::Object) {
        return 0;
    }

    auto& children = std::get<ElementMap>(*this);
    return children.size();
}

size_t ConfigValue::getElementCount(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return 0;
    }

    auto& children = std::get<ElementMap>(*this);
    return children.count(key);
}

ChildMapIterator ConfigValue::getIterator() const
{
    if (getType() != ConfigValue::Object) {
        return {};
    }

    auto& children = std::get<ElementMap>(*this);
    return { children.begin(), children.end() };
}

ChildMapIterator ConfigValue::getIterator(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return {};
    }

    auto& children = std::get<ElementMap>(*this);
    return children.equal_range(key);
}

const ConfigValue* ConfigValue::find(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return nullptr;
    }

    auto& children = std::get<ElementMap>(*this);
    auto it = children.find(key);
    if (it != children.end()) {
        return &it->second;
    }
    return nullptr;
}

Config::Config() = default;

bool Config::parse(const std::string& text)
{
    mText = text;
    SJSON parser;
    mRoot = parser.parse({ mText.data(), mText.length() });
    return !mRoot.isNull();
}

const ConfigValue* Config::find(const std::string_view& key)
{
    return mRoot.find(key);
}
}