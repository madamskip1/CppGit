#pragma once

#include <cstdint>
#include <stdexcept>
#include <string>
#include <string_view>

namespace CppGit {

/// @brief Type of rebase todo command
enum class RebaseTodoCommandType : uint8_t
{
    PICK,      ///< Pick commit
    REWORD,    ///< Stop to edit commit message
    EDIT,      ///< Stop to edit commit
    SQUASH,    ///< Squash commit with previous commit and stop to edit commit message
    FIXUP,     ///< Fixup commit with previous commit
    EXEC,      ///< Execute shell command. NOT IMPLEMENTED
    BREAK,     ///< Stop for a break
    DROP,      ///< Drop commit
    LABEL,     ///< Label commit. NOT IMPLEMENTED
    RESET,     ///< Reset to label(?) NOT IMPLEMENTED
    MERGE,     ///< Merge commit. NOT IMPLEMENTED
    UPDATE_REF ///< Update ref. NOT IMPLEMENTED
};

class RebaseTodoCommandTypeWrapper
{
public:
    RebaseTodoCommandType type; // NOLINT(misc-non-private-member-variables-in-classes)

    /// @param type Type of the command
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

    /// @brief Convert the command type to a string representation
    /// @return The representation of the command type
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
        [[unlikely]] default:
            throw std::invalid_argument("Unknown rebase todo command");
        }
    }

    /// @brief Convert the command type to a short string representation
    /// @return The short representation of the command type
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
        [[unlikely]] default:
            throw std::invalid_argument("Unknown rebase todo command");
        }
    }

    /// @brief Convert a string representation of a command to a RebaseTodoCommandTypeWrapper
    /// @param command The string representation of the command
    /// @return The RebaseTodoCommandTypeWrapper representing the command
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
    // NOLINTBEGIN(misc-non-private-member-variables-in-classes)
    RebaseTodoCommandTypeWrapper type;
    std::string hash;
    std::string message;
    // NOLINTEND(misc-non-private-member-variables-in-classes)

    /// @brief Construct a new RebaseTodoCommand object
    /// @param type Type of the command
    /// @param hash Hash of the commit
    /// @param message Message of the commit
    RebaseTodoCommand(const RebaseTodoCommandType type, const std::string& hash, const std::string& message)
        : type{ type },
          hash{ hash },
          message{ message }
    {
    }

    /// @brief Construct a new RebaseTodoCommand object
    /// @param type Type of the command
    explicit(false) RebaseTodoCommand(const RebaseTodoCommandType type)
        : type{ type }
    {
    }

    auto operator==(const RebaseTodoCommand& rhs) const -> bool = default;

    /// @brief Convert the command to a string representation
    /// @return The representation of the command
    auto toString() const -> std::string
    {
        if (hash.empty())
        {
            return type.toStringFull();
        }

        return type.toStringFull() + " " + hash + " " + message;
    }
};

} // namespace CppGit
