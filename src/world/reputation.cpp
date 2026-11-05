#include "reputation.hpp"

#include <algorithm>
#include <set>
#include <stdexcept>

#include "app/session.hpp"
#include "catalogs/content_paths.hpp"
#include "catalogs/guild.hpp"
#include "catalogs/spell.hpp"
#include "gameplay/datafile.hpp"
#include "gameplay/inventory.hpp"
#include "world/story_types.hpp"

namespace {
using Value = datafile::Value;

void require(bool condition, const std::string& message) {
    if (!condition) throw std::runtime_error(message);
}
void fields(const Value& value, const std::set<std::string>& allowed) {
    for (const auto& field : value.object())
        require(allowed.count(field.first), "unknown field: " + field.first);
}
std::string identifier(const Value& value, const std::string& key) {
    auto result = value.at(key).string();
    require(!result.empty() && result.size() <= 4096, key + ": expected nonempty text");
    return result;
}
Value read(const std::filesystem::path& path) {
    Value value;
    std::string error;
    if (!datafile::loadFile(path, value, error)) throw std::runtime_error(error);
    return value;
}
bool knownStockId(const PlayerInventory& inventory, const std::string& id) {
    PlayerInventory::ItemType type;
    size_t index;
    return inventory.resolveItemId(id, type, index) || SpellDatabase().find(id) != nullptr;
}
void readStock(const Value& value, const std::string& key,
               std::vector<std::string>& out, const PlayerInventory& inventory) {
    require(value.at(key).isArray(), key + ": expected array");
    require(value.at(key).size() <= 16, key + ": too many items");
    std::set<std::string> ids;
    for (const auto& entry : value.at(key).array()) {
        const auto id = entry.string();
        require(!id.empty() && id.size() <= 4096 && ids.insert(id).second,
                key + ": duplicate or invalid id " + id);
        require(knownStockId(inventory, id), key + ": unknown stock id " + id);
        out.push_back(id);
    }
}
}

bool ReputationDatabase::load(const std::filesystem::path& root, std::string& error) {
    loaded_ = false;
    factions_.clear();
    error_.clear();
    error.clear();
    const auto where = (root / "narrative/reputation.json").string();
    try {
        const auto document = read(where);
        fields(document, {"schemaVersion", "friendlyThreshold", "hostileThreshold",
                          "friendlyDiscountPercent", "hostileMarkupPercent",
                          "guildQuestStanding", "factions"});
        require(document.at("schemaVersion").integer() == 1, "unsupported schemaVersion");
        friendlyThreshold_ = document.at("friendlyThreshold").integer();
        hostileThreshold_ = document.at("hostileThreshold").integer();
        friendlyDiscountPercent_ = document.at("friendlyDiscountPercent").integer();
        hostileMarkupPercent_ = document.at("hostileMarkupPercent").integer();
        guildQuestStanding_ = document.at("guildQuestStanding").integer();
        require(friendlyThreshold_ > 0 && friendlyThreshold_ <= maxStanding,
                "friendlyThreshold outside 1..100");
        require(hostileThreshold_ < 0 && hostileThreshold_ >= minStanding,
                "hostileThreshold outside -100..-1");
        require(friendlyThreshold_ > hostileThreshold_, "friendly threshold must exceed hostile");
        require(friendlyDiscountPercent_ >= 1 && friendlyDiscountPercent_ <= 90,
                "friendlyDiscountPercent outside 1..90");
        require(hostileMarkupPercent_ >= 1 && hostileMarkupPercent_ <= 200,
                "hostileMarkupPercent outside 1..200");
        require(guildQuestStanding_ >= 0 && guildQuestStanding_ <= maxStanding,
                "guildQuestStanding outside 0..100");

        require(document.at("factions").size() == guildDatabase().getGuilds().size(),
                "expected one faction per guild");
        PlayerInventory inventory;
        std::set<std::string> ids;
        for (const auto& value : document.at("factions").array()) {
            fields(value, {"id", "friendlyStock", "hostileHidden"});
            FactionReputation faction;
            faction.id = identifier(value, "id");
            require(ids.insert(faction.id).second, "duplicate faction id: " + faction.id);
            require(guildDatabase().find(faction.id), "unknown guild: " + faction.id);
            readStock(value, "friendlyStock", faction.friendlyStock, inventory);
            readStock(value, "hostileHidden", faction.hostileHidden, inventory);
            for (const auto& item : faction.friendlyStock)
                require(std::find(faction.hostileHidden.begin(), faction.hostileHidden.end(), item) ==
                            faction.hostileHidden.end(),
                        "stock lists overlap: " + item);
            factions_.push_back(std::move(faction));
        }
        for (const auto& guild : guildDatabase().getGuilds())
            require(ids.count(guild.id), "missing faction: " + guild.id);
        loaded_ = true;
        return true;
    } catch (const std::exception& e) {
        error_ = where + ": " + e.what();
        error = error_;
        factions_.clear();
        return false;
    }
}

