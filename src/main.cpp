
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

using namespace std;

// Structure to hold individual log data
struct LogEntry {
    string level;
    string timestamp;
    string message;
};

class LogAnalyzer {
private:
    vector<LogEntry> logs;
    unordered_map<string, int> levelCount;
    unordered_map<string, int> messageCount;

    LogEntry parseLine(const string &line) {
    LogEntry entry;

    // 1. Find the level [ERROR]
    size_t openBracket = line.find('[');
    size_t closeBracket = line.find(']');
    if (openBracket == string::npos || closeBracket == string::npos) {
        entry.level = "UNKNOWN";
        entry.message = line;
        return entry;
    }
    entry.level = line.substr(openBracket + 1, closeBracket - openBracket - 1);

    // 2. The Timestamp starts after the close bracket
    // We look for the first digit of the date
    size_t timeStart = line.find_first_of("0123456789", closeBracket);
    if (timeStart != string::npos && timeStart + 19 <= line.size()) {
        entry.timestamp = line.substr(timeStart, 19);

        // 3. The Message starts AFTER the timestamp
        // Instead of +20, we find the first character that IS NOT a space or digit 
        // after the time ends
        size_t msgStart = line.find_first_not_of(" ", timeStart + 19);
        
        if (msgStart != string::npos) {
            string rawMsg = line.substr(msgStart);
            
            // CLEANUP: Remove hidden \r or spaces at the end of the line
            size_t last = rawMsg.find_last_not_of(" \r\n\t");
            if (last != string::npos) {
                entry.message = rawMsg.substr(0, last + 1);
            } else {
                entry.message = rawMsg;
            }
        }
    }
    return entry;
}

public:
    // Loads and parses the file, updating statistics in real-time
    bool loadFile(const string &filename) {
        ifstream file(filename);
        if (!file.is_open()) return false;

        logs.reserve(1000); // Pre-allocate memory for performance
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;

            LogEntry entry = parseLine(line);
            logs.push_back(entry);
            levelCount[entry.level]++; // $O(1)$ update during read
            messageCount[entry.message]++;
        }
        file.close();
        return true;
    }

    // Displays logs with optional level filtering and color coding
    void displayLogs(string filter = "") {
        cout << "\n--- Log Detailed View ---\n";
        for (const auto &entry : logs) {
            if (!filter.empty() && entry.level != filter) continue;

            // ANSI Color Coding
            string color = "\033[0m"; // Default
            if (entry.level == "ERROR") color = "\033[1;31m";      // Red
            else if (entry.level == "WARNING") color = "\033[1;33m"; // Yellow
            else if (entry.level == "INFO") color = "\033[1;32m";    // Green

            // cout<<"Level" << color << left << setw(10) << "[" + entry.level + "]" 
            //<< "\033[0m " <<endl;
            cout << "Level: " << color << "[" << entry.level << "]" << "\033[0m\n";
                cout << "Time: " << entry.timestamp << endl;
                cout << "Message: " << entry.message << endl;
                cout << "------------------------" << endl;

        }
    }

    // Prints a high-level summary of the log file
    void printStats() {
        cout << "\n==============================\n";
        cout << "      LOG LEVEL SUMMARY       \n";
        cout << "==============================\n";
        for (const auto &pair : levelCount) {
            cout << left << setw(10) << pair.first << ": " << pair.second << endl;
        }
        cout << "------------------------------\n";
        cout << "Total Logs Processed: " << logs.size() << endl;
        cout << "==============================\n";
    }

    vector<pair<string, int>> getSortedMessages() {
    vector<pair<string, int>> vec(messageCount.begin(), messageCount.end());

    sort(vec.begin(), vec.end(), [](auto &a, auto &b) {
        return a.second > b.second; // descending
    });

    return vec;
}

void printTopKMessages(int k) {
    auto sorted = getSortedMessages();

    cout << "\nTop " << k << " Frequent Messages:\n";

    for (int i = 0; i < k && i < sorted.size(); i++) {
        cout << i + 1 << ". "
            << sorted[i].first
            << " → "
            << sorted[i].second
            << " times\n";
    }
}

void searchLogs(string keyword) {
    cout << "\n--- Search Results for: \"" << keyword << "\" ---\n";
    cout << "------------------------------------------------\n";
    
    bool found = false;
    int matchCount = 0;

    for (const auto &entry : logs) {
        // .find() looks for the keyword anywhere inside the message string
        if (entry.message.find(keyword) != string::npos) {
            
            // Re-using your color logic for the search results
            string color = "\033[0m";
            if (entry.level == "ERROR") color = "\033[1;31m";
            else if (entry.level == "WARNING") color = "\033[1;33m";
            else if (entry.level == "INFO") color = "\033[1;32m";

            cout << color << "[" << entry.level << "]\033[0m " 
                << entry.timestamp << " | " << entry.message << endl;
            
            found = true;
            matchCount++;
        }
    }

    if (!found) {
        cout << "No logs found containing the keyword: " << keyword << endl;
    } else {
        cout << "------------------------------------------------\n";
        cout << "Total matches found: " << matchCount << endl;
    }
}
};

int main() {
    LogAnalyzer analyzer;

    cout << "Initializing Advanced Log Analyzer..." << endl;

    if (!analyzer.loadFile("data/logs.txt")) {
        cerr << "Error: Could not open log file. Ensure 'data/logs.txt' exists." << endl;
        return 1;
    }

    // Example: Only display ERRORS
    //analyzer.displayLogs();
    //analyzer.displayLogs("ERROR");

    // Display total statistics
    analyzer.printStats();
    analyzer.printTopKMessages(3);

    // 2. Ask user for a search term
    string searchTerm;
    cout << "\nEnter a keyword to search in logs (or 'exit' to quit): ";
    getline(cin, searchTerm);

    if (searchTerm != "exit" && !searchTerm.empty()) {
        analyzer.searchLogs(searchTerm);
    }

    return 0;
}