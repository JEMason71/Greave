// actions/cheat.cc -- Cheating, debugging and testing commands.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/cheat.h"
#include "actions/look.h"
#include "core/core.h"
#include "core/strx.h"


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

// Displays all the colours!
void ActionCheat::colours()
{
    core()->message(StrX::rainbow_text("COLOUR TESTING", "RYGCUM"));
    core()->message("{b}BLACK```{B}BOLD BLACK");
    core()->message("{0}{w}WHITE```{W}BOLD WHITE");
    core()->message("{0}{r}RED`````{R}BOLD RED");
    core()->message("{0}{y}YELLOW``{Y}BOLD YELLOW");
    core()->message("{0}{g}GREEN```{G}BOLD GREEN");
    core()->message("{0}{c}CYAN````{C}BOLD CYAN");
    core()->message("{0}{u}BLUE````{U}BOLD BLUE");
    core()->message("{0}{m}MAGENTA`{M}BOLD MAGENTA");
}

// Heals the player or an NPC.
void ActionCheat::heal(size_t target)
{
    const auto player = core()->world()->player();
    std::shared_ptr<Mobile> mob;
    if (target == SIZE_MAX) mob = player;
    else mob = core()->world()->mob_vec(target);

    if (mob->is_dead())
    {
        core()->message("{r}It's a little bit too late for that...");
        return;
    }

    mob->restore_hp(mob->hp(true));
    mob->clear_buff(Buff::Type::BLEED);
    mob->clear_buff(Buff::Type::POISON);
    mob->clear_tag(MobileTag::SnakeEyes);
    if (target == SIZE_MAX)
    {
        core()->message("{G}You feel hale and hearty!");
        player->restore_sp(player->sp(true));
        player->restore_mp(player->mp(true));
    }
    else
    {
        const std::string mob_name = mob->name(Mobile::NAME_FLAG_THE | Mobile::NAME_FLAG_CAPITALIZE_FIRST);
        if (mob->tag(MobileTag::Unliving)) core()->message("{G}" + mob_name + " {G}is fully restored!");
        else core()->message("{G}" + mob_name + " {G}is fully healed!");
    }
}

// Attempts to spawn an item.
void ActionCheat::spawn_item(std::string item)
{
    int count = -1;
    if (item.find(" ") != std::string::npos)
    {
        std::vector<std::string> split = StrX::string_explode(item, " ");
        if (split.size() == 2 && StrX::is_number(split.at(0)))
        {
            item = split.at(1);
            count = core()->parser()->parse_int(split.at(0));
        }
    }

    item = StrX::str_toupper(item);
    if (core()->world()->item_exists(item))
    {
        const std::shared_ptr<Item> new_item = core()->world()->get_item(item, count);
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
        ActionLook::look();
    }
    else core()->message("{R}" + dest + " {y}is not a valid room ID.");
}
