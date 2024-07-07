#include <string_view>
#include <vector>
#include "Commit.hpp"
namespace CppGit
{
    class CommitParser
    {
    public:
        static constexpr const char*const COMMIT_LOG_DEFAULT_FORMAT = "%H;%P;%an;%ae;%at;%cn;%ce;%ct;%s;%b";
        static constexpr char COMMIT_LOG_DEFAULT_DELIMITER = ';';

        static Commit parseCommit(std::string_view commitLog);
        static Commit parseCommit(std::string_view commitLog, std::string_view format, const char delimiter);

    private:
        static bool isHashToken(std::string_view token);
        static bool isParentsToken(std::string_view token);
        static bool isAuthorNameToken(std::string_view token);
        static bool isAuthorEmailToken(std::string_view token);
        static bool isAuthorDateToken(std::string_view token);
        static bool isCommitterNameToken(std::string_view token);
        static bool isCommitterEmailToken(std::string_view token);
        static bool isCommitterDateToken(std::string_view token);
        static bool isMessageToken(std::string_view token);
        static bool isDescriptionToken(std::string_view token);
        static bool isTreeHashToken(std::string_view token);

        static std::vector<std::string_view> split(std::string_view str, char delimiter);
    };
}
