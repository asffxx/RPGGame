#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct GameSession;

enum class ReputationTier { Hostile, Neutral, Friendly };

struct FactionReputation {
    std::string id;
    std::vector<std::string> friendlyStock;
    std::vector<std::string> hostileHidden;
};

class ReputationDatabase {
public:
    bool load(const std::filesystem::path& root, std::string& error);
    bool loaded() const { return loaded_; }
    const std::string& error() const { return error_; }
    int friendlyThreshold() const { return friendlyThreshold_; }
    int hostileThreshold() const { return hostileThreshold_; }
    int friendlyDiscountPercent() const { return friendlyDiscountPercent_; }
    int hostileMarkupPercent() const { return hostileMarkupPercent_; }
    int guildQuestStanding() const { return guildQuestStanding_; }
    const std::vector<FactionReputation>& factions() const { return factions_; }
    const FactionReputation* find(const std::string& id) const;
private:
    bool loaded_ = false;
    std::string error_;
    int friendlyThreshold_ = 6;
    int hostileThreshold_ = -6;
    int friendlyDiscountPercent_ = 10;
    int hostileMarkupPercent_ = 20;
    int guildQuestStanding_ = 3;
    std::vector<FactionReputation> factions_;
};

const ReputationDatabase& reputationDatabase();

ReputationTier tierFor(int standing);
const char* tierName(ReputationTier tier);

// City standing is the standing of the guild that lives in that city. Cities
// without a guild, and non-city locations, read as neutral zero.
int cityStanding(const GameSession& session, const std::string& city);
ReputationTier cityTier(const GameSession& session, const std::string& city);

// Adds to one guild's standing in the existing story store and clamps it.
void applyStanding(GameSession& session, const std::string& factionId, int delta);

long long adjustedBuyPrice(long long baseCopper, ReputationTier tier);

struct CoinAmount {
    int platinum = 0;
    int gold = 0;
    int silver = 0;
    int copper = 0;
};

// Converts a catalog price to adjusted coins for display and charging.
CoinAmount adjustedPrice(int platinum, int gold, int silver, int copper,
                         ReputationTier tier);
std::string priceLabel(const CoinAmount& price);

// True when an item is allowed in a faction's shop stock at this tier.
bool stockVisible(const std::string& factionId, ReputationTier tier,
                  const std::string& itemId);
