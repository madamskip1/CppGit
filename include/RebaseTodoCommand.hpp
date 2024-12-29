#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

namespace CppGit {
enum class RebaseTodoCommandType
{
    PICK,
    REWORD,
    EDIT,
    SQUASH,
    FIXUP,
    EXEC,
    BREAK,
    DROP,
    LABEL,
    RESET,
    MERGE,
    UPDATE_REF
};

class RebaseTodoCommandTypeWrapper
{
public:
    RebaseTodoCommandType type;

    explicit(false) RebaseTodoCommandTypeWrapper(const RebaseTodoCommandType type)
        : type{ type }
    {
    }

    explicit(false) operator RebaseTodoCommandType() const { return type; }

    friend auto operator==(const RebaseTodoCommandTypeWrapper& lhs, const RebaseTodoCommandTypeWrapper& rhs) -> bool = default;
    friend auto operator==(const RebaseTodoCommandTypeWrapper& lhs, const RebaseTodoCommandType& rhs) -> bool
    {
        return lhs.type == rhs;
    }

    auto toStringFull() const -> std::string
    {
        switch (type)
        {
        case RebaseTodoCommandType::PICK:
            return "pick";
        case RebaseTodoCommandType::REWORD:
            return "reword";
        case RebaseTodoCommandType::EDIT:
            return "edit";
        case RebaseTodoCommandType::SQUASH:
            return "squash";
        case RebaseTodoCommandType::FIXUP:
            return "fixup";
        case RebaseTodoCommandType::EXEC:
            return "exec";
        case RebaseTodoCommandType::BREAK:
            return "break";
        case RebaseTodoCommandType::DROP:
            return "drop";
        case RebaseTodoCommandType::LABEL:
            return "label";
        case RebaseTodoCommandType::RESET:
            return "reset";
        case RebaseTodoCommandType::MERGE:
            return "merge";
        case RebaseTodoCommandType::UPDATE_REF:
            return "update_ref";
        default:
            throw std::invalid_argument("Unknown rebase todo command");
        }
    }

    auto toStringShort() const -> std::string
    {
        switch (type)
        {
        case RebaseTodoCommandType::PICK:
            return "p";
        case RebaseTodoCommandType::REWORD:
            return "r";
        case RebaseTodoCommandType::EDIT:
            return "e";
        case RebaseTodoCommandType::SQUASH:
            return "s";
        case RebaseTodoCommandType::FIXUP:
            return "f";
        case RebaseTodoCommandType::EXEC:
            return "x";
        case RebaseTodoCommandType::BREAK:
            return "b";
        case RebaseTodoCommandType::DROP:
            return "d";
        case RebaseTodoCommandType::LABEL:
            return "l";
        case RebaseTodoCommandType::RESET:
            return "t";
        case RebaseTodoCommandType::MERGE:
            return "m";
        case RebaseTodoCommandType::UPDATE_REF:
            return "u";
        default:
            throw std::invalid_argument("Unknown rebase todo command");
        }
    }

    static auto fromString(const std::string_view command) -> RebaseTodoCommandTypeWrapper
    {
        if (command == "pick" || command == "p")
        {
            return RebaseTodoCommandType::PICK;
        }
        if (command == "reword" || command == "r")
        {
            return RebaseTodoCommandType::REWORD;
        }
        if (command == "edit" || command == "e")
        {
            return RebaseTodoCommandType::EDIT;
        }
        if (command == "squash" || command == "s")
        {
            return RebaseTodoCommandType::SQUASH;
        }
        if (command == "fixup" || command == "f")
        {
            return RebaseTodoCommandType::FIXUP;
        }
        if (command == "exec" || command == "x")
        {
            return RebaseTodoCommandType::EXEC;
        }
        if (command == "break" || command == "b")
        {
            return RebaseTodoCommandType::BREAK;
        }
        if (command == "drop" || command == "d")
        {
            return RebaseTodoCommandType::DROP;
        }
        if (command == "label" || command == "l")
        {
            return RebaseTodoCommandType::LABEL;
        }
        if (command == "reset" || command == "t")
        {
            return RebaseTodoCommandType::RESET;
        }
        if (command == "merge" || command == "m")
        {
            return RebaseTodoCommandType::MERGE;
        }
        if (command == "update_ref" || command == "u")
        {
            return RebaseTodoCommandType::UPDATE_REF;
        }

        throw std::invalid_argument("Unknown rebase todo command: " + std::string(command));
    }
};

class RebaseTodoCommand
{
public:
    RebaseTodoCommandTypeWrapper type;
    std::string hash;
    std::string message;

    RebaseTodoCommand(const RebaseTodoCommandType type, const std::string& hash, const std::string& message)
        : type{ type },
          hash{ hash },
          message{ message }
    {
    }

    explicit(false) RebaseTodoCommand(const RebaseTodoCommandType type)
        : type{ type }
    {
    }

    auto operator==(const RebaseTodoCommand& rhs) const -> bool = default;

    auto toString() const -> std::string
    {
        return type.toStringFull() + " " + hash + " " + message;
    }
};

} // namespace CppGit
