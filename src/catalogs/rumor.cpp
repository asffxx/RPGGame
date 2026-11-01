#include "rumor.hpp"

RumorDatabase::RumorDatabase() {
    rumors = {
        {"road_attania", "", "Attania", "Follow the iron mileposts beyond the plains to Attania. Its Ironward Company is hiring roadwardens."},
        {"road_erden", "", "Erden", "The lantern road leads to Erden. Scholars there pay well for pages others mistake for rubbish."},
        {"road_trisselie", "", "Trisselie", "Follow the silver barges upriver to Trisselie. The merchants are whispering about missing cargo."},
        {"road_capital", "", "The Royal Capital", "The white-stone highway ends at The Royal Capital. Ask for the Crownward Order if you seek honorable work."},
        {"road_velmora", "", "Velmora", "Follow the salt wind west to Velmora. The Tidebound Fellowship still searches for a lost expedition."},
        {"attania_patrol", "Attania", "", "Captain Sera at the Ironward hall has heard nothing from her patrol. Start by searching Barren Plains."},
        {"attania_tracks", "Attania", "", "A goblin scout was seen in Shadowed Grove carrying a warden's map. The Ironward Company will want to hear of it."},
        {"erden_pages", "Erden", "", "Archivist Iven lost a research party in Enchanted Forest. The Lantern Scholars need someone who can read a trail."},
        {"erden_keeper", "Erden", "", "Bones still walk the shelves of Enchanted Library. A missing folio lies behind its restless keeper."},
        {"trisselie_ledger", "Trisselie", "", "Factor Mirelle's ledger vanished on the Ghost Town route. Visit the Silverthread hall before chasing it."},
        {"trisselie_cargo", "Trisselie", "", "An outlaw at Bandit Hideout has been selling cargo with silver seals. The Consortium remembers every stolen crate."},
        {"capital_courier", "The Royal Capital", "", "The Crownward Order in The Royal Capital awaits a courier from Whispering Woods. Warden Alis is worried."},
        {"capital_seal", "The Royal Capital", "", "A marauder in Abandoned Castle boasts of a royal seal. The Royal Capital cannot let that claim go unanswered."},
        {"velmora_journal", "Velmora", "", "Navigator Oren at the Tidebound hall has a chart ending at Desert Oasis. Perhaps the expedition left a journal."},
        {"velmora_relic", "Velmora", "", "The expedition's last cache was in Crystal Caverns. Sailors say one chest there has teeth."}
    };
}

const RumorDefinition* RumorDatabase::find(const std::string& id) const {
    for (const auto& rumor : rumors) if (rumor.id == id) return &rumor;
    return nullptr;
}

const RumorDatabase& rumorDatabase() {
    static const RumorDatabase database;
    return database;
}
