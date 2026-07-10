#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <signal.h>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <cstdlib>

using namespace std;

class Colors
{
public:
    const char *BLACK = "\033[0;30m";
    const char *RED = "\033[0;31m";
    const char *GREEN = "\033[0;32m";
    const char *BROWN = "\033[0;33m";
    const char *BLUE = "\033[0;34m";
    const char *PURPLE = "\033[0;35m";
    const char *CYAN = "\033[0;36m";
    const char *LIGHT_GRAY = "\033[0;37m";
    const char *DARK_GRAY = "\033[1;30m";
    const char *LIGHT_RED = "\033[1;31m";
    const char *LIGHT_GREEN = "\033[1;32m";
    const char *YELLOW = "\033[1;33m";
    const char *LIGHT_BLUE = "\033[1;34m";
    const char *LIGHT_PURPLE = "\033[1;35m";
    const char *LIGHT_CYAN = "\033[1;36m";
    const char *LIGHT_WHITE = "\033[1;37m";
    const char *BOLD = "\033[1m";
    const char *FAINT = "\033[2m";
    const char *ITALIC = "\033[3m";
    const char *UNDERLINE = "\033[4m";
    const char *BLINK = "\033[5m";
    const char *NEGATIVE = "\033[7m";
    const char *CROSSED = "\033[9m";
    const char *END = "\033[0m";
};

struct ProcessInfo
{
    int pid;
    string name;
    double cpuUsage;
    long memoryUsage;
    string user;
    int priority;
    string startTime;
};

struct NetworkUsage
{
    long rxBytes;
    long txBytes;
};

NetworkUsage getNetworkUsage(int pid)
{
    stringstream ss;
    ss << "/proc/" << pid << "/net/dev";
    ifstream netFile(ss.str());
    string line;
    NetworkUsage usage = {0, 0};

    getline(netFile, line); // Skip the first header line
    getline(netFile, line); // Skip the second header line

    while (getline(netFile, line))
    {
        string iface;
        long rxBytes, txBytes;
        stringstream ss(line);
        ss >> iface >> rxBytes;
        for (int i = 0; i < 8; ++i)
            ss >> txBytes; // The 9th column is txBytes
        usage.rxBytes += rxBytes;
        usage.txBytes += txBytes;
    }
    return usage;
}

void displayNetworkUsage(const NetworkUsage &usage)
{
    Colors color;
    cout << color.BOLD << color.BLINK << color.LIGHT_CYAN << "Network Usage:\n"
         << color.END;
    cout << color.BOLD << "Received Bytes: " << usage.rxBytes << " Bytes\n"
         << color.END;
    cout << color.BOLD << "Transmitted Bytes: " << usage.txBytes << " Bytes\n"
         << color.END;
}

void showNetworkUsageOption()
{
    Colors color;
    int pid;
    cout << color.BOLD << "Enter PID to show network usage: " << color.END;
    cin >> pid;
    NetworkUsage usage = getNetworkUsage(pid);
    displayNetworkUsage(usage);
}

long getSystemUptime()
{
    ifstream uptimeFile("/proc/uptime");
    long uptime;
    uptimeFile >> uptime;
    return uptime;
}

long getMemoryUsage(int pid)
{
    stringstream ss;
    ss << "/proc/" << pid << "/statm";
    ifstream statmFile(ss.str());
    long memory;
    statmFile >> memory;
    return memory * sysconf(_SC_PAGESIZE);
}

double getCpuUsage(int pid, long systemUptime)
{
    stringstream ss;
    ss << "/proc/" << pid << "/stat";
    ifstream statFile(ss.str());
    string ignore;
    long utime, stime, starttime;
    for (int i = 0; i < 13; ++i)
        statFile >> ignore;
    statFile >> utime >> stime;
    for (int i = 0; i < 4; ++i)
        statFile >> ignore;
    statFile >> starttime;

    long total_time = utime + stime;
    long seconds = systemUptime - (starttime / sysconf(_SC_CLK_TCK));
    return 100.0 * ((double)total_time / sysconf(_SC_CLK_TCK)) / seconds;
}

string getProcessName(int pid)
{
    stringstream ss;
    ss << "/proc/" << pid << "/comm";
    ifstream commFile(ss.str());
    string name;
    getline(commFile, name);
    return name;
}

string getProcessUser(int pid)
{
    stringstream ss;
    ss << "/proc/" << pid << "/status";
    ifstream statusFile(ss.str());
    string line;
    while (getline(statusFile, line))
    {
        if (line.find("Uid:") != string::npos)
        {
            stringstream ss(line);
            string uid;
            ss >> uid >> uid;
            return uid;
        }
    }
    return "unknown";
}

int getProcessPriority(int pid)
{
    return getpriority(PRIO_PROCESS, pid);
}

string convertToReadableTime(long ticks)
{
    time_t rawTime = ticks / sysconf(_SC_CLK_TCK);
    struct tm *timeinfo = localtime(&rawTime);
    char buffer[80];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", timeinfo);
    return string(buffer);
}

