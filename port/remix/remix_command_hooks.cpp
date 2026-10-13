/*
 * remix_command_hooks.cpp — connects the Remix custom moveset command state
 * (decomp/src/ft/ftremix/ftremixcommand.c) to the fighter events the engine
 * already fires for these Remix patches (port/hooks/list/FighterEvent.h):
 * create_hitbox_, apply_direction_ and override_env_color_.
 */
#include "remix_command_hooks.h"

#ifdef REGION_REMIX

#include <cstdint>

#include "hooks/Events.h"

extern "C" {
struct FTStruct;
struct FTAttackColl;

void ftRemixCommandResetAttack(int32_t player, int32_t attack_id);
void ftRemixCommandApplyDamageDirection(FTStruct* this_fp, FTStruct* attacker_fp, FTAttackColl* attack_coll);
uint32_t ftRemixCommandGetEnvColor(int32_t player);
}

namespace {

void OnHitboxSlotReset(IEvent* raw) {
    auto* ev = reinterpret_cast<FighterHitboxSlotResetEvent*>(raw);

    ftRemixCommandResetAttack(ev->player, ev->slot);
}

void OnDamageDirectionApply(IEvent* raw) {
    auto* ev = reinterpret_cast<FighterDamageDirectionApplyEvent*>(raw);

    ftRemixCommandApplyDamageDirection(static_cast<FTStruct*>(ev->this_fp), static_cast<FTStruct*>(ev->attacker_fp),
                                       static_cast<FTAttackColl*>(ev->attack_coll));
}

void OnEnvColorQuery(IEvent* raw) {
    auto* ev = reinterpret_cast<FighterEnvColorQueryEvent*>(raw);
    uint32_t rgba = ftRemixCommandGetEnvColor(ev->player);

    if (rgba != 0) {
        ev->rgba = rgba;
    }
}

} // namespace

void RegisterRemixCommandListeners() {
    REGISTER_LISTENER(FighterHitboxSlotResetEvent, EVENT_PRIORITY_NORMAL, OnHitboxSlotReset);
    REGISTER_LISTENER(FighterDamageDirectionApplyEvent, EVENT_PRIORITY_NORMAL, OnDamageDirectionApply);
    REGISTER_LISTENER(FighterEnvColorQueryEvent, EVENT_PRIORITY_NORMAL, OnEnvColorQuery);
}

#else

void RegisterRemixCommandListeners() {}

#endif
