#include "timedtrigger.h"

QString timedTriggerModeToString(TimedTriggerMode mode)
{
    switch (mode) {
        case TimedTriggerMode::ClockTime:
            return "clockTime";
        case TimedTriggerMode::Interval:
        default:
            return "interval";
    }
}

TimedTriggerMode timedTriggerModeFromString(const QString &value)
{
    if (value == "clockTime") {
        return TimedTriggerMode::ClockTime;
    }
    return TimedTriggerMode::Interval;
}
