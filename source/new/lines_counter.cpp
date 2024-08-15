#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

unsigned int countLinesInFile(const std::filesystem::path& pathToFile) {
    std::ifstream file(pathToFile);
    auto result_count = std::count(std::istreambuf_iterator<char>(file),
				   std::istreambuf_iterator<char>(), '\n');
    if (result_count > 100) std::cout << pathToFile.filename() << " lienes in file: [ " << result_count << " ] " << std::endl;

    return result_count;
}

unsigned int countLinesInProject(const std::filesystem::path& pathToDir) {
    unsigned int lineCount = 0;
    for (const auto& entry : std::filesystem::recursive_directory_iterator(pathToDir)) {

	if (entry.is_regular_file() && entry.path().extension() == ".cpp") {
	    lineCount += countLinesInFile(entry.path());
	}
    }
    return lineCount;
}

int main() {
    std::filesystem::path currentPath = R"(C:\Users\rob22\CLionProjects\cw_os\new)";
    int res = countLinesInProject(currentPath);
    std::cout << '\n';
    std::cout << "Total number of lines in C++ files: " << res << std::endl;
    return 0;
}