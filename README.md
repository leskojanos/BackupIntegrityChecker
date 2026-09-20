# Backup Integrity Checker

A high-performance command-line interface (CLI) tool written in C++17 for capturing directory states and identifying differences (new, modified, and deleted files) between two states at lightning speed. The program utilizes an optimized SQLite database and multi-threaded MD5 hashing, allowing it to process tens of thousands of files in seconds. With its structured JSON output, it integrates perfectly into automated backup scripts (PowerShell, Bash).

## Basic Concept: The Importance of Step 0

A reliable backup strategy is not just about copying files from point A to point B; it is about ensuring that the data you are about to back up is healthy and untampered. Performing an integrity check as "Step 0" before initiating the actual backup process is critical for several reasons:

* **Preventing Backup Poisoning:** If ransomware silently encrypts your local files, or a failing storage drive causes silent data corruption (bit rot), blindly running an automated backup script will overwrite your safe, historical backups with corrupted garbage.
* **Early Warning System:** By identifying exactly which files have been altered, added, or deleted since the last snapshot, you gain immediate visibility into suspicious system activities, malware behaviors, or accidental bulk deletions before they become permanent.
* **Targeted Synchronization:** Knowing the exact *delta* (the precise list of changes) allows your synchronization scripts to process only the files that actually need updating, avoiding blind full-directory overwrites and saving significant time and disk I/O.

## The solution: Backup Integrity Checker

It acts as this essential gatekeeper, verifying your directory's health and providing a machine-readable action list before a single byte gets transferred.

## 🚀 Key Features

* **Multi-threaded Processing:** Calculates MD5 hash values of files in parallel, utilizing all logical cores of the CPU (`ThreadPool`).
* **SQLite-based State Saving:** Saves file system metadata (size, modification date, hash) into a single `.db` file and determines differences using indexed, native SQL queries (`LEFT JOIN`), making the "diff" operation almost instantaneous.
* **Two-Phase Integrity Check:** Features an optimized "Fast Diff" mode that first compares metadata (file size), eliminating unnecessary hashing. A full MD5 check (`--full-check`) can optionally be forced.
* **Script-Friendly Architecture:** 
  * Dedicated Exit Codes for software flow control (`0` = no changes, `1` = error, `2` = changes detected).
  * Selectable JSON output (`-j` / `--json`) using the `nlohmann/json` library for seamless machine-to-machine communication.
* **Clean Code & RAII:** Ensures safe memory management, separated responsibilities, and a clean overall architecture.

## 🛠️ Dependencies and Build

The project includes an automated build system for Windows environments.

**System Requirements:**
* C++17 compatible compiler (e.g., MSVC, GCC, Clang)
* CMake (>= 3.16)
* [vcpkg](https://github.com/microsoft/vcpkg) package manager

**1. Install dependencies with vcpkg (Windows x64):**
```cmd
vcpkg install sqlite3:x64-windows
vcpkg install nlohmann-json:x64-windows

```

**2. Build:**
Ensure that the `VCPKG_ROOT` environment variable is set to your vcpkg directory. Run the build script in the root directory:

```cmd
build.bat

```

Upon a successful build, `backup_integrity_checker.exe` will be generated in the project root.

## 📖 Usage

The program operates in two main modes: **Scan** and **Compare**.

### 1. Scan (State Capture)

Creates a snapshot of the target directory and saves it to an SQLite database.

```cmd
backup_integrity_checker.exe "C:\Important_Folder" -o "D:\Backups"

```

*Result: A `backup_integrity_check_YYYYMMDD_HHMMSS.db` file is created in the `D:\Backups` folder.*

### 2. Compare (Diff)

Compares two database files to find new, deleted, or modified files.

**Detailed, human-readable output:**

```cmd
backup_integrity_checker.exe -c base.db new.db -v

```

**Strict (forced) MD5 comparison:**

```cmd
backup_integrity_checker.exe -cv -f base.db new.db

```

### 3. Integration into Backup Scripts (JSON mode)

For automation, use the `-j` (JSON) flag. The output is a highly structured, easily parsable data format.

```cmd
backup_integrity_checker.exe -c base.db final.db -j

```

**Sample PowerShell processing:**

```powershell
$jsonOutput = & .\backup_integrity_checker.exe -c base.db new.db -j
$result = $jsonOutput | ConvertFrom-Json

if ($LASTEXITCODE -eq 2) {
    Write-Host "Changes detected!"
    Write-Host "Number of new files: $($result.summary.added)"
    
    foreach ($file in $result.details.modified) {
        Write-Host "Modified: $($file.path) (Reason: $($file.reason))"
        # Synchronization logic can start here...
    }
} elseif ($LASTEXITCODE -eq 0) {
    Write-Host "The directory is unchanged."
}

```
## ⚙️ Command Line Options

| Flag | Description |
| --- | --- |
| `-h`, `--help` | Display the usage guide. |
| `-o <folder>` | Specify the output directory for scanning. |
| `-c` | Enable Compare (diff) mode. |
| `-v`, `--verbose` | Detailed visual console output (dates, sizes, hashes). |
| `-f`, `--full-check` | Force full MD5 hash check (ignores size-based pre-filtering). |
| `-p`, `--plain` | Script-friendly, simple list output (ACTION|PATH). |
| `-j`, `--json` | Structured JSON output for machine processing. |

## 🚦 Exit Codes (Error Levels)

The program returns specific exit codes (`%ERRORLEVEL%` in CMD, `$LASTEXITCODE` in PowerShell, or `$?` in Bash) to allow seamless flow control in automated backup pipelines:

| Code | Meaning | Description |
| --- | --- | --- |
| `0` | **Success / No Changes** | Execution completed successfully, and the two directory states are completely identical. |
| `1` | **Error** | An error occurred (e.g., invalid command-line parameters, missing directory, database connection or syntax failure). |
| `2` | **Changes Detected** | Execution completed successfully, but differences (added, missing, or modified files) were found between the two states. |

