# 🖥️ Linux Process Monitor

A terminal-based **Linux Process Monitoring Tool** built in **C++** using the Linux `/proc` filesystem. This project provides real-time process information, process management utilities, network usage statistics, CSV export functionality, and process priority control.

---

## ✨ Features

- 📋 Display running processes
- 🆔 View Process ID (PID)
- 📛 View Process Name
- ⚡ Monitor CPU Usage
- 💾 Display Memory Usage
- 👤 Display User ID
- 🔼 View Process Priority
- 🕒 Display Process Start Time
- 🌐 Monitor Network Usage
- ❌ Terminate Processes
- ⚙️ Adjust Process Priority (Nice Value)
- 📄 Export Process Information to CSV
- 🔄 Refresh Process List
- 🚀 Launch Applications, Files, or Folders

---

## 🛠️ Technologies Used

- C++
- Linux `/proc` Filesystem
- POSIX System Calls
- STL (Standard Template Library)
- File Handling

---

## 📂 Project Structure

```
Linux-Process-Monitor/
│
├── process_monitor.cpp
├── README.md
```

---

## ⚙️ Compilation

Compile using g++:

```bash
g++ -std=c++17 process_monitor.cpp -o process_monitor
```

---

## ▶️ Running the Application

```bash
./process_monitor
```

---

## 📋 Menu Options

```
1. Show Network Usage
2. Terminate a Process
3. Adjust Process Priority
4. Export to CSV
5. Show Options
6. Refresh Process List
7. Run Application/File/Folder
8. Exit
```

---

## 📄 CSV Export

Selecting **Option 4** exports the currently displayed process information into:

```
processes.csv
```

The CSV file contains:

- PID
- Process Name
- CPU Usage
- Memory Usage
- User
- Priority
- Start Time

---

## ⚠️ Known Issues

### 1. WSL Compatibility

The **Run Application/File/Folder** feature (**Option 7**) currently relies on Linux desktop terminal emulators such as:

- gnome-terminal
- xterm
- konsole

These applications are generally **not available in Windows Subsystem for Linux (WSL)**.

As a result, the application may display:

```
Failed to launch a new terminal.
```

The feature works correctly on a standard Linux desktop environment. WSL compatibility will be added in a future update.

---

### 2. Incorrect Process Start Time

The process start time currently displays dates such as:

```
1970-01-01 00:00:01
```

This occurs because the current implementation directly interprets the `starttime` field from `/proc/[pid]/stat`.

A future update will calculate the correct timestamp using the system boot time (`btime`) together with the process start ticks.

This issue affects **only the displayed start time**. All other process information continues to function correctly.

---

## 🚀 Future Improvements

- ✅ Correct process start time calculation
- ✅ Full WSL compatibility
- ✅ Search and filter processes
- ✅ Sort by CPU or Memory usage
- ✅ Better terminal UI
- ✅ Improved error handling
- ✅ Additional process statistics

---

## 💻 Requirements

- Linux Operating System
- GCC/G++ Compiler
- `/proc` filesystem support

---

## 📚 Concepts Used

- Linux Process Management
- Process Control
- File Parsing
- System Programming
- POSIX APIs
- C++ STL
- CSV File Generation

---

## 👨‍💻 Author

 Yashi
