

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <iomanip>
#include <algorithm>

using namespace std;

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
            entry.hour = stoi(entry.timestamp.substr(11, 2));

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
        int hour = stoi(timestamp.substr(11, 2));
        int minute = stoi(timestamp.substr(14, 2));
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
};

int main(int argc, char* argv[]) {
    LogAnalyzer analyzer;

    if (!analyzer.loadFile("data/logs.txt")) {
        cerr << "FATAL ERROR: Execution suspended. System unable to initialize log pipeline data source targets.\n";
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
        cout << "\n💡 Pro-Tip: Run with flag '--help' to explore runtime CLI filters.\n";
    } 
    else {
        string command = argv[1];
        if (command == "--help") {
            cout << "Available Command Engine Switches:\n"
                << "  " << argv[0] << "                   Run full diagnostic report profile\n"
                << "  " << argv[0] << " --level [TYPE]    Filter log display by level (INFO/ERROR/WARNING)\n"
                << "  " << argv[0] << " --search [TERM]   Search for specific string fragments deep within records\n"
                << "  " << argv[0] << " --top [K]         Extract top K highest recurring messages\n"
                << "  " << argv[0] << " --burst [W] [T]   Analyze burst anomaly window (W=minutes, T=error threshold)\n";
        } 
        else if (command == "--level" && argc > 2) {
            analyzer.displayLogs(argv[2]);
        } 
        else if (command == "--search" && argc > 2) {
            analyzer.searchLogs(argv[2]);
        } 
        else if (command == "--top" && argc > 2) {
            analyzer.printTopKMessages(stoi(argv[2]));
        } 
        else if (command == "--burst" && argc > 3) {
            analyzer.detectBurstErrors(stoi(argv[2]), stoi(argv[3]));
        } 
        else {
            cout << "Invalid argument command routing setup. Execute with '--help' flag.\n";
        }
    }
    return 0;
}