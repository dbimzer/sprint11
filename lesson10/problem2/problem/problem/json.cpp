#include "json.h"
#include <sstream>
#include <iomanip>
#include <cctype>

using namespace std;

namespace json {

namespace {

Node LoadNode(istream& input);

using Number = variant<int, double>;

Number LoadNumber(istream& input) {
    using namespace std::literals;

    string parsed_num;

    auto read_char = [&parsed_num, &input] {
        parsed_num += static_cast<char>(input.get());
        if (!input) {
            throw ParsingError("Failed to read number from stream"s);
        }
    };

    auto read_digits = [&input, read_char] {
        if (!isdigit(input.peek())) {
            throw ParsingError("A digit is expected"s);
        }
        while (isdigit(input.peek())) {
            read_char();
        }
    };

    if (input.peek() == '-') {
        read_char();
    }

    if (input.peek() == '0') {
        read_char();
    } else {
        read_digits();
    }

    bool is_int = true;
    if (input.peek() == '.') {
        read_char();
        read_digits();
        is_int = false;
    }

    if (int ch = input.peek(); ch == 'e' || ch == 'E') {
        read_char();
        if (ch = input.peek(); ch == '+' || ch == '-') {
            read_char();
        }
        read_digits();
        is_int = false;
    }

    try {
        if (is_int) {
            try {
                return stoi(parsed_num);
            } catch (...) {}
        }
        return stod(parsed_num);
    } catch (...) {
        throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
    }
}

string LoadString(istream& input) {
    using namespace std::literals;

    // Считываем открывающую кавычку
    char quote;
    input >> quote;
    if (quote != '"') {
        throw ParsingError("String should start with quote");
    }

    auto it = istreambuf_iterator<char>(input);
    auto end = istreambuf_iterator<char>();
    string s;
    while (true) {
        if (it == end) {
            throw ParsingError("String parsing error");
        }
        const char ch = *it;
        if (ch == '"') {
            ++it;
            break;
        } else if (ch == '\\') {
            ++it;
            if (it == end) {
                throw ParsingError("String parsing error");
            }
            const char escaped_char = *(it);
            switch (escaped_char) {
            case 'n': s.push_back('\n'); break;
            case 't': s.push_back('\t'); break;
            case 'r': s.push_back('\r'); break;
            case '"': s.push_back('"'); break;
            case '\\': s.push_back('\\'); break;
            default: throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
            }
        } else if (ch == '\n' || ch == '\r') {
            throw ParsingError("Unexpected end of line"s);
        } else {
            s.push_back(ch);
        }
        ++it;
    }

    return s;
}

Node LoadNull(istream& input) {
    string word;
    for (int i = 0; i < 4; ++i) {
        word += static_cast<char>(input.get());
    }
    if (word != "null") {
        throw ParsingError("Invalid null value: " + word);
    }
    return Node(nullptr);
}

Node LoadBool(istream& input) {
    string word;
    char c = static_cast<char>(input.get());
    word += c;

    if (c == 't') {
        for (int i = 0; i < 3; ++i) {
            c = static_cast<char>(input.get());
            word += c;
        }
        if (word != "true") {
            throw ParsingError("Invalid boolean value: " + word);
        }
        return Node(true);
    } else {
        for (int i = 0; i < 4; ++i) {
            c = static_cast<char>(input.get());
            word += c;
        }
        if (word != "false") {
            throw ParsingError("Invalid boolean value: " + word);
        }
        return Node(false);
    }
}

void SkipWhitespace(istream& input) {
    while (isspace(input.peek())) {
        input.get();
    }
}

Node LoadArray(istream& input) {
    Array result;
    char c;
    input >> c; // read '['

    SkipWhitespace(input);
    if (input.peek() == ']') {
        input.get();
        return Node(move(result));
    }

    while (true) {
        result.push_back(LoadNode(input));
        SkipWhitespace(input);
        input >> c;
        if (c == ']') {
            break;
        } else if (c != ',') {
            throw ParsingError("Expected ',' or ']' in array");
        }
        SkipWhitespace(input);
    }

    return Node(move(result));
}

Node LoadDict(istream& input) {
    Dict result;
    char c;
    input >> c; // read '{'

    SkipWhitespace(input);
    if (input.peek() == '}') {
        input.get();
        return Node(move(result));
    }

    while (true) {
        if (input.peek() != '"') {
            throw ParsingError("Dictionary key must be string");
        }
        string key = LoadString(input); // Теперь LoadString сама читает кавычки

        SkipWhitespace(input);
        input >> c;
        if (c != ':') {
            throw ParsingError("Expected ':' after dictionary key");
        }

        result[move(key)] = LoadNode(input);

        SkipWhitespace(input);
        input >> c;
        if (c == '}') {
            break;
        } else if (c != ',') {
            throw ParsingError("Expected ',' or '}' in dictionary");
        }
        SkipWhitespace(input);
    }

    return Node(move(result));
}

Node LoadNode(istream& input) {
    SkipWhitespace(input);
    char c = static_cast<char>(input.peek());

    if (c == '[') {
        return LoadArray(input);
    } else if (c == '{') {
        return LoadDict(input);
    } else if (c == '"') {
        return Node(LoadString(input));
    } else if (c == 'n') {
        auto node = LoadNull(input);
        // Проверяем, что после ключевого слова идет разделитель
        SkipWhitespace(input);
        if (isalnum(input.peek())) {
            throw ParsingError("Invalid value after null");
        }
        return node;
    } else if (c == 't' || c == 'f') {
        auto node = LoadBool(input);
        // Проверяем, что после ключевого слова идет разделитель
        SkipWhitespace(input);
        if (isalnum(input.peek())) {
            throw ParsingError("Invalid value after boolean");
        }
        return node;
    } else if (isdigit(c) || c == '-') {
        auto number = LoadNumber(input);
        if (holds_alternative<int>(number)) {
            return Node(get<int>(number));
        } else {
            return Node(get<double>(number));
        }
    } else {
        throw ParsingError("Unexpected character: " + string(1, c));
    }
}

// Print functions
struct PrintContext {
    ostream& out;
    int indent_step = 4;
    int indent = 0;