string getProcessStartTime(int pid)
{
    stringstream ss;
    ss << "/proc/" << pid << "/stat";
    ifstream statFile(ss.str());
    string ignore;
    long starttime;
    for (int i = 0; i < 21; ++i)
        statFile >> ignore;
    statFile >> starttime;
    return convertToReadableTime(starttime);
}

vector<ProcessInfo> listProcesses()
{
    vector<ProcessInfo> processes;
    DIR *procDir = opendir("/proc");
    dirent *entry;
    long systemUptime = getSystemUptime();

    while ((entry = readdir(procDir)) != nullptr)
    {
        if (isdigit(entry->d_name[0]))
        {
            int pid = stoi(entry->d_name);
            ProcessInfo pInfo;
            pInfo.pid = pid;
            pInfo.name = getProcessName(pid);
            pInfo.cpuUsage = getCpuUsage(pid, systemUptime);
            pInfo.memoryUsage = getMemoryUsage(pid);
            pInfo.user = getProcessUser(pid);
            pInfo.priority = getProcessPriority(pid);
            pInfo.startTime = getProcessStartTime(pid);
            processes.push_back(pInfo);
        }
    }
    closedir(procDir);
    return processes;
}

void displayProcesses(const vector<ProcessInfo> &processes)
{
    Colors color;
    cout << color.BOLD << color.LIGHT_GRAY << left << setw(8) << "PID" << setw(16) << "Name" << setw(12) << "CPU(%)"
         << setw(12) << "Memory(KB)" << setw(12) << "User" << setw(8) << "Priority"
         << setw(20) << "Start Time" << color.END << endl;
    cout << color.BOLD << color.DARK_GRAY << "-------------------------------------------------------------------------------" << color.END << endl;
    for (const auto &process : processes)
    {
        cout << color.BOLD << color.LIGHT_GRAY << left << setw(8) << process.pid << setw(16) << process.name
             << setw(12) << process.cpuUsage << setw(12) << process.memoryUsage / 1024
             << setw(12) << process.user << setw(8) << process.priority
             << setw(20) << process.startTime << color.END << endl;
    }
}

void terminateProcess(int pid, vector<ProcessInfo> &processes)
{
    Colors color;
    if (kill(pid, SIGKILL) == 0)
    {
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "Process " << pid << " terminated successfully." << color.END << endl;
        processes = listProcesses();
        displayProcesses(processes);
    }
    else
    {
        cerr << color.BOLD << color.BLINK << color.LIGHT_RED << "Error terminating process" << color.END;
    }
}

void adjustPriority(int pid, int priority)
{
    Colors color;
    if (setpriority(PRIO_PROCESS, pid, priority) == 0)
    {
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "Priority of process " << pid << " set to " << priority << color.END << endl;
    }
    else
    {
        cerr << color.BOLD << color.BLINK << color.LIGHT_RED << "Error adjusting priority" << color.END;
    }
}

bool sortByCpuUsage(const ProcessInfo &a, const ProcessInfo &b)
{
    return a.cpuUsage > b.cpuUsage;
}

bool sortByMemoryUsage(const ProcessInfo &a, const ProcessInfo &b)
{
    return a.memoryUsage > b.memoryUsage;
}

bool sortByPid(const ProcessInfo &a, const ProcessInfo &b)
{
    return a.pid < b.pid;
}

vector<ProcessInfo> filterByName(const vector<ProcessInfo> &processes, const string &name)
{
    vector<ProcessInfo> filtered;
    for (const auto &process : processes)
    {
        if (process.name.find(name) != string::npos)
        {
            filtered.push_back(process);
        }
    }
    return filtered;
}

vector<ProcessInfo> filterByPid(const vector<ProcessInfo> &processes, int pid)
{
    vector<ProcessInfo> filtered;
    for (const auto &process : processes)
    {
        if (process.pid == pid)
        {
            filtered.push_back(process);
        }
    }
    return filtered;
}

void showOptions(vector<ProcessInfo> &processes)
{
    Colors color;
    int choice;
    cout << color.BOLD << color.YELLOW << "\nSort by:\n1. CPU Usage\n2. Memory Usage\n3. PID\n4. Filter by Process Name\n5. Filter by PID\n6. Partial Search by Name\n"
         << color.END;
    cout << color.BOLD << "Enter your choice: " << color.END;
    cin >> choice;

    switch (choice)
    {
    case 1:
        sort(processes.begin(), processes.end(), sortByCpuUsage);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nSorted by CPU Usage:\n"
             << color.END;
        break;
    case 2:
        sort(processes.begin(), processes.end(), sortByMemoryUsage);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nSorted by Memory Usage:\n"
             << color.END;
        break;
    case 3:
        sort(processes.begin(), processes.end(), sortByPid);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nSorted by PID:\n"
             << color.END;
        break;
    case 4:
    {
        string name;
        cout << color.BOLD << "Enter process name to filter: " << color.END;
        cin >> name;
        processes = filterByName(processes, name);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nFiltered by Process Name:\n"
             << color.END;
        break;
    }
    case 5:
    {
        int pid;
        cout << color.BOLD << "Enter PID to filter: " << color.END;
        cin >> pid;
        processes = filterByPid(processes, pid);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nFiltered by PID:\n"
             << color.END;
        break;
    }
    case 6:
    {
        string partialName;
        cout << color.BOLD << "Enter partial name to search: " << color.END;
        cin >> partialName;
        processes = filterByName(processes, partialName);
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "\nProcesses matching '" << partialName << "':\n"
             << color.END;
        break;
    }
    default:
        cout << color.BOLD << color.BLINK << color.LIGHT_RED << "Invalid choice." << color.END << endl;
        return;
    }
    displayProcesses(processes);
}

