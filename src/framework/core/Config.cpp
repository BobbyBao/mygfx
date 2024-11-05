#include "Config.h"
#include "utils/Log.h"
#include <array>

namespace mygfx {

static void parseError(const std::string_view& source, size_t at, const std::string_view& expected)
{
    auto token = source[at];
    auto near = source.substr(at, 25);
    auto line = 1;
    for (auto i = 0; i < at; ++i)
        if (source[i] == 10)
            ++line;

    return LOG_ERROR("Unexpected token {}, expected {} on line {} near {}", token, expected, line, near);
}

static void parseError(const std::string_view& source, size_t at, char expected)
{
    auto token = source[at];
    auto near = source.substr(at, 25);
    auto line = 1;
    for (auto i = 0; i < at; ++i)
        if (source[i] == 10)
            ++line;

    return LOG_ERROR("Unexpected token {}, expected {} on line {} near {}", token, expected, line, near);
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
const std::array<uint32_t, 4> ID_TERM = characterMask(" \t\n=:");
const std::array<uint32_t, 4> WHITESPACE = characterMask(" \n\r\t,");

class SJSON {

public:
    auto parse(std::string_view str)
    {
        s = str;
        i = 0;
        return proot();
    }

private:
    size_t i = 0;
    std::string_view s;

    void ws()
    {
        while (i < s.length()) {
            if (s[i] == 47) { // "/"
                ++i;
                if (s[i] == 47)
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
            parseError(s, i - 1, c);
    }

    void consumeKeyword(const std::string_view& kw)
    {
        ws();
        for (auto c : kw) {
            if (s[i++] != c)
                parseError(s, i - 1, { c });
        }
    }

    ConfigValue pvalue()
    {
        ws();
        const auto c = s[i];
        if (hasChar(NUMBER, c))
            return pnumber();
        if (c == 123)
            return pobject(); // "{"
        if (c == 91)
            return parray(); // "["
        if (c == 34)
            return pstring(); // "
        if (c == 116) {
            consumeKeyword("true");
            return true;
        }
        if (c == 102) {
            consumeKeyword("false");
            return false;
        }
        if (c == 110) {
            consumeKeyword("null");
            return nullptr;
        }
        parseError(s, i, "number, {, [, \", true, false or null");
        return {};
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
            std::from_chars(n.data(), n.data() + n.length(), dv);
            return dv;
        } else {
            int iv;
            std::from_chars(n.data(), n.data() + n.length(), iv);
            return iv;
        }
    }

    std::string_view pstring()
    {
        // Literal string
        if (s[i] == 34 && s[i + 1] == 34 && s[i + 2] == 34) {
            i += 3;
            auto start = i;
            for (; s[i] != 34 || s[i + 1] != 34 || s[i + 2] != 34; ++i)
                ;
            i += 3;
            return s.substr(start, i - 3 - start);
        }

        const auto start = i;
        bool escape = false;
        consume(34);
        for (; s[i] != 34; ++i) { // unescaped "
            // if (s[i] == 92) {
            //     ++i;
            //    escape = true;
            //}
        }
        consume(34);
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

    std::string_view psource() {
        return std::string_view{};
    }

    std::vector<ConfigValue> parray()
    {
        std::vector<ConfigValue> ar;
        ws();
        consume(91); // "["
        ws();
        for (; s[i] != 93; ws()) // "]"
            ar.push_back(pvalue());

        consume(93);
        return ar;
    }

    std::string_view pidentifier()
    {
        ws();
        if (i == s.length()) // Catch whitespace EOF
            return {};
        if (s[i] == 34)
            return pstring();

        auto start = i;
        for (; !hasChar(ID_TERM, s[i]); ++i)
            ;
        return s.substr(start, i - start);
    }

    void pchildren(ChildMap& object) {
        auto key = pidentifier();
        ws();
        std::string_view name;
        if (s[i] == ':') {
            consume(':');
        } else if (s[i] == '=') {
            consume('=');
        } else if (s[i] == '"') {
            auto str = pstring();
            ws();
            if (s[i] != '{') {
                object.emplace(key, str);
                return;
            }
            name = str;
        }

        if (s[i] == '{') {
            if (key.starts_with('@')) {
                auto child = object.emplace(key, psource());
                child->second.name = name;
                return;
            }
        }

        auto child = object.emplace(key, pvalue());
        child->second.name = name;
    }

    ConfigValue pobject()
    {
        ChildMap object;
        consume(123); // "{"
        ws();
        for (; s[i] != 125; ws()) { // "}"
            pchildren(object);
        }
        consume(125); // "}"
        return object;
    }

    ConfigValue proot()
    {
        ws();
        if (s[i] == 123)
            return pobject();

        ChildMap object;
        while (i < s.length()) { // "}"
            pchildren(object);
        }

        ws();
        if (i != s.length())
            parseError(s, i, "end-of-string");

        return object;
    }
};

String ConfigValue::getString() const {
    if (getType() == Source) {
        auto& str = std::get<std::string_view>(*this);
        return String { str.data(), str.length() }; 
    }
    return "";
}

size_t ConfigValue::getChildCount() const
{
    if (getType() != ConfigValue::Object) {
        return 0;
    }

    auto& children = std::get<ChildMap>(*this);
    return children.size();
}

size_t ConfigValue::getChildCount(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return 0;
    }

    auto& children = std::get<ChildMap>(*this);
    return children.count(key); 
}

ChildMapIterator ConfigValue::getIterator(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return {};
    }

    auto& children = std::get<ChildMap>(*this);
    return children.equal_range(key);
}

const ConfigValue* ConfigValue::find(const std::string_view& key) const
{
    if (getType() != ConfigValue::Object) {
        return nullptr;
    }

    auto& children = std::get<ChildMap>(*this);
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