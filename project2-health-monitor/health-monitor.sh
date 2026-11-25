#!/bin/bash

LOG_FILE="$(dirname "$(readlink -f "$0")")/activity.log"
CPU_THRESHOLD=80
MEM_THRESHOLD=80
DISK_THRESHOLD=80

# write a timestamped entry to the activity log
log_activity() {
    local level="$1" message="$2"
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] [$level] $message" >> "$LOG_FILE" 2>/dev/null
}

# check that a command exists before using it
require_cmd() {
    if ! command -v "$1" >/dev/null 2>&1; then
        echo "Error: '$1' is not available on this system."
        log_activity "ERROR" "Missing utility: $1"
        return 1
    fi
    return 0
}

# true if the value is a whole number from 1 to 100
is_valid_percent() {
    [[ "$1" =~ ^[0-9]+$ ]] && [ "$1" -ge 1 ] && [ "$1" -le 100 ]
}

pause() {
    read -rp "Press Enter to continue..."
}

# ask for a threshold, keep the current one if input is empty
prompt_threshold() {
    local label="$1" current="$2" input
    while true; do
        read -rp "Enter $label threshold % [current: $current]: " input
        if [ -z "$input" ]; then
            echo "$current"
            return
        fi
        if is_valid_percent "$input"; then
            echo "$input"
            return
        fi
        echo "Invalid value. Enter a whole number from 1 to 100." >&2
    done
}

show_system_info() {
    echo ""
    echo "System information"
    echo "Hostname  : $(hostname)"
    if [ -r /etc/os-release ]; then
        echo "OS        : $(. /etc/os-release && echo "$PRETTY_NAME")"
    else
        echo "OS        : $(uname -s)"
    fi
    echo "Kernel    : $(uname -r)"
    echo "Uptime    : $(uptime -p 2>/dev/null || uptime)"
    echo "Date/Time : $(date '+%Y-%m-%d %H:%M:%S')"
    log_activity "INFO" "Viewed system information"
}

# overall CPU usage in percent, measured over one second
get_cpu_usage() {
    local user nice system idle iowait irq softirq steal
    local total1 idle1 total2 idle2 dtotal didle

    read -r _ user nice system idle iowait irq softirq steal _ < /proc/stat
    total1=$((user + nice + system + idle + iowait + irq + softirq + steal))
    idle1=$((idle + iowait))

    sleep 1

    read -r _ user nice system idle iowait irq softirq steal _ < /proc/stat
    total2=$((user + nice + system + idle + iowait + irq + softirq + steal))
    idle2=$((idle + iowait))

    dtotal=$((total2 - total1))
    didle=$((idle2 - idle1))

    if [ "$dtotal" -le 0 ]; then
        echo 0
    else
        echo $(( (dtotal - didle) * 100 / dtotal ))
    fi
}

monitor_cpu() {
    if [ ! -r /proc/stat ]; then
        echo "Cannot read /proc/stat on this system."
        log_activity "ERROR" "CPU monitoring failed: /proc/stat unreadable"
        return
    fi
    require_cmd ps || return

    CPU_THRESHOLD=$(prompt_threshold "CPU" "$CPU_THRESHOLD")

    echo ""
    echo "Measuring CPU usage..."
    local usage
    usage=$(get_cpu_usage)
    echo "CPU usage      : ${usage}%"
    echo "CPU threshold  : ${CPU_THRESHOLD}%"
    log_activity "INFO" "CPU usage ${usage}% (threshold ${CPU_THRESHOLD}%)"

    if [ "$usage" -ge "$CPU_THRESHOLD" ]; then
        echo "WARNING: CPU usage is above the threshold."
        log_activity "WARN" "CPU usage ${usage}% exceeds threshold ${CPU_THRESHOLD}%"
    else
        echo "CPU usage is within the normal range."
    fi

    echo ""
    echo "Top 5 processes by CPU"
    printf "%-8s %-20s %s\n" "PID" "NAME" "CPU%"
    ps -eo pid,comm,%cpu --sort=-%cpu --no-headers | head -5 | \
        awk '{printf "%-8s %-20s %s\n", $1, $2, $3}'

    echo ""
    echo "Processes above ${CPU_THRESHOLD}% CPU"
    local heavy
    heavy=$(ps -eo pid,comm,%cpu --sort=-%cpu --no-headers | \
        awk -v t="$CPU_THRESHOLD" '$3 + 0 >= t {printf "%-8s %-20s %s\n", $1, $2, $3}')
    if [ -z "$heavy" ]; then
        echo "None"
    else
        echo "$heavy"
        log_activity "WARN" "Processes above ${CPU_THRESHOLD}% CPU detected"
    fi
}

