#include "timesystem.hpp"

#include <cmath>
#include <iostream>

#include "game/entities/player.hpp"

void TimeSystem::advanceTime(Player& player) {
    currentTurn++;
    switch (currentPeriod) {
        case TimePeriod::Morning:
            currentPeriod = TimePeriod::Afternoon;
            break;
        case TimePeriod::Afternoon:
            currentPeriod = TimePeriod::Evening;
            break;
        case TimePeriod::Evening:
            currentPeriod = TimePeriod::Night;
            break;
        case TimePeriod::Night:
            currentPeriod = TimePeriod::Morning;
            totalDays++;
            totalWeeks = totalDays / 7;

            if (!player.sleptToday) {
                int penalty = static_cast<int>(std::round(player.stats.maxHitpoints * 0.05f));
                player.stats.hitpoints -= penalty;
                if (player.stats.hitpoints < 1) player.stats.hitpoints = 1;
                std::cout << "[!] Your vision blurs. The shadows of 'The Lands Between' seem longer than usual." << std::endl;
            }

            player.sleptToday = false;
            break;
    }
}

std::string TimeSystem::getPeriodString() const {
    switch (currentPeriod) {
        case TimePeriod::Morning: return "Morning";
        case TimePeriod::Afternoon: return "Afternoon";
        case TimePeriod::Evening: return "Evening";
        case TimePeriod::Night: return "Night";
        default: return "Unknown";
    }
}

std::string TimeSystem::getFormattedTimePassed() const {
    int daysPassed = getTotalDays();
    if (daysPassed == 0) return "0 days";

    std::string result;
    int years = daysPassed / (12 * 4 * 7);
    int remaining = daysPassed % (12 * 4 * 7);
    int months = remaining / (4 * 7);
    remaining %= (4 * 7);
    int weeks = remaining / 7;
    int days = remaining % 7;

    if (years > 0) result += std::to_string(years) + " years ";
    if (months > 0) result += std::to_string(months) + " months ";
    if (weeks > 0) result += std::to_string(weeks) + " weeks ";
    if (days > 0) result += std::to_string(days) + " days ";

    if (!result.empty()) result.pop_back();
    return result;
}
