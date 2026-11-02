/**
 * remix_training.h - the Training custom menu of Smash Remix (Training.asm entries at 3899-3913) as a runtime registry.
 *
 * The Remix menu opens with Z during a Training match; the port edits the same entries in the port menu (Remix > Training) and stores
 * each as the CVar `gRemix.Training.<Key>`. A run can override values with SSB64_REMIX_TRAINING="<Key>=<value>;...", for example
 * "ShieldBreak=1;Oos=3;SpamPractice=7". The match code reads the registry through remix_training_get.
 */

#ifndef PORT_REMIX_REMIX_TRAINING_H
#define PORT_REMIX_REMIX_TRAINING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum RemixTrainingOption {
    REMIX_TRAINING_SHIELD_BREAK,
    REMIX_TRAINING_OOS,
    REMIX_TRAINING_MUSIC,
    REMIX_TRAINING_STAGE_BG,
    REMIX_TRAINING_TECH,
    REMIX_TRAINING_DI_TYPE,
    REMIX_TRAINING_DI_STRENGTH,
    REMIX_TRAINING_DI_DIRECTION,
    REMIX_TRAINING_DI_FIRST_HIT,
    REMIX_TRAINING_DPAD,
    REMIX_TRAINING_DI_PRACTICE,
    REMIX_TRAINING_SPAM_PRACTICE,
    REMIX_TRAINING_SPAM_INTERVAL,
    REMIX_TRAINING_SPAM_RANDOM,
    REMIX_TRAINING_PERCENT_P1,
    REMIX_TRAINING_PERCENT_P2,
    REMIX_TRAINING_PERCENT_P3,
    REMIX_TRAINING_PERCENT_P4,
    REMIX_TRAINING_RESET_SETS_PERCENT_P1,
    REMIX_TRAINING_RESET_SETS_PERCENT_P2,
    REMIX_TRAINING_RESET_SETS_PERCENT_P3,
    REMIX_TRAINING_RESET_SETS_PERCENT_P4,
    REMIX_TRAINING_COUNT
} RemixTrainingOption;

/** Per-port entries (Training.asm tail_px): the option of `port` 0..3 for a base of the P1 id. */
#define REMIX_TRAINING_PERCENT(port) (REMIX_TRAINING_PERCENT_P1 + (port))
#define REMIX_TRAINING_RESET_SETS_PERCENT(port) (REMIX_TRAINING_RESET_SETS_PERCENT_P1 + (port))

/** D-pad Controls entry values (string_table_dpad_controls). */
#define REMIX_TRAINING_DPAD_ON 0
#define REMIX_TRAINING_DPAD_RESET_ONLY 1
#define REMIX_TRAINING_DPAD_OFF 2

/** Value of `option`, its default for an invalid index. */
int remix_training_get(int option);

/** Sets `option`, clamped to its range, and stores it in its CVar. */
void remix_training_set(int option, int value);

/** Sets every option to its default. */
void remix_training_reset(void);

/** Reads the SSB64_REMIX_TRAINING overrides. Call once at start-up. */
void remix_training_init(void);

#ifdef __cplusplus
}

#include <cstdint>
#include <map>
#include <string>
#include <vector>

namespace remix_training {

struct Option {
    RemixTrainingOption id;
    const char *key;
    const char *title;
    int min;
    int max;
    int fallback;
    std::vector<std::string> strings;
};

const std::vector<Option> &All();
std::string CVarName(const Option &option);
std::map<int32_t, const char *> ComboMap(const Option &option);

} // namespace remix_training
#endif

#endif
