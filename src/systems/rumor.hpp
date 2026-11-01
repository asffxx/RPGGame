#pragma once

#include <random>
#include "app/session.hpp"
#include "data/rumor.hpp"

std::vector<const RumorDefinition*> eligibleRumors(const GameSession& session);
const RumorDefinition* hearRumor(GameSession& session, std::mt19937& random);
void listenToRumors(GameSession& session);
