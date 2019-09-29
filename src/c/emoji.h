#ifndef emoji_h
#define emoji_h

static char* location_emojis[] = {
    "",
    "🏠",
    "🏢",
    "🏬",
    "⛲",
    "🏖",
    "🏞",
    "🚗",
    "🚲",
    "🚈",
    "🚌",
    "🚕",
    "⛴",
    "✈",
};

static char* action_emojis[] = {
    "",
    "🍽",
    "🚽",
    "🛁",
    "🛏",
    "💻",
    "📺",
    "🎮",
    "🎧",
    "🎲",
    "📕",
    "🛋",
    "🚶",
    "🛒",
};

#define LOCATION_EMOJI_COUNT (sizeof(location_emojis) / sizeof(location_emojis[0]))
#define ACTION_EMOJI_COUNT (sizeof(action_emojis) / sizeof(action_emojis[0]))

#endif
