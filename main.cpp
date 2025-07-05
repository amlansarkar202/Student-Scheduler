#include <iostream>
#include <vector>
#include <queue>
#include <algorithm>
#include <iomanip>
#include <string>
#include <sstream>
#include <limits>
#include <fstream>

using namespace std;

const int SLOT_STEP = 30;
const int MAX_DAYS = 7;
const int END_OF_DAY = 1380; // 11 PM

struct TimeSlot {
    int start, end;
};

struct Task {
    string name;
    int duration;
    int priority;
    int deadlineDay;
    int preferredStart = 0;
    int preferredEnd = END_OF_DAY;
};

struct TaskCompare {
    bool operator()(const Task& a, const Task& b) const {
        if (a.priority == b.priority)
            return a.deadlineDay > b.deadlineDay;
        return a.priority < b.priority;
    }
};

struct DayStats {
    int productiveMinutes = 0;
    int breakMinutes = 0;
    int totalFreeTime = 0;
};

vector<vector<TimeSlot>> schedule(MAX_DAYS);
priority_queue<Task, vector<Task>, TaskCompare> taskQueue;

void printTime(int minutes) {
    int h = minutes / 60;
    int m = minutes % 60;
    cout << setw(2) << setfill('0') << h << ":" << setw(2) << setfill('0') << m;
}

bool isFreeSlot(const vector<TimeSlot>& occupied, int start, int end) {
    for (const auto& slot : occupied) {
        if (max(slot.start, start) < min(slot.end, end))
            return false;
    }
    return true;
}

DayStats analyzeDay(const vector<TimeSlot>& daySlots) {
    DayStats stats;
    int lastEnd = 0;
    for (const auto& slot : daySlots) {
        int duration = slot.end - slot.start;
        if (duration == SLOT_STEP)
            stats.productiveMinutes += duration;
        else
            stats.breakMinutes += duration;

        if (slot.start > lastEnd)
            stats.totalFreeTime += slot.start - lastEnd;

        lastEnd = max(lastEnd, slot.end);
    }
    stats.totalFreeTime += END_OF_DAY - lastEnd;
    return stats;
}

bool trySchedule(Task task, vector<vector<TimeSlot>>& schedule, int BREAK_AFTER = 90, int BREAK_DURATION = 30) {
    for (int day = 0; day <= task.deadlineDay; ++day) {
        int remaining = task.duration;
        int worked = 0;

        for (int time = task.preferredStart; time + SLOT_STEP <= min(task.preferredEnd, END_OF_DAY) && remaining > 0; time += SLOT_STEP) {
            if (isFreeSlot(schedule[day], time, time + SLOT_STEP)) {
                schedule[day].push_back({time, time + SLOT_STEP});
                cout << "[Day " << day + 1 << "] ";
                printTime(time); cout << " - "; printTime(time + SLOT_STEP);
                cout << " : " << task.name << (remaining < task.duration ? " (split)" : "") << endl;
                remaining -= SLOT_STEP;
                worked += SLOT_STEP;

                if (worked >= BREAK_AFTER && remaining > 0) {
                    int breakStart = time + SLOT_STEP;
                    int breakEnd = breakStart + BREAK_DURATION;
                    if (isFreeSlot(schedule[day], breakStart, breakEnd)) {
                        schedule[day].push_back({breakStart, breakEnd});
                        cout << "[Day " << day + 1 << "] ";
                        printTime(breakStart); cout << " - "; printTime(breakEnd);
                        cout << " : BREAK\n";
                        worked = 0;
                        time += BREAK_DURATION;
                    }
                }
            }
        }
        if (remaining <= 0) return true;
    }
    return false;
}

void safeIntInput(const string& prompt, int& value) {
    cout << prompt;
    while (!(cin >> value)) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << "Invalid input. Try again: ";
    }
}

void addOccupiedSlots() {
    int n;
    safeIntInput("Enter number of pre-occupied time blocks (e.g., classes, meals): ", n);

    for (int i = 0; i < n; ++i) {
        int d, sh, sm, eh, em;
        cout << "\n--- Block " << i + 1 << " ---\n";

        do {
            safeIntInput("Enter day [1-7]: ", d);
            if (d < 1 || d > 7)
                cout << "Day must be between 1 and 7.\n";
        } while (d < 1 || d > 7);

        do {
            safeIntInput("Enter start hour [0-23]: ", sh);
            if (sh < 0 || sh > 23)
                cout << "Start hour must be between 0 and 23.\n";
        } while (sh < 0 || sh > 23);

        do {
            safeIntInput("Enter start minute [0-59]: ", sm);
            if (sm < 0 || sm > 59)
                cout << "Start minute must be between 0 and 59.\n";
        } while (sm < 0 || sm > 59);

        do {
            safeIntInput("Enter end hour [0-23]: ", eh);
            if (eh < 0 || eh > 23)
                cout << "End hour must be between 0 and 23.\n";
        } while (eh < 0 || eh > 23);

        do {
            safeIntInput("Enter end minute [0-59]: ", em);
            if (em < 0 || em > 59)
                cout << "End minute must be between 0 and 59.\n";
        } while (em < 0 || em > 59);

        int startMin = sh * 60 + sm;
        int endMin = eh * 60 + em;

        if (endMin <= startMin) {
            cout << "End time must be after start time. Try again.\n";
            --i; continue;
        }

        schedule[d - 1].push_back({startMin, endMin});
    }
}

