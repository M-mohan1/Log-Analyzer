#include <iostream>
#include <fstream>
#include <string>
#include <regex>

using namespace std;

struct LogEntry {
    string level;
    string timestamp;
    string message;
};

LogEntry parseLine(const string &line) {
    LogEntry entry;
    // This regex looks for [LEVEL] followed by a timestamp and then the message
    // It ignores any tags that appear before the level
    regex logRegex(R"(\[(?:source: \d+\]\s*)?\[(\w+)\]\s(\d{4}-\d{2}-\d{2}\s\d{2}:\d{2}:\d{2})\s(.*))");
    smatch match;

    if (regex_search(line, match, logRegex)) {
        entry.level = match[1];     // The (\w+) part
        entry.timestamp = match[2]; // The timestamp part
        entry.message = match[3];   // Everything after
    } else {
        entry.level = "UNKNOWN";
        entry.message = line; // Keep the raw line if parsing fails
    }

    return entry;
}

// #include <iostream>
// #include <fstream>
// #include <string>
// #include <vector>
// #include <unordered_map>
// using namespace std;
// struct LogEntry {
//     string level;
//     string timestamp;
//     string message;
// };

// LogEntry parseLine(const string &line) {
//     LogEntry entry;

//     size_t start = line.find('[');
//     size_t end = line.find(']');

//     // Validate level
//     if (start == string::npos || end == string::npos || end <= start) {
//         entry.level = "UNKNOWN";
//         entry.timestamp = "";
//         entry.message = line;
//         return entry;
//     }

//     entry.level = line.substr(start + 1, end - start - 1);

//     // Validate timestamp position
//     size_t time_start = end + 2;

//     if (time_start + 19 > line.size()) {
//         entry.timestamp = "INVALID";
//         entry.message = "";
//         return entry;
//     }

//     entry.timestamp = line.substr(time_start, 19);

//     // Extract message safely
//     size_t msg_start = time_start + 20;
//     if (msg_start < line.size()) {
//         entry.message = line.substr(msg_start);
//     } else {
//         entry.message = "";
//     }

//     return entry;
// }
// int main() {
//     // File path
//     string filename = "data/logs.txt";

//     // Open file
//     ifstream file(filename);

//     // Check if file opened successfully
//     if (!file.is_open()) {
//         cout << "Error: Could not open file!" << endl;
//         return 1;
//     }

//     cout << "Reading logs...\n" << endl;

//     string line;
//     vector<LogEntry>logs;
//     unordered_map<string, int> levelCount;

// while (getline(file, line)) {
//     if (line.empty()) continue;
//     LogEntry entry = parseLine(line);
//     //push that
//     logs.push_back(entry);
//     levelCount[entry.level]++;
// }
//     for (const auto &entry : logs) {
//     // cout << "Level: " << entry.level << endl;
//     // cout << "Time: " << entry.timestamp << endl;
//     // cout << "Message: " << entry.message << endl;
//     // cout << "------------------------" << endl;
//     string color = "\033[0m"; // Default
//     if (entry.level == "ERROR") color = "\033[1;31m";   // Red
//     else if (entry.level == "WARNING") color = "\033[1;33m"; // Yellow
//     else if (entry.level == "INFO") color = "\033[1;32m";    // Green

//     cout <<"Level:"<< color << "[" << entry.level << "]\033[0m " 
//         << endl;
//         cout << "Time: " << entry.timestamp << endl;
//     cout << "Message: " << entry.message << endl;
//     cout << "------------------------" << endl;
// }

//     // Close file
//     cout << "\nLog Level Summary:\n";

//     for (const auto &pair : levelCount) {
//     cout << pair.first << ": " << pair.second << endl;
// }
//     cout<<"total logs ="<<logs.size()<<"\n";

//     file.close();

//     return 0;
// }
