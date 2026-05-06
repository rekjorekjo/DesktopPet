#ifndef TIMEDTRIGGER_H
#define TIMEDTRIGGER_H

#include <QString>

enum class TimedTriggerMode
{
    Interval,
    ClockTime
};

QString timedTriggerModeToString(TimedTriggerMode mode);
TimedTriggerMode timedTriggerModeFromString(const QString &value);

#endif // TIMEDTRIGGER_H