void addTasks() {
    int t;
    safeIntInput("\nEnter number of tasks to schedule: ", t);

    for (int i = 0; i < t; ++i) {
        Task task;
        int psh, peh;

        cout << "\n--- Task " << i + 1 << " ---\n";

        cout << "Enter task name: ";
        cin >> ws;
        getline(cin, task.name);

        safeIntInput("Enter duration (in minutes): ", task.duration);

        do {
            safeIntInput("Enter priority [1-10]: ", task.priority);
            if (task.priority < 1 || task.priority > 10)
                cout << "Priority must be between 1 and 10.\n";
        } while (task.priority < 1 || task.priority > 10);

        do {
            safeIntInput("Enter deadline day [1-7]: ", task.deadlineDay);
            if (task.deadlineDay < 1 || task.deadlineDay > 7)
                cout << "Deadline day must be between 1 and 7.\n";
        } while (task.deadlineDay < 1 || task.deadlineDay > 7);

        do {
            safeIntInput("Enter preferred start hour [0-23]: ", psh);
            if (psh < 0 || psh > 23)
                cout << "Start hour must be between 0 and 23.\n";
        } while (psh < 0 || psh > 23);

        do {
            safeIntInput("Enter preferred end hour [1-23]: ", peh);
            if (peh < 1 || peh > 23 || peh <= psh)
                cout << "End hour must be between 1 and 23 and greater than start hour.\n";
        } while (peh > 23 || peh < 1 || peh <= psh);

        task.preferredStart = psh * 60;
        task.preferredEnd = peh * 60;
        task.deadlineDay--;

        taskQueue.push(task);
    }
}

void generateAndPrintSchedule() {
    cout << "\n\n======= GENERATED WEEKLY SCHEDULE =======\n";
    while (!taskQueue.empty()) {
        Task task = taskQueue.top(); taskQueue.pop();
        bool scheduled = trySchedule(task, schedule);
        if (!scheduled && task.deadlineDay < MAX_DAYS - 1) {
            task.deadlineDay++;
            taskQueue.push(task);
        } else if (!scheduled) {
            cout << "\u274C Could not schedule: " << task.name << endl;
        }
    }
}

void showDailySummary() {
    cout << "\n======= DAILY SUMMARY =======\n";
    for (int d = 0; d < MAX_DAYS; ++d) {
        DayStats stats = analyzeDay(schedule[d]);
        cout << "\n[Day " << d + 1 << "]\n";
        cout << "Productive Time: " << stats.productiveMinutes << " mins\n";
        cout << "Break Time:      " << stats.breakMinutes << " mins\n";
        cout << "Free Time Left:  " << stats.totalFreeTime << " mins\n";
    }
}

// ✅ Save schedule to file
void saveScheduleToFile(const string& filename = "schedule.txt") {
    ofstream out(filename);
    if (!out) {
        cout << "Error opening file for writing.\n";
        return;
    }

    for (int d = 0; d < MAX_DAYS; ++d) {
        out << "Day " << d + 1 << ":\n";
        for (const auto& slot : schedule[d]) {
            out << slot.start << " " << slot.end << "\n";
        }
        out << "-1 -1\n";
    }

    out.close();
    cout << "\n✅ Schedule saved to " << filename << "\n";
}

// ✅ Load schedule from file
void loadScheduleFromFile(const string& filename = "schedule.txt") {
    ifstream in(filename);
    if (!in) {
        cout << "Error opening file for reading.\n";
        return;
    }

    schedule.clear();
    schedule.resize(MAX_DAYS);

    string line;
    int day = 0;

    while (getline(in, line)) {
        if (line.rfind("Day", 0) == 0) continue;

        istringstream iss(line);
        int start, end;
        iss >> start >> end;
        if (start == -1 && end == -1) {
            day++;
            continue;
        }
        if (day < MAX_DAYS)
            schedule[day].push_back({start, end});
    }

    in.close();
    cout << "\n✅ Schedule loaded from " << filename << "\n";
}

int main() {
    cout << "===========================\n";
    cout << " STUDENT DAILY SCHEDULER \n";
    cout << "===========================\n\n";

    int choice;
    do {
        cout << "\n========= MENU =========\n";
        cout << "1. Add Occupied Time Slots\n";
        cout << "2. Add Tasks\n";
        cout << "3. Generate Schedule\n";
        cout << "4. View Daily Summary\n";
        cout << "5. Exit\n";
        cout << "6. Save Schedule to File\n";
        cout << "7. Load Schedule from File\n";
        safeIntInput("Choose an option: ", choice);

        switch (choice) {
            case 1: addOccupiedSlots(); break;
            case 2: addTasks(); break;
            case 3: generateAndPrintSchedule(); break;
            case 4: showDailySummary(); break;
            case 5: cout << "\nThank you for using the Student Scheduler!\n"; break;
            case 6: saveScheduleToFile(); break;
            case 7: loadScheduleFromFile(); break;
            default: cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 5);

    return 0;
}