    void PrintIndent() const {
        for (int i = 0; i < indent; ++i) {
            out.put(' ');
        }
    }

    PrintContext Indented() const {
        return {out, indent_step, indent + indent_step};
    }
};

template <typename Value>
void PrintValue(const Value& value, const PrintContext& ctx) {
    ctx.out << value;
}

void PrintValue(nullptr_t, const PrintContext& ctx) {
    ctx.out << "null";
}

void PrintValue(bool value, const PrintContext& ctx) {
    ctx.out << (value ? "true" : "false");
}

void PrintValue(const string& value, const PrintContext& ctx) {
    ctx.out << '"';
    for (char c : value) {
        switch (c) {
        case '\n': ctx.out << "\\n"; break;
        case '\r': ctx.out << "\\r"; break;
        case '\t': ctx.out << "\\t"; break;
        case '"': ctx.out << "\\\""; break;
        case '\\': ctx.out << "\\\\"; break;
        default: ctx.out << c;
        }
    }
    ctx.out << '"';
}

void PrintNode(const Node& node, const PrintContext& ctx) {
    visit([&ctx](const auto& value) {
        PrintValue(value, ctx);
    }, node.GetValue());
}

void PrintValue(const Array& array, const PrintContext& ctx) {
    ctx.out << "[\n";
    bool first = true;
    PrintContext indented_ctx = ctx.Indented();
    for (const auto& item : array) {
        if (!first) {
            ctx.out << ",\n";
        }
        first = false;
        indented_ctx.PrintIndent();
        PrintNode(item, indented_ctx);
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << ']';
}

void PrintValue(const Dict& dict, const PrintContext& ctx) {
    ctx.out << "{\n";
    bool first = true;
    PrintContext indented_ctx = ctx.Indented();
    for (const auto& [key, value] : dict) {
        if (!first) {
            ctx.out << ",\n";
        }
        first = false;
        indented_ctx.PrintIndent();
        PrintValue(key, indented_ctx);
        ctx.out << ": ";
        PrintNode(value, indented_ctx);
    }
    ctx.out << '\n';
    ctx.PrintIndent();
    ctx.out << '}';
}


}  // namespace

Document Load(istream& input) {
    return Document{LoadNode(input)};
}

void Print(const Document& doc, ostream& output) {
    PrintContext ctx{output};
    PrintNode(doc.GetRoot(), ctx);
}

}  // namespace json
