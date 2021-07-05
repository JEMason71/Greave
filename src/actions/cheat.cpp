// actions/cheat.cpp -- Cheating, debugging and testing commands.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/cheat.hpp"
#include "actions/look.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/world.hpp"


// Adds money to the player's wallet.
void ActionCheat::add_money(int32_t amount)
{
    if (amount <= 0)
    {
        core()->message("{m}Don't be ridiculous.");
        return;
    }
    core()->world()->player()->add_money(amount);
    core()->message("{G}Your purse suddenly feels heavier!");
}

// Attempts to spawn an item.
void ActionCheat::spawn_item(std::string item)
{
    item = StrX::str_toupper(item);
    if (core()->world()->item_exists(item))
    {
        const std::shared_ptr<Item> new_item = core()->world()->get_item(item);
        core()->message("{C}You use the power of {R}w{Y}i{G}s{U}h{C}f{M}u{R}l {Y}t{G}h{U}i{C}n{M}k{R}i{Y}n{G}g {C}to bring " + new_item->name() + " {C}into the world!");
        core()->world()->player()->inv()->add_item(new_item);
    }
    else core()->message("{R}" + item + " {y}is not a valid item ID.");
}

// Attempts to spawn a mobile.
void ActionCheat::spawn_mobile(std::string mob)
{
    mob = StrX::str_toupper(mob);
    if (core()->world()->mob_exists(mob))
    {
        auto new_mob = core()->world()->get_mob(mob);
        core()->message("{C}You use the power of {R}w{Y}i{G}s{U}h{C}f{M}u{R}l {Y}t{G}h{U}i{C}n{M}k{R}i{Y}n{G}g {C}to summon " + new_mob->name() + " {C}into the world!");
        new_mob->set_location(core()->world()->player()->location());
        core()->world()->add_mobile(new_mob);
    }
    else core()->message("{R}" + mob + " {y}is not a valid mobile ID.");
}

// Attemtps to teleport to another room.
void ActionCheat::teleport(std::string dest)
{
    dest = StrX::str_toupper(dest);
    if (core()->world()->room_exists(dest))
    {
        core()->message("{U}The world around you {M}s{C}h{M}i{C}m{M}m{C}e{M}r{C}s{U}!");
        core()->world()->player()->set_location(StrX::hash(dest));
        ActionLook::look(core()->world()->player());
    }
    else core()->message("{R}" + dest + " {y}is not a valid room ID.");
}
