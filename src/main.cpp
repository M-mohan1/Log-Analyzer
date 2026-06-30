#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

using namespace std;

// Safe int parse: returns fallback instead of crashing on bad input
int safeStoi(const string &s, int fallback = -1) {
    try {
        size_t pos;
        int val = stoi(s, &pos);
        return val;
    } catch (...) {
        return fallback;
    }
}

struct LogEntry {
    string level;
    string timestamp;
    string message;
    int hour; // Cached for O(1) performance lookup later
};

class LogAnalyzer {
private:
    vector<LogEntry> logs;
    unordered_map<string, int> levelCount;
    unordered_map<string, int> messageCount;
    unordered_map<string, int> patternCount;
    unordered_map<int, int> hourCount;
    unordered_map<int, int> errorPerHour;
    vector<int> errorMinutes; // Flattened raw minutes for lightning-fast burst check

    LogEntry parseLine(const string &line) {
        LogEntry entry;
        entry.hour = -1;

        size_t openBracket = line.find('[');
        size_t closeBracket = line.find(']');
        if (openBracket == string::npos || closeBracket == string::npos || closeBracket <= openBracket) {
            entry.level = "UNKNOWN";
            entry.message = line;
            return entry;
        }
        entry.level = line.substr(openBracket + 1, closeBracket - openBracket - 1);

        size_t timeStart = line.find_first_of("0123456789", closeBracket);
        if (timeStart != string::npos && timeStart + 19 <= line.size()) {
            entry.timestamp = line.substr(timeStart, 19);
            
            // Fast inline extraction of hour to eliminate double parsing functions
            entry.hour = safeStoi(entry.timestamp.substr(11, 2), -1);
            if (entry.hour < 0 || entry.hour > 23) entry.hour = -1; // reject malformed hour

            size_t msgStart = line.find_first_not_of(" ", timeStart + 19);
            if (msgStart != string::npos) {
                string rawMsg = line.substr(msgStart);
                size_t last = rawMsg.find_last_not_of(" \r\n\t");
                entry.message = (last != string::npos) ? rawMsg.substr(0, last + 1) : rawMsg;
            }
        }
        return entry;
    }

