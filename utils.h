#ifndef FILELOADER_H
#define FILELOADER_H

#include <string>
#include <vector>

namespace Utils {
void split(const std::string& s, const std::string& delim, std::vector<std::string>& parts);

auto loadXJX(const std::string& filename) -> bool;
}

#endif // FILELOADER_H
