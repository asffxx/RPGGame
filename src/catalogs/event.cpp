#include "event.hpp"
#include "content.hpp"

eventDatabase::eventDatabase()
    : eventList(content::catalog<eventProperties>("world/events.json")) {}