    string extractPattern(const string &msg) {
        // High Performance: Inline stack check without mutating or allocating large strings
        string lower = msg;
        transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

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

    int convertToMinutes(const string &timestamp) {
        int hour = safeStoi(timestamp.substr(11, 2), 0);
        int minute = safeStoi(timestamp.substr(14, 2), 0);
        return hour * 60 + minute;
    }


public:
    bool loadFile(const string &filename) {
        ifstream file(filename);
        if (!file.is_open()) return false;

        logs.reserve(5000); // Increased allocation footprint
        string line;
        while (getline(file, line)) {
            if (line.empty()) continue;

            LogEntry entry = parseLine(line);
            logs.push_back(entry);
            
            levelCount[entry.level]++;
            messageCount[entry.message]++;
            patternCount[extractPattern(entry.message)]++;
            
            if (entry.hour != -1) {
                hourCount[entry.hour]++;
                if (entry.level == "ERROR") {
                    errorPerHour[entry.hour]++;
                    errorMinutes.push_back(convertToMinutes(entry.timestamp));
                }
            }
        }
        file.close();
        return true;
    }

    void displayLogs(string filter = "") {
        cout << "\n--- Log Detailed View ---\n";
        for (const auto &entry : logs) {
            if (!filter.empty() && entry.level != filter) continue;

            string color = "\033[0m";
            if (entry.level == "ERROR") color = "\033[1;31m";
            else if (entry.level == "WARNING") color = "\033[1;33m";
            else if (entry.level == "INFO") color = "\033[1;32m";

            cout << color << left << setw(10) << "[" + entry.level + "]" << "\033[0m "
                << entry.timestamp << " | " << entry.message << "\n";
        }
    }

    void printStats() {
        cout << "\n========================================\n";
        cout << "           SYSTEM METRICS DASHBOARD     \n";
        cout << "========================================\n";
        for (const auto &pair : levelCount) {
            cout << left << setw(12) << pair.first << ": " << pair.second << " occurrences\n";
        }
        cout << "----------------------------------------\n";
        cout << "Total Data Segments Handled: " << logs.size() << "\n";
        cout << "========================================\n";
    }

    void printTopKMessages(int k) {
        vector<pair<string, int>> sorted(messageCount.begin(), messageCount.end());
        sort(sorted.begin(), sorted.end(), [](const auto &a, const auto &b) {
            return a.second > b.second;
        });

        cout << "\n🔥 Top " << k << " Most Frequent Anomalies/Messages:\n";
        for (int i = 0; i < k && i < sorted.size(); i++) {
            cout << "  [" << i + 1 << "] " << left << setw(55) << sorted[i].first << " → " << sorted[i].second << " times\n";
        }
    }

    void detectBurstErrors(int windowSize, int threshold) {
        cout << "\n⚡ [Real-time Analytics] Burst Error Detection (Window: " << windowSize << "m):\n";
        sort(errorMinutes.begin(), errorMinutes.end());

        int left = 0;
        int lastAlertTime = -1; // Prevent duplication spams
        bool alertTriggered = false;

        for (int right = 0; right < errorMinutes.size(); right++) {
            while (errorMinutes[right] - errorMinutes[left] > windowSize) {
                left++;
            }
            int count = right - left + 1;

            if (count >= threshold && errorMinutes[right] != lastAlertTime) {
                cout << "  🚨 ALERT TRIGGER: Spike observed! " << count << " errors tracked within " << windowSize << " minute interval.\n";
                lastAlertTime = errorMinutes[right];
                alertTriggered = true;
            }
        }
        if (!alertTriggered) cout << "  System stability normal. No multi-error bursts tracked.\n";
    }

    void searchLogs(string keyword) {
        cout << "\n🔍 Deep Scan Query for keyword: \"" << keyword << "\"\n";
        int matchCount = 0;
        for (const auto &entry : logs) {
            if (entry.message.find(keyword) != string::npos) {
                string color = (entry.level == "ERROR") ? "\033[1;31m" : (entry.level == "WARNING" ? "\033[1;33m" : "\033[1;32m");
                cout << "  " << color << "[" << entry.level << "]\033[0m " << entry.timestamp << " | " << entry.message << "\n";
                matchCount++;
            }
        }
        cout << "✨ Query executed. Matches found: " << matchCount << "\n";
    }

    void printPatternStats() {
        cout << "\n📋 Pattern Root-Cause Distribution Metrics:\n";
        for (auto &p : patternCount) {
            cout << "  " << left << setw(20) << p.first << " → " << p.second << " hits\n";
        }
    }

    void printTimeAnalysis() {
        cout << "\n⏱️ Traffic Frequency Breakdown by Hour:\n";
        for (auto &p : hourCount) {
            cout << "  " << setfill('0') << setw(2) << p.first << ":00 → " << setfill(' ') << setw(4) << p.second << " transactions tracked\n";
        }
    }

    void printPeakMetrics() {
        int maxLogHour = -1, maxLogs = 0;
        for (auto &p : hourCount) {
            if (p.second > maxLogs) { maxLogs = p.second; maxLogHour = p.first; }
        }
        int maxErrHour = -1, maxErrors = 0;
        for (auto &p : errorPerHour) {
            if (p.second > maxErrors) { maxErrors = p.second; maxErrHour = p.first; }
        }

        cout << "\n📈 System Volatility Peaks:\n";
        if (maxLogHour != -1) cout << "  • Traffic Peak Hour : " << setfill('0') << setw(2) << maxLogHour << ":00 (" << maxLogs << " log rows generated)\n";
        if (maxErrHour != -1) cout << "  • Crash Peak Hour   : " << setfill('0') << setw(2) << maxErrHour << ":00 (" << maxErrors << " ERROR states tracked)\n";
    }

    void exportToCSV(const string &outputFilename = "report.csv") {
    ofstream csvFile(outputFilename);
    if (!csvFile.is_open()) {
        cerr << "Error: Unable to generate CSV report target.\n";
        return;
    }

    // 1. Write Header Row
    csvFile << "Metric Type,Key/Identifier,Value/Count\n";

    // 2. Export Log Level Distribution
    for (const auto &pair : levelCount) {
        csvFile << "Log Level," << pair.first << "," << pair.second << "\n";
    }

    // 3. Export Pattern Roots
    for (const auto &p : patternCount) {
        csvFile << "Root Cause Pattern," << p.first << "," << p.second << "\n";
    }

    // 4. Export Peak Volatility Times
    int maxLogHour = -1, maxLogs = 0;
    for (const auto &p : hourCount) {
        if (p.second > maxLogs) { maxLogs = p.second; maxLogHour = p.first; }
    }
    if (maxLogHour != -1) {
        csvFile << "System Peak,Traffic Peak Hour," << maxLogHour << ":00\n";
    }

    csvFile.close();
    cout << "📊 System report exported successfully to " << outputFilename << "\n";
}
};

int main(int argc, char* argv[]) {
    LogAnalyzer analyzer;

    // Default file path, overridable with --file <path>
    string logFilePath = "data/logs.txt";
    for (int i = 1; i < argc - 1; i++) {
        if (string(argv[i]) == "--file") {
            logFilePath = argv[i + 1];
            break;
        }
    }

    if (!analyzer.loadFile(logFilePath)) {
        cerr << "FATAL ERROR: Could not open log file '" << logFilePath
            << "'. Use --file <path> to specify a different location.\n";
        return 1;
    }

    // Dynamic Production-grade CLI controller routing via argv switches
    if (argc == 1) {
        // Default system diagnostic summary profile run
        cout << "====================================================================\n";
        cout << "                  REAL-TIME DISTRIBUTED SYSTEM REPORT               \n";
        cout << "====================================================================\n";
        analyzer.printStats();
        analyzer.printPatternStats();
        analyzer.printPeakMetrics();
        analyzer.detectBurstErrors(5, 3);

        analyzer.exportToCSV("report.csv");

        cout << "\n💡 Pro-Tip: Run with flag '--help' to explore runtime CLI filters.\n";
    } 
    else {
        string command = argv[1];
        if (command == "--help") {
            cout << "Available Command Engine Switches:\n"
                << "  " << argv[0] << "                   Run full diagnostic report profile\n"
                << "  " << argv[0] << " --file [PATH]     Specify a log file path (default: data/logs.txt)\n"
                << "  " << argv[0] << " --level [TYPE]    Filter log display by level (INFO/ERROR/WARNING)\n"
                << "  " << argv[0] << " --search [TERM]   Search for specific string fragments deep within records\n"
                << "  " << argv[0] << " --top [K]         Extract top K highest recurring messages\n"
                << "  " << argv[0] << " --burst [W] [T]   Analyze burst anomaly window (W=minutes, T=error threshold)\n";
        } 
        else if (command == "--file") {
            // Already consumed above to load the file; just run default report.
            cout << "====================================================================\n";
            cout << "                  REAL-TIME DISTRIBUTED SYSTEM REPORT               \n";
            cout << "====================================================================\n";
            analyzer.printStats();
            analyzer.printPatternStats();
            analyzer.printPeakMetrics();
            analyzer.detectBurstErrors(5, 3);
            analyzer.exportToCSV("report.csv");
        }
        else if (command == "--level" && argc > 2) {
            analyzer.displayLogs(argv[2]);
        } 
        else if (command == "--search" && argc > 2) {
            analyzer.searchLogs(argv[2]);
        } 
        else if (command == "--top" && argc > 2) {
            int k = safeStoi(argv[2], -1);
            if (k <= 0) {
                cerr << "Error: --top requires a positive integer (got '" << argv[2] << "').\n";
                return 1;
            }
            analyzer.printTopKMessages(k);
        } 
        else if (command == "--burst" && argc > 3) {
            int window = safeStoi(argv[2], -1);
            int threshold = safeStoi(argv[3], -1);
            if (window <= 0 || threshold <= 0) {
                cerr << "Error: --burst requires two positive integers (window, threshold). Got '"
                    << argv[2] << "' and '" << argv[3] << "'.\n";
                return 1;
            }
            analyzer.detectBurstErrors(window, threshold);
        } 
        else {
            cout << "Invalid argument command routing setup. Execute with '--help' flag.\n";
        }
    }
    return 0;
}