monitor_memory() {
    require_cmd free || return

    MEM_THRESHOLD=$(prompt_threshold "memory" "$MEM_THRESHOLD")

    local total used available percent
    read -r total used available < <(free -m | awk '/^Mem:/ {print $2, $3, $7}')

    if [ -z "$total" ] || [ "$total" -le 0 ]; then
        echo "Could not read memory information."
        log_activity "ERROR" "Memory monitoring failed: no data from free"
        return
    fi

    percent=$(( (total - available) * 100 / total ))

    echo ""
    echo "Memory utilization"
    echo "Total      : ${total} MB"
    echo "Used       : ${used} MB"
    echo "Available  : ${available} MB"
    echo "Utilization: ${percent}%"
    echo "Threshold  : ${MEM_THRESHOLD}%"
    log_activity "INFO" "Memory usage ${percent}% (threshold ${MEM_THRESHOLD}%)"

    if [ "$percent" -ge "$MEM_THRESHOLD" ]; then
        echo "WARNING: memory utilization is above the threshold."
        log_activity "WARN" "Memory usage ${percent}% exceeds threshold ${MEM_THRESHOLD}%"
    else
        echo "Memory utilization is within the normal range."
    fi

    echo ""
    echo "Top 5 processes by memory"
    printf "%-8s %-20s %s\n" "PID" "NAME" "MEM%"
    ps -eo pid,comm,%mem --sort=-%mem --no-headers | head -5 | \
        awk '{printf "%-8s %-20s %s\n", $1, $2, $3}'
}

monitor_disk() {
    require_cmd df || return

    DISK_THRESHOLD=$(prompt_threshold "disk" "$DISK_THRESHOLD")

    # skip virtual filesystems, keep real mounted storage
    local disk_data
    disk_data=$(df -hP -x tmpfs -x devtmpfs -x squashfs 2>/dev/null | tail -n +2)

    if [ -z "$disk_data" ]; then
        echo "Could not read disk information."
        log_activity "ERROR" "Disk monitoring failed: no data from df"
        return
    fi

    echo ""
    echo "Disk utilization"
    printf "%-28s %-8s %-8s %-8s %-6s %s\n" "FILESYSTEM" "SIZE" "USED" "AVAIL" "USE%" "MOUNT"
    echo "$disk_data" | awk '{printf "%-28s %-8s %-8s %-8s %-6s %s\n", $1, $2, $3, $4, $5, $6}'
    log_activity "INFO" "Viewed disk utilization (threshold ${DISK_THRESHOLD}%)"

    echo ""
    echo "Filesystems at or above ${DISK_THRESHOLD}%"
    local full
    full=$(echo "$disk_data" | awk -v t="$DISK_THRESHOLD" '
        { use = $5; sub(/%/, "", use)
          if (use + 0 >= t) printf "%s mounted on %s is %s full\n", $1, $6, $5 }')

    if [ -z "$full" ]; then
        echo "None"
    else
        echo "$full"
        echo "$full" | while read -r line; do
            log_activity "WARN" "Disk threshold exceeded: $line"
        done
    fi
}
# print process rows in a fixed-width table
print_process_rows() {
    printf "%-8s %-12s %-20s %-6s %s\n" "PID" "USER" "NAME" "CPU%" "MEM%"
    awk '{printf "%-8s %-12s %-20s %-6s %s\n", $1, $2, $3, $4, $5}'
}

list_processes() {
    echo ""
    echo "Running processes (top 20 by CPU)"
    ps -eo pid=,user=,comm=,%cpu=,%mem= --sort=-%cpu | head -20 | print_process_rows
    log_activity "INFO" "Listed running processes"
}

search_process() {
    local term results
    read -rp "Enter process name or PID: " term

    if [ -z "$term" ]; then
        echo "Search term cannot be empty."
        return
    fi

    if [[ "$term" =~ ^[0-9]+$ ]]; then
        results=$(ps -p "$term" -o pid=,user=,comm=,%cpu=,%mem= 2>/dev/null)
    else
        results=$(ps -eo pid=,user=,comm=,%cpu=,%mem= | \
            awk -v t="$term" 'index(tolower($3), tolower(t)) > 0')
    fi

    if [ -z "$results" ]; then
        echo "No process found matching '$term'."
        log_activity "WARN" "Process search found nothing for '$term'"
        return
    fi

    echo ""
    echo "$results" | print_process_rows
    log_activity "INFO" "Searched processes for '$term'"
}

process_menu() {
    require_cmd ps || return
    while true; do
        echo ""
        echo "Process viewer"
        echo "1. List running processes"
        echo "2. Search by name or PID"
        echo "0. Back to main menu"
        read -rp "Choose an option [0-2]: " sub
        case "$sub" in
            1) list_processes ;;
            2) search_process ;;
            0) return ;;
            *) echo "Invalid choice. Enter 0, 1 or 2." ;;
        esac
    done
}
# system processes that must never be terminated
PROTECTED_NAMES="init systemd kthreadd sshd dbus-daemon cron rsyslogd systemd-journal systemd-logind systemd-udevd NetworkManager"

