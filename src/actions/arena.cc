// actions/arena.cc -- The arena, where you can participate in fights or bet on NPC fights.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/arena.h"

#include "actions/look.h"
#include "core/mathx.h"
#include "core/strx.h"


namespace greave {
namespace arena {

// One of the arena combatant mobiles died.
void combatant_died()
{
    // At some point in the future I'd like to support multiple combatants in arenas. For now, we'll just trigger the win condition.
    fight_over();
}

// The battle has been won!
void fight_over()
{
    const auto world = core()->world();
    const auto player = world->player();
    const auto arena_room = world->get_room(player->location());
    const Direction exit_dir = core()->parser()->parse_direction(arena_room->meta("arena_exit"));
    const std::string door_name = arena_room->meta("arena_door");

    arena_room->clear_link_tag(exit_dir, LinkTag::TempPermalock);
    arena_room->set_link_tag(exit_dir, LinkTag::Unlocked);
    const float hp_perc = player->hp() / player->hp(true);
    if (hp_perc <= 0.1f) core()->message("{m}Blood and sweat cloud your vision as your foe falls lifeless to the ground. You barely even notice the ovation of the crowd as you drag yourself towards the " + door_name + ", each step an agony.");
    else if (hp_perc < 0.3f) core()->message("{m}The bloodthirsty baying of the crowd seem distant and dull as you stumble, bloodied and beaten, towards the " + door_name + ". You were victorious, but at what cost?");
    else if (hp_perc > 0.9f) core()->message("{m}The crowd goes wild as you dispatch your opponent with ease, taking a moment for a hubristic victory pose before you head to the " + door_name + ".");
    else core()->message("{m}You take a moment to catch your breath, the cheers of the crowd drowned out by the heavy thump of your heartbeat. Victorious, you head to the " + door_name + ", leaving your opponent's ruined corpse on the arena floor.");
}

// Participates in an arena fight!
void participate()
{
    const auto world = core()->world();
    const auto player = world->player();
    const auto room = world->get_room(player->location());
    if (!room->tag(RoomTag::Arena))
    {
        core()->message("{y}There isn't anything for you to participate in here.");
        return;
    }

    const auto arena_spawn_list = world->get_list("ARENA_SPAWNS");
    const std::string arena_spawn_pick = arena_spawn_list->rnd().str;
    const auto mob = world->get_mob(arena_spawn_pick);
    const auto arena_room = world->get_room(room->meta("arena_room", false));

    core()->message("{R}The crowd roars with {r}bloodthirsty {R}delight as you step into the " + room->meta("arena_area") + ", a " + room->meta("arena_door") + " slamming shut behind you. Your opponent approaches from the far side, murder in " + mob->his_her() + " eyes.");
    player->set_location(arena_room->id());
    world->add_mobile(mob);
    mob->set_location(arena_room->id());
    mob->set_tag(MobileTag::ArenaFighter);
    player->set_tag(MobileTag::ArenaFighter);
    const Direction exit_dir = core()->parser()->parse_direction(arena_room->meta("arena_exit"));
    arena_room->clear_link_tag(exit_dir, LinkTag::Open);
    arena_room->clear_link_tag(exit_dir, LinkTag::Unlocked);
    arena_room->set_link_tag(exit_dir, LinkTag::TempPermalock);
    ActionLook::look();
}

// Time to collect your fight money.
void reward()
{
    const int coin_gained = MathX::mixup(500);
    std::string reward_str = "{G}You are the victor! {g}The pit master hands you a small bag of coins, inside which you find {G}" + StrX::mgsc_string(coin_gained, StrX::MGSC::LONG_COINS) + "{g}.";
    StrX::find_and_replace(reward_str, "{w}", "{g}");
    core()->message(reward_str);
    core()->world()->player()->add_money(coin_gained);
    core()->world()->player()->clear_tag(MobileTag::ArenaFighter);
}

}   // namespace arena
}   // namespace greave
