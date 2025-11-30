#pragma once

#include <iostream>
#include <map>
#include <string>
#include <vector>
#include <variant>

namespace json {

    class Node;
    using Dict = std::map<std::string, Node>;
    using Array = std::vector<Node>;

    class ParsingError : public std::runtime_error {
    public:
        using runtime_error::runtime_error;
    };

    class Node {
    public:
        using Value = std::variant<std::nullptr_t, Array, Dict, bool, int, double, std::string>;

        Node() = default;

        Node(std::nullptr_t val) : value_(val) {}
        Node(int val) : value_(val) {}
        Node(double val) : value_(val) {}
        Node(const std::string& val) : value_(val) {}
        Node(Array val) : value_(std::move(val)) {}
        Node(Dict val) : value_(std::move(val)) {}
        Node(bool val) : value_(val) {}

        bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value_); }
        bool IsBool() const { return std::holds_alternative<bool>(value_); }
        bool IsInt() const { return std::holds_alternative<int>(value_); }
        bool IsDouble() const { return std::holds_alternative<int>(value_) || std::holds_alternative<double>(value_); }
        bool IsPureDouble() const { return std::holds_alternative<double>(value_); }
        bool IsString() const { return std::holds_alternative<std::string>(value_); }
        bool IsArray() const { return std::holds_alternative<Array>(value_); }
        bool IsMap() const { return std::holds_alternative<Dict>(value_); }

        int AsInt() const;
        bool AsBool() const;
        double AsDouble() const;
        const std::string& AsString() const;
        const Array& AsArray() const;
        const Dict& AsMap() const;

        bool operator==(const Node& other) const { return value_ == other.value_; }
        bool operator!=(const Node& other) const { return !(*this == other); }

        const Value& GetValue() const {
            return value_;
        }

    private:
        Value value_;
    };

    class Document {
    public:
        explicit Document(Node root);

        const Node& GetRoot() const;

        bool operator==(const Document& other) const {
            return root_ == other.root_;
        }

        bool operator!=(const Document& other) const {
            return !(*this == other);
        }

    private:
        Node root_;
    };

    Document Load(std::istream& input);

    void Print(const Document& doc, std::ostream& output);

    void Print(const Node& node, std::ostream& output);

}  // namespace json