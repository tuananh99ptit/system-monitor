#ifndef CPU_MONITOR_H
#define CPU_MONITOR_H
class CpuMonitor {
    public:
    // Constructor khoi tao moc thoi gian ban dau bang 0
    CpuMonitor();
    // tinh toan tra ve % CPU dang dang su dung
    float GetCpuUsage();
    private:
    // Luu Idle Time va Total time cua lan doc truoc do
    unsigned long long prevIdleTime;
    unsigned long long prevTotalTime;
}
#endif // CPU_MONITOR_H