# true if the process is protected: low PIDs, kernel threads, this script, its parent, or a listed name
is_protected_process() {
    local pid="$1" name ppid entry
    name=$(ps -p "$pid" -o comm= 2>/dev/null)
    ppid=$(ps -p "$pid" -o ppid= 2>/dev/null | tr -d ' ')

    [ "$pid" -le 2 ] && return 0
    [ "$ppid" = "2" ] && return 0
    [ "$pid" = "$$" ] && return 0
    [ "$pid" = "$PPID" ] && return 0
    for entry in $PROTECTED_NAMES; do
        [ "$name" = "$entry" ] && return 0
    done
    return 1
}

# ask a yes/no question, return success only on y or yes
confirm() {
    local answer
    read -rp "$1 [y/N]: " answer
    [[ "$answer" =~ ^[Yy]([Ee][Ss])?$ ]]
}

terminate_process() {
    require_cmd ps || return
    require_cmd kill || return

    local pid name owner
    read -rp "Enter the PID to terminate: " pid

    if ! [[ "$pid" =~ ^[0-9]+$ ]] || [ "$pid" -le 0 ]; then
        echo "Invalid PID. Enter a positive whole number."
        log_activity "WARN" "Terminate rejected: invalid PID '$pid'"
        return
    fi

    if ! ps -p "$pid" >/dev/null 2>&1; then
        echo "No process with PID $pid exists."
        log_activity "WARN" "Terminate rejected: PID $pid not found"
        return
    fi

    name=$(ps -p "$pid" -o comm= | tr -d ' ')
    owner=$(ps -p "$pid" -o user= | tr -d ' ')

    echo ""
    ps -p "$pid" -o pid=,user=,comm=,%cpu=,%mem= | print_process_rows

    if is_protected_process "$pid"; then
        echo "Refused: '$name' (PID $pid) is a protected process."
        log_activity "WARN" "Terminate blocked: protected process $name (PID $pid)"
        return
    fi

    if [ "$owner" != "$(id -un)" ] && [ "$(id -u)" -ne 0 ]; then
        echo "Permission denied: PID $pid belongs to '$owner'."
        log_activity "WARN" "Terminate blocked: no permission for $name (PID $pid, owner $owner)"
        return
    fi

    if ! confirm "Terminate $name (PID $pid)?"; then
        echo "Cancelled."
        log_activity "INFO" "Terminate cancelled for $name (PID $pid)"
        return
    fi

    if kill -15 "$pid" 2>/dev/null; then
        sleep 1
        if ps -p "$pid" >/dev/null 2>&1; then
            echo "Process is still running after a normal terminate."
            if confirm "Force kill PID $pid?"; then
                kill -9 "$pid" 2>/dev/null
                sleep 1
            fi
        fi
        if ps -p "$pid" >/dev/null 2>&1; then
            echo "Process $pid is still running."
            log_activity "ERROR" "Failed to terminate $name (PID $pid)"
        else
            echo "Process $name (PID $pid) terminated."
            log_activity "INFO" "Terminated process $name (PID $pid)"
        fi
    else
        echo "Could not signal PID $pid."
        log_activity "ERROR" "kill failed for $name (PID $pid)"
    fi
}
generate_report() {
    require_cmd free || return
    require_cmd df || return
    require_cmd ps || return

    local default_dir dir stamp file cpu total used available mem_percent
    default_dir="$(dirname "$LOG_FILE")/reports"

    read -rp "Directory to save the report [default: $default_dir]: " dir
    dir="${dir:-$default_dir}"
    dir="${dir/#\~/$HOME}"

    if [ ! -d "$dir" ]; then
        if confirm "Directory '$dir' does not exist. Create it?"; then
            if ! mkdir -p "$dir" 2>/dev/null; then
                echo "Could not create '$dir'. Check the path and your permissions."
                log_activity "ERROR" "Report failed: cannot create directory $dir"
                return
            fi
        else
            echo "Cancelled."
            log_activity "INFO" "Report cancelled: directory $dir not created"
            return
        fi
    fi

    if [ ! -w "$dir" ]; then
        echo "Permission denied: cannot write to '$dir'."
        log_activity "ERROR" "Report failed: no write access to $dir"
        return
    fi

    stamp=$(date '+%Y%m%d_%H%M%S')
    file="$dir/health_report_${stamp}.txt"

    echo "Collecting system data..."
    cpu=$(get_cpu_usage)
    read -r total used available < <(free -m | awk '/^Mem:/ {print $2, $3, $7}')

    if [ -z "$total" ] || [ "$total" -le 0 ]; then
        echo "Could not read memory information."
        log_activity "ERROR" "Report failed: no memory data"
        return
    fi
    mem_percent=$(( (total - available) * 100 / total ))

    if ! {
        echo "System health report"
        echo "Generated : $(date '+%Y-%m-%d %H:%M:%S')"
        echo "Hostname  : $(hostname)"
        echo "Kernel    : $(uname -r)"
        echo "Uptime    : $(uptime -p 2>/dev/null || uptime)"
        echo ""
        echo "CPU"
        echo "Usage     : ${cpu}% (threshold ${CPU_THRESHOLD}%)"
        if [ "$cpu" -ge "$CPU_THRESHOLD" ]; then
            echo "Status    : WARNING, above threshold"
        else
            echo "Status    : OK"
        fi
        echo ""
        echo "Memory"
        echo "Total     : ${total} MB"
        echo "Used      : ${used} MB"
        echo "Available : ${available} MB"
        echo "Usage     : ${mem_percent}% (threshold ${MEM_THRESHOLD}%)"
        if [ "$mem_percent" -ge "$MEM_THRESHOLD" ]; then
            echo "Status    : WARNING, above threshold"
        else
            echo "Status    : OK"
        fi
        echo ""
        echo "Disk (threshold ${DISK_THRESHOLD}%)"
        printf "%-28s %-8s %-8s %-8s %-6s %s\n" "FILESYSTEM" "SIZE" "USED" "AVAIL" "USE%" "MOUNT"
        df -hP -x tmpfs -x devtmpfs -x squashfs 2>/dev/null | tail -n +2 | \
            awk -v t="$DISK_THRESHOLD" '{
                use = $5; sub(/%/, "", use)
                flag = (use + 0 >= t) ? "  WARNING" : ""
                printf "%-28s %-8s %-8s %-8s %-6s %s%s\n", $1, $2, $3, $4, $5, $6, flag
            }'
        echo ""
        echo "Top 5 processes by CPU"
        printf "%-8s %-20s %s\n" "PID" "NAME" "CPU%"
        ps -eo pid,comm,%cpu --sort=-%cpu --no-headers | head -5 | \
            awk '{printf "%-8s %-20s %s\n", $1, $2, $3}'
    } > "$file"; then
        echo "Failed to write the report file."
        log_activity "ERROR" "Report failed: could not write $file"
        return
    fi

    echo "Report saved to: $file"
    log_activity "INFO" "Generated health report: $file"
}
show_menu() {
    echo ""
    echo "System Health Monitor"
    echo "1. Display system information"
    echo "2. Monitor CPU utilization"
    echo "3. Monitor memory utilization"
    echo "4. Monitor disk utilization"
    echo "5. View and search running processes"
    echo "6. Terminate a process"
    echo "7. Generate system health report"
    echo "8. View activity history"
    echo "9. Clear activity log"
    echo "0. Exit"
}

main() {
    touch "$LOG_FILE" 2>/dev/null || { echo "Cannot write to log file: $LOG_FILE"; exit 1; }
    log_activity "INFO" "Application started"

    while true; do
        show_menu
        read -rp "Choose an option [0-9]: " choice
        case "$choice" in
            1) show_system_info; pause ;;
            0) log_activity "INFO" "Application exited"; echo "Goodbye."; exit 0 ;;
            2) monitor_cpu; pause ;;
            3) monitor_memory; pause ;;
            4) monitor_disk; pause ;;
            5) process_menu; pause ;;
            6) terminate_process; pause ;;
            7) generate_report; pause ;;
            8|9) echo "Not built yet."; pause ;;
            *) echo "Invalid choice. Enter a number from 0 to 9."
               log_activity "WARN" "Invalid menu choice: '$choice'" ;;
        esac
    done
}

main