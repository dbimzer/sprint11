#include "json.h"
#include <sstream>

//using namespace std;

namespace json {

    namespace {

        Node LoadNode(std::istream& input);

        using Number = std::variant<int, double>;

        Number LoadNumber(std::istream& input) {
            using namespace std::literals;

            std::string parsed_num;

            auto read_char = [&parsed_num, &input] {
                parsed_num += static_cast<char>(input.get());
                if (!input) {
                    throw ParsingError("Failed to read number from stream");
                }
                };

            auto read_digits = [&input, read_char] {
                if (!std::isdigit(input.peek())) {
                    throw ParsingError("A digit is expected"s);
                }
                while (std::isdigit(input.peek())) {
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
                        return std::stoi(parsed_num);
                    }
                    catch (...) {
                    }
                }
                return std::stod(parsed_num);
            }
            catch (...) {
                throw ParsingError("Failed to convert "s + parsed_num + " to number"s);
            }
        }

        std::string LoadString(std::istream& input) {
            using namespace std::literals;

            // Считываем открывающую кавычку
            char quote;
            input >> quote;
            if (quote != '"') {
                throw ParsingError("String should start with quote");
            }

            auto it = std::istreambuf_iterator<char>(input);
            auto end = std::istreambuf_iterator<char>();
            std::string s;
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
                    case 'n':
                        s.push_back('\n');
                        break;
                    case 't':
                        s.push_back('\t');
                        break;
                    case 'r':
                        s.push_back('\r');
                        break;
                    case '"':
                        s.push_back('"');
                        break;
                    case '\\':
                        s.push_back('\\');
                        break;
                    default:
                        throw ParsingError("Unrecognized escape sequence \\"s + escaped_char);
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

        void SkipWhitespace(std::istream& input) {
            while (isspace(input.peek())) {
                input.get();
            }
        }

        Node LoadArray(std::istream& input) {
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

        Node LoadDict(std::istream& input) {
            Dict dict;

            char c;
            input >> c;
            SkipWhitespace(input);
            if (input.peek() == '}') {
                input.get();
                return Node(std::move(dict));
            }

            while (true) {
                if (input.peek() != '"') {
                    throw ParsingError("Dictionary key must be string");
                }
                std::string key = LoadString(input); // Теперь LoadString сама читает кавычки

                SkipWhitespace(input);
                input >> c;
                if (c != ':') {
                    throw ParsingError("Expected ':' after dictionary key");
                }

                dict[move(key)] = LoadNode(input);

                SkipWhitespace(input);
                input >> c;
                if (c == '}') {
                    break;
                } else if (c != ',') {
                    throw ParsingError("Expected ',' or '}' in dictionary");
                }
                SkipWhitespace(input);
            }

            return Node(std::move(dict));
        }


        Node LoadNode(std::istream& input) {
            char c;
            input >> c;

            if (c == '[') {
                return LoadArray(input);
            }
            else if (c == '{') {
                input.putback(c);
                return LoadDict(input);
            }
            else if (c == '"') {
                std::string str = LoadString(input);
                return Node(str);
            }
            else {
                input.putback(c);

                try {
                    auto number = LoadNumber(input); // Пытаемся считать число
                    if (std::holds_alternative<int>(number)) {
                        return Node(std::get<int>(number));
                    } else {
                        return Node(std::get<double>(number));
                    }
                }
                catch (...) {
                    // Если число не удалось считать, проверяем на логические значения и 
                    std::string literal;
                    
                    input.clear(); // Сброс состояния ошибки потока
                    
                    for (char ch; input >> ch && ch != ',';) {
                        if (ch == ',') {
                            input.putback(ch);
                            break;
                        }
                        literal.push_back(ch);
                    }

                    if (literal == "true") {
                        return Node(true);
                    } else if (literal == "false") {
                        return Node(false);
                    } else if (literal == "null") {
                        return Node(nullptr);
                    } else {
                        throw ParsingError("Unrecognized token");
                    }
                }
            }
        }

    }  // namespace

    Document::Document(Node root)
        : root_(std::move(root)) {
    }

    const Node& Document::GetRoot() const {
        return root_;
    }

    Document Load(std::istream& input) {
        return Document{ LoadNode(input) };
    }

    void Print(const Document& doc, std::ostream& output) {
        Print(doc.GetRoot(), output);
    }

    void Print(const Node& node, std::ostream& output) {
        if (node.IsArray()) {
            output << "[";
            for (size_t i = 0; i < node.AsArray().size(); ++i) {
                if (i > 0) {
                    output << ", ";
                }
                Print(node.AsArray()[i], output);
            }
            output << "]";
        }
        else if (node.IsMap()) {
            output << "{";
            size_t count = 0;
            for (const auto& [key, value] : node.AsMap()) {
                if (count > 0) {
                    output << ", ";
                }
                output << "\"" << key << "\": ";
                Print(value, output);
                ++count;
            }
            output << "}";
        }
        else if (node.IsInt()) {
            output << node.AsInt();
        }
        else if (node.IsDouble()) {
            if (node.IsPureDouble()) {
                output << node.AsDouble();
            }
            else {
                output << static_cast<int>(node.AsDouble());
            }
        }
        else if (node.IsBool()) {
            output << (node.AsBool() ? "true" : "false");
        }
        else if (node.IsString()) {
            output << "\"";
            const std::string& str = node.AsString();
            for (char ch : str) {
                switch (ch) {
                case '\n': output << "\\n"; break;
                case '\t': output << "\\t"; break;
                case '\r': output << "\\r"; break;
                case '"':  output << "\\\""; break;
                case '\\': output << "\\\\"; break;
                default:  output << ch; break;
                }
            }
            output << "\"";
        }
        else {
            output << "null";
        }
    }

    int Node::AsInt() const
    {
        if (!IsInt()) throw std::logic_error("Not an int");
        return std::get<int>(value_);
    }

    bool Node::AsBool() const
    {
        if (!IsBool()) throw std::logic_error("Not a bool");
        return std::get<bool>(value_);
    }

    double Node::AsDouble() const
    {
        if (!IsDouble()) throw std::logic_error("Not a double");
        if (auto int_val = std::get_if<int>(&value_)) {
            return static_cast<double>(*int_val);
        }
        return std::get<double>(value_);
    }

    const std::string& Node::AsString() const
    {
        if (!IsString()) throw std::logic_error("Not a string");
        return std::get<std::string>(value_);
    }

    const Array& Node::AsArray() const
    {
        if (!IsArray()) throw std::logic_error("Not an array");
        return std::get<Array>(value_);
    }

    const Dict& Node::AsMap() const
    {
        if (!IsMap()) throw std::logic_error("Not a map");
        return std::get<Dict>(value_);
    }
}  // namespace json