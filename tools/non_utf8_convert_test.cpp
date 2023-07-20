#include <iostream>
#include <string>
#include <fstream>

std::string replaceNonUTF8Characters(const std::string& input) {
    std::string result;
    for (std::size_t i = 0; i < input.size(); ++i) {
        char ch = input[i];
        if ((ch & 0x80) == 0x00) {
            // Single-byte character (ASCII range)
            result += ch;
        } else if ((ch & 0xE0) == 0xC0) {
            // Two-byte character (UTF-8 encoded Chinese character)
            result += ch;
            result += input[++i];
        } else if ((ch & 0xF0) == 0xE0) {
            // Three-byte character (UTF-8 encoded Chinese character)
            result += ch;
            result += input[++i];
            result += input[++i];
        } else {
            // Not a valid UTF-8 character, replace with byte code
            result += "_x" + std::to_string(static_cast<unsigned char>(ch));
        }
    }
    return result;
}

std::string removeNullCharacters(const std::string& input) {
    std::string result;
    for (char ch : input) {
        if (ch != '\0') {
            result += ch;
        }
    }
    return result;
}

int main() {
    std::ifstream inputFile("bad.h");
    if (!inputFile) {
        std::cerr << "Error opening input file 'bad.h'." << std::endl;
        return 1;
    }

    std::ofstream outputFile("good no null.h");
    if (!outputFile) {
        std::cerr << "Error creating output file 'good.h'." << std::endl;
        return 1;
    }

    std::string line;
    while (std::getline(inputFile, line)) {
    	std::string processedLine = removeNullCharacters(line);
        processedLine = replaceNonUTF8Characters(processedLine);
        outputFile << processedLine << std::endl;
    }
    return 0;
}
