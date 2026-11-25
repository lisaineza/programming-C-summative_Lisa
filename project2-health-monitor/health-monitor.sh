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
            3|4|5|6|7|8|9) echo "Not built yet."; pause ;;
            *) echo "Invalid choice. Enter a number from 0 to 9."
               log_activity "WARN" "Invalid menu choice: '$choice'" ;;
        esac
    done
}

main