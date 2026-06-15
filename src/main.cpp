
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
    unordered_map<string, int> patternCount;
    unordered_map<string, int> hourCount;
    unordered_map<int, int> errorPerHour;
    vector<string> errorTimestamps;

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

string extractPattern(const string &msg) {
    string lower;

    // convert to lowercase
    for (char c : msg) {
        lower += tolower(c);
    }

    // pattern rules (simple but powerful)
    if (lower.find("disk") != string::npos) return "disk issue";
    if (lower.find("memory") != string::npos) return "memory issue";
    if (lower.find("cpu") != string::npos) return "cpu issue";
    if (lower.find("database") != string::npos) return "database issue";
    if (lower.find("login") != string::npos) return "login activity";
    if (lower.find("security") != string::npos) return "security event";
    if (lower.find("network") != string::npos) return "network issue";
    if (lower.find("backup") != string::npos) return "backup process";
    if (lower.find("error") != string::npos) return "generic error";

    return "other";
}

string extractHour(const string &timestamp) {
    if (timestamp.size() < 13) return "UNKNOWN";

    return timestamp.substr(11, 2); // HH from YYYY-MM-DD HH:MM:SS
}
int ExtractHour(const string &timestamp) {
    if (timestamp.size() < 13) return -1;

    // HH is at index 11 and 12
    return stoi(timestamp.substr(11, 2));
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
            string pattern = extractPattern(entry.message);
            patternCount[pattern]++;
            string hour = extractHour(entry.timestamp);
            hourCount[hour]++;

            if (entry.level == "ERROR") {
    int hour = ExtractHour(entry.timestamp);
    if (hour != -1) {
        errorPerHour[hour]++;
    }
}

if (entry.level == "ERROR") {
    errorTimestamps.push_back(entry.timestamp);
}
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
int convertToMinutes(const string &timestamp) {
    int hour = stoi(timestamp.substr(11, 2));
    int minute = stoi(timestamp.substr(14, 2));
    return hour * 60 + minute;
}

void detectBurstErrors(int windowSize, int threshold) {
    cout << "\n=== Burst Error Detection ===\n";

    vector<int> times;

    for (auto &t : errorTimestamps) {
        times.push_back(convertToMinutes(t));
    }

    sort(times.begin(), times.end());

    int left = 0;

    for (int right = 0; right < times.size(); right++) {
        while (times[right] - times[left] > windowSize) {
            left++;
        }

        int count = right - left + 1;

        if (count >= threshold) {
            cout << "🚨 ALERT: "
                << count << " errors within "
                << windowSize << " minutes\n";
        }
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
void printPatternStats() {
    cout << "\n===== PATTERN ANALYSIS =====\n";

    for (auto &p : patternCount) {
        cout << p.first << " → " << p.second << endl;
    }
}
void printTimeAnalysis() {
    cout << "\n===== TIME ANALYSIS =====\n";

    for (auto &p : hourCount) {
        cout << p.first << ":00 → " << p.second << " logs\n";
    }
}
void printPeakHour() {
    string peakHour;
    int maxCount = 0;

    for (auto &p : hourCount) {
        if (p.second > maxCount) {
            maxCount = p.second;
            peakHour = p.first;
        }
    }

    cout << "\n🔥 Peak Log Hour: " << peakHour 
        << ":00 with " << maxCount << " logs\n";
}
void printErrorByHour() {
    cout << "\n=== Error Distribution by Hour ===\n";

    for (auto &p : errorPerHour) {
        cout << p.first << ":00 - " << p.second << " errors\n";
    }
}
void printPeakErrorHour() {
    int maxHour = -1;
    int maxCount = 0;

    for (auto &p : errorPerHour) {
        if (p.second > maxCount) {
            maxCount = p.second;
            maxHour = p.first;
        }
    }

    cout << "\n🚨 Peak Error Hour: " 
        << maxHour << ":00 with " 
        << maxCount << " errors\n";
}
void detectAnomalies(int threshold) {
    cout << "\n=== Anomaly Detection ===\n";

    for (auto &p : errorPerHour) {
        if (p.second > threshold) {
            cout << "🚨 ALERT: High error rate at "
                << p.first << ":00 ("
                << p.second << " errors)\n";
        }
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
//     analyzer.printPatternStats();
//     analyzer.printTimeAnalysis();
//     analyzer.printPeakHour();
//     analyzer.printErrorByHour();
// analyzer.printPeakErrorHour();
analyzer.detectAnomalies(2); // try 2 or 3
analyzer.detectBurstErrors(5,1); 
    // 2. Ask user for a search term
    //string searchTerm;
    //cout << "\nEnter a keyword to search in logs (or 'exit' to quit): ";
    //getline(cin, searchTerm);

    //if (searchTerm != "exit" && !searchTerm.empty()) {
     //   analyzer.searchLogs(searchTerm);
   // }

    return 0;
}