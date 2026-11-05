/**
 * remix_player_panel.h - the Smash Remix per-port CSS panel (the css asm files, 20 options) as a runtime registry.
 *
 * The Remix panel is drawn inside the character select screens; the port does not redraw it. Each option is a value per
 * port, edited in the port menu (Remix > Player Panel) and stored as the CVar `gRemix.Panel.<Key>.P<port>`. A run can
 * override values with SSB64_REMIX_PANEL="<Key>.<port 1..4>=<value>;...", for example "Size.2=1;Damage.1=50". The match
 * hooks read the registry through remix_panel_get. The 20 custom player tags have a name each (remix_panel_tag_name).
 */

#ifndef PORT_REMIX_REMIX_PLAYER_PANEL_H
#define PORT_REMIX_REMIX_PLAYER_PANEL_H

#ifdef __cplusplus
extern "C" {
#endif

#define REMIX_PANEL_PORTS 4

typedef enum RemixPanelOption {
    REMIX_PANEL_SHIELD,
    REMIX_PANEL_PLAYER_TAG,
    REMIX_PANEL_VISIBILITY,
    REMIX_PANEL_SKELETON,
    REMIX_PANEL_MODEL_DISPLAY,
    REMIX_PANEL_INPUT_DISPLAY,
    REMIX_PANEL_SIZE,
    REMIX_PANEL_STOCK_MODE,
    REMIX_PANEL_KNOCKBACK,
    REMIX_PANEL_INPUT_DELAY,
    REMIX_PANEL_HANDICAP,
    REMIX_PANEL_START_WITH,
    REMIX_PANEL_TAUNT_ITEM,
    REMIX_PANEL_TAUNT_BUTTON,
    REMIX_PANEL_KIRBY_HAT,
    REMIX_PANEL_DPAD_FUNCTIONS,
    REMIX_PANEL_DPAD_CONTROL,
    REMIX_PANEL_DAMAGE,
    REMIX_PANEL_POISON_DAMAGE,
    REMIX_PANEL_PRACTICE_1P,
    REMIX_PANEL_STOCK_COUNT,
    REMIX_PANEL_COUNT
} RemixPanelOption;

/** Value of `option` for `port` (0..3), the option default for an invalid index. */
int remix_panel_get(int option, int port);

/** Sets `option` for `port`, clamped to the range of the option, and stores it in its CVar. */
void remix_panel_set(int option, int port, int value);

/** Sets every option of every port to its default. */
void remix_panel_reset(void);

/** Reads the SSB64_REMIX_PANEL overrides. Call once at start-up. */
void remix_panel_init(void);

/** Number of custom player tags (css/PlayerTag.asm MAX_TAGS). */
#define REMIX_PANEL_TAG_COUNT 20

/**
 * Name of the custom player tag `tag` (1..REMIX_PANEL_TAG_COUNT, the PlayerTag panel value): the text typed in the port menu
 * (Remix > Player Tags, CVar `gRemix.Panel.TagName.<tag>`) or set by SSB64_REMIX_TAG_NAMES="<tag>=<name>;...".
 * Returns "" for an unnamed or invalid tag. The pointer is valid until the next call with the same tag.
 */
const char *remix_panel_tag_name(int tag);

#ifdef __cplusplus
}

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace remix_panel {

struct Option {
    RemixPanelOption id;
    const char *key;
    const char *title;
    int min;
    int max;
    int fallback;
    std::vector<std::string> strings;
};

const std::vector<Option> &All();
std::string CVarName(const Option &option, int port);
std::string TagNameCVarName(int tag);
std::map<int32_t, const char *> ComboMap(const Option &option);

} // namespace remix_panel
#endif

#endif /* PORT_REMIX_REMIX_PLAYER_PANEL_H */
