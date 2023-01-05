#include "utils.h"

std::vector<std::string> get_path()
{
    const char* path = getenv("PATH");
    std::vector<std::string> search_path;

    if (path)
    {
        std::string curr;
        while (*path)
        {
            char ch = *path;
            if (ch == ':')
            {
                search_path.push_back(curr);
                curr.clear();
            }
            else
                curr += ch;
            path++;
        }
        search_path.push_back(curr);
    }

    for (const auto i : {"/bin/", "/sbin/", "/usr/bin/", "/usr/sbin/"})
    {
        if (std::filesystem::exists(i))
            search_path.push_back(i);
    }

    return search_path;
}
