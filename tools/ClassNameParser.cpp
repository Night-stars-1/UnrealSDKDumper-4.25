#include <iostream>
#include <fstream>
#include <string>
#include <vector>

using namespace std;


std::string extractStructName(const std::string& input) {
    size_t pos = input.find("struct");
    if (pos != std::string::npos) { // "class" found in the input string
        pos += 6; // Move to the character right after "class"

        // Find the next colon or newline character from the position of "class"
        size_t colonPos = input.find(":", pos);
        size_t newlinePos = input.find("\n", pos);
        
        if (colonPos != std::string::npos && newlinePos != std::string::npos) {
            // Return the content between "class" and the next colon
            if (colonPos < newlinePos)
                return input.substr(pos, colonPos - pos);
            // Return the content between "class" and the next newline
            else
                return input.substr(pos, newlinePos - pos);
        }
        else if (colonPos != std::string::npos) {
            // Return the content between "class" and the next colon
            return input.substr(pos, colonPos - pos);
        }
        else if (newlinePos != std::string::npos) {
            // Return the content between "class" and the next newline
            return input.substr(pos, newlinePos - pos);
        }
    }

    // If "class" was not found or no content between "class" and the next colon or newline
    return "";
}

std::string extractClassName(const std::string& input) {
    size_t pos = input.find("class");
    if (pos != std::string::npos) { // "class" found in the input string
        pos += 5; // Move to the character right after "class"

        // Find the next colon or newline character from the position of "class"
        size_t colonPos = input.find(":", pos);
        size_t newlinePos = input.find("\n", pos);
        
        if (colonPos != std::string::npos && newlinePos != std::string::npos) {
            // Return the content between "class" and the next colon
            if (colonPos < newlinePos)
                return input.substr(pos, colonPos - pos);
            // Return the content between "class" and the next newline
            else
                return input.substr(pos, newlinePos - pos);
        }
        else if (colonPos != std::string::npos) {
            // Return the content between "class" and the next colon
            return input.substr(pos, colonPos - pos);
        }
        else if (newlinePos != std::string::npos) {
            // Return the content between "class" and the next newline
            return input.substr(pos, newlinePos - pos);
        }
    }

    // If "class" was not found or no content between "class" and the next colon or newline
    return "";
}

std::string trimWhitespace(const std::string& input) {
    size_t start = 0;
    size_t end = input.length() - 1;

    // Find the first non-space character from the beginning of the string
    while (start <= end && std::isspace(input[start])) {
        start++;
    }

    // Find the last non-space character from the end of the string
    while (end >= start && std::isspace(input[end])) {
        end--;
    }

    // Return the trimmed substring
    return input.substr(start, end - start + 1);
}

vector<string> classnames;

ifstream input("input.h");

ofstream output("result.cpp");


int main(){
	if(!input) return -1;
	string linecontent;
	while(getline(input, linecontent)) {
		if(linecontent.find("template") != string::npos) continue;
		if(linecontent.find("friend") != string::npos) continue;
		if(linecontent.find("//") != string::npos) continue;
		if(linecontent.find("(") != string::npos) continue;
		
		string res = extractClassName(linecontent + "\n"); 
		if(res != "") {
			classnames.push_back(trimWhitespace(res));
			cout << "origin: " << linecontent << endl;
			cout << "res: " << res << endl;
		}
		res = extractStructName(linecontent + "\n");
		if(res != "") {
			classnames.push_back(trimWhitespace(res));
			cout << "origin: " << linecontent << endl;
			cout << "res: " << res << endl;
		}
	}
	output << "std::vector<std::string> RefGraphSolver::BasicTypes = {" << endl;
	for(auto name : classnames) {
		cout << "name: " << name << endl;
		output << "\t\"" << name << "\"," << endl; 
	}
	output << "};";
	return 0;
}