const ReputationDatabase& reputationDatabase() {
    static const ReputationDatabase database = [] {
        ReputationDatabase result;
        std::string error;
        result.load(paths::dataDir(), error);
        return result;
    }();
    return database;
}

const FactionReputation* ReputationDatabase::find(const std::string& id) const {
    for (const auto& faction : factions_) if (faction.id == id) return &faction;
    return nullptr;
}

ReputationTier tierFor(int standing) {
    if (standing >= reputationDatabase().friendlyThreshold()) return ReputationTier::Friendly;
    if (standing <= reputationDatabase().hostileThreshold()) return ReputationTier::Hostile;
    return ReputationTier::Neutral;
}

const char* tierName(ReputationTier tier) {
    switch (tier) {
        case ReputationTier::Friendly: return "Friendly";
        case ReputationTier::Hostile: return "Hostile";
        case ReputationTier::Neutral: return "Neutral";
    }
    return "Neutral";
}

int cityStanding(const GameSession& session, const std::string& city) {
    const auto* guild = guildDatabase().inCity(city);
    return guild ? session.story.standing(guild->id) : 0;
}

ReputationTier cityTier(const GameSession& session, const std::string& city) {
    return tierFor(cityStanding(session, city));
}

void applyStanding(GameSession& session, const std::string& factionId, int delta) {
    auto found = std::find_if(session.story.factions.begin(), session.story.factions.end(),
                              [&](const FactionStanding& entry) { return entry.factionId == factionId; });
    if (found == session.story.factions.end()) {
        session.story.factions.push_back({factionId, std::clamp(delta, minStanding, maxStanding)});
    } else {
        found->standing = std::clamp(found->standing + delta, minStanding, maxStanding);
    }
}

long long adjustedBuyPrice(long long baseCopper, ReputationTier tier) {
    if (baseCopper <= 0) return 0;
    long long percent = 100;
    if (tier == ReputationTier::Friendly) percent -= reputationDatabase().friendlyDiscountPercent();
    else if (tier == ReputationTier::Hostile) percent += reputationDatabase().hostileMarkupPercent();
    return std::max(0LL, baseCopper * percent / 100);
}

CoinAmount adjustedPrice(int platinum, int gold, int silver, int copper,
                         ReputationTier tier) {
    const long long base = static_cast<long long>(platinum) * 1000000LL +
                           static_cast<long long>(gold) * 10000LL +
                           static_cast<long long>(silver) * 100LL + copper;
    long long value = adjustedBuyPrice(base, tier);
    CoinAmount result;
    result.platinum = static_cast<int>(value / 1000000LL);
    value %= 1000000LL;
    result.gold = static_cast<int>(value / 10000LL);
    value %= 10000LL;
    result.silver = static_cast<int>(value / 100LL);
    result.copper = static_cast<int>(value % 100LL);
    return result;
}

std::string priceLabel(const CoinAmount& price) {
    std::string result;
    const auto add = [&](int value, char suffix) {
        if (value == 0) return;
        if (!result.empty()) result += ' ';
        result += std::to_string(value);
        result += suffix;
    };
    add(price.platinum, 'p');
    add(price.gold, 'g');
    add(price.silver, 's');
    add(price.copper, 'c');
    return result.empty() ? "0c" : result;
}

bool stockVisible(const std::string& factionId, ReputationTier tier,
                  const std::string& itemId) {
    const auto* faction = reputationDatabase().find(factionId);
    if (!faction) return true;
    for (const auto& id : faction->friendlyStock)
        if (id == itemId) return tier == ReputationTier::Friendly;
    for (const auto& id : faction->hostileHidden)
        if (id == itemId) return tier != ReputationTier::Hostile;
    return true;
}