void exportProcessesToCSV(const vector<ProcessInfo> &processes)
{
    Colors color;
    ofstream outFile("processes.csv");
    if (outFile.is_open())
    {
        outFile << "PID,Name,CPU(%),Memory(KB),User,Priority,Start Time\n";
        for (const auto &process : processes)
        {
            outFile << process.pid << ',' << process.name << ',' << process.cpuUsage << ','
                    << process.memoryUsage / 1024 << ',' << process.user << ','
                    << process.priority << ',' << process.startTime << '\n';
        }
        outFile.close();
        cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "Processes exported to processes.csv" << color.END << endl;
    }
    else
    {
        cerr << color.BOLD << color.BLINK << color.LIGHT_RED << "Unable to open file for writing." << color.END << endl;
    }
}

void refreshProcesses(vector<ProcessInfo> &processes)
{
    Colors color;
    processes = listProcesses();
    cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "Process list refreshed." << color.END << endl;
    displayProcesses(processes);
}

void runApplication()
{
    Colors color;
    string path;
    cout << color.BOLD << "Enter the path of the application/file/folder to run: " << color.END;
    cin.ignore();
    getline(cin, path);

    struct stat pathStat;
    if (stat(path.c_str(), &pathStat) != 0)
    {
        cerr << color.BOLD << color.BLINK << color.LIGHT_RED << "Error: Path does not exist or cannot be accessed.\n"
             << color.END;
        return;
    }

    // Check and use different terminal emulators
    const char *terminals[] = {"gnome-terminal", "xterm", "konsole"};
    bool launched = false;

    for (const auto &term : terminals)
    {
        if (system((string("command -v ") + term + " > /dev/null 2>&1").c_str()) == 0)
        {
            string command;
            if (S_ISDIR(pathStat.st_mode))
            {
                command = string(term) + " -- bash -c 'xdg-open " + path + "; exec bash'";
            }
            else if (S_ISREG(pathStat.st_mode) && (pathStat.st_mode & S_IXUSR))
            {
                command = string(term) + " -- bash -c '" + path + "; exec bash'";
            }
            else
            {
                command = string(term) + " -- bash -c 'xdg-open " + path + "; exec bash'";
            }
            if (system(command.c_str()) == 0)
            {
                launched = true;
                cout << color.BOLD << color.BLINK << color.LIGHT_GREEN << "Launched " << path << " in a new terminal successfully using " << term << "." << color.END << endl;
                break;
            }
        }
    }

    if (!launched)
    {
        cerr << color.BOLD << color.BLINK << color.LIGHT_RED << "Failed to launch a new terminal. Please check your terminal emulator setup.\n"
             << color.END;
    }
}

int main()
{
    Colors color;
    vector<ProcessInfo> processes = listProcesses();
    displayProcesses(processes);

    int option;
    do
    {
        cout << color.BOLD << color.BLINK << color.LIGHT_PURPLE << "\nOptions:\n"
             << color.END << color.BOLD << color.LIGHT_CYAN << "1. Show Network Usage\n2. Terminate a Process\n3. Adjust Process Priority\n4. Export to CSV\n5. Show Options\n6. Refresh Process List\n7. Run Application/File/Folder\n8. Exit\n"
             << color.END;
        cout << color.BOLD << color.YELLOW << "Select an option: " << color.END;
        cin >> option;

        switch (option)
        {
        case 1:
            showNetworkUsageOption();
            break;
        case 2:
        {
            int pid;
            cout << color.BOLD << "Enter PID to terminate: " << color.END;
            cin >> pid;
            terminateProcess(pid, processes);
            break;
        }
        case 3:
        {
            int pid, priority;
            cout << color.BOLD << "Enter PID to adjust priority: " << color.END;
            cin >> pid;
            cout << color.BOLD << "Enter new priority: " << color.END;
            cin >> priority;
            adjustPriority(pid, priority);
            break;
        }
        case 4:
            exportProcessesToCSV(processes);
            break;
        case 5:
            showOptions(processes);
            break;
        case 6:
            refreshProcesses(processes);
            break;
        case 7:
            runApplication();
            break;
        case 8:
            cout << color.BOLD << color.BLINK << color.LIGHT_RED << "Exiting..." << color.END << endl;
            break;
        default:
            cout << color.BOLD << color.BLINK << color.LIGHT_RED << "Invalid option. Please try again." << color.END << endl;
        }
    } while (option != 8);

    return 0;
}
