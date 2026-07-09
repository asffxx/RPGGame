#pragma once

#include <string>

class Player;  // forward declaration; advanceTime mutates the player

// Tracks the in-game day/night cycle and elapsed time.
class TimeSystem {
public:
    enum class TimePeriod { Morning, Afternoon, Evening, Night };

    TimeSystem() : currentTurn(0), currentPeriod(TimePeriod::Morning), totalDays(0), totalWeeks(0) {}

    void advanceTime(Player& player);

    std::string getPeriodString() const;
    std::string getFormattedTimePassed() const;

    TimePeriod getCurrentPeriod() const { return currentPeriod; }
    int getCurrentTurn() const { return currentTurn; }
    int getTotalDays() const { return totalDays; }
    int getTotalWeeks() const { return totalWeeks; }

    void setState(int turn, TimePeriod period, int days) {
        currentTurn = turn;
        currentPeriod = period;
        totalDays = days;
        totalWeeks = totalDays / 7;
    }

private:
    int currentTurn;
    TimePeriod currentPeriod;
    int totalDays;
    int totalWeeks;
};
