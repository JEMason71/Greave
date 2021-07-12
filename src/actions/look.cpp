// actions/look.cpp -- Look around you. Just look around you.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/look.hpp"
#include "core/core.hpp"
#include "core/guru.hpp"
#include "core/mathx.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/world.hpp"


// Examines an Item or Mobile.
void ActionLook::examine(ParserTarget target_type, size_t target)
{
    const auto world = core()->world();
    const auto player = world->player();
    switch (target_type)
    {
        case ParserTarget::TARGET_EQUIPMENT: examine_item(player->equ()->get(target)); break;
        case ParserTarget::TARGET_INVENTORY: examine_item(player->inv()->get(target)); break;
        case ParserTarget::TARGET_MOBILE: examine_mobile(world->mob_vec(target)); break;
        case ParserTarget::TARGET_ROOM: examine_item(world->get_room(player->location())->inv()->get(target)); break;
        default: core()->guru()->nonfatal("Invalid examine target.", Guru::ERROR);
    }
}

// Examines an Item.
void ActionLook::examine_item(std::shared_ptr<Item> target)
{
    core()->message("You are looking at: " + target->name(Item::NAME_FLAG_FULL_STATS | Item::NAME_FLAG_ID | Item::NAME_FLAG_RARE));
    if (target->desc().size()) core()->message("{0}" + target->desc());
    std::string stat_string;
    switch (target->type())
    {
        case ItemType::AMMO:
        {
            stat_string = "This is {U}ammunition {w}that can be fired from ";
            if (target->tag(ItemTag::AmmoArrow)) stat_string += "a bow. ";
            else if (target->tag(ItemTag::AmmoBolt)) stat_string += "a crossbow. ";
            else throw std::runtime_error("Unknown ammo type: " + target->name());

            std::vector<std::string> damage_str_vec;
            damage_str_vec.push_back("It has a damage multiplier of {U}" + StrX::ftos(target->ammo_power()) + "x{w}");
            if (target->crit()) damage_str_vec.push_back("a critical hit bonus of {U}" + std::to_string(target->crit()) + "%{w}");
            if (target->bleed()) damage_str_vec.push_back("a bleeding bonus of {U}" + std::to_string(target->bleed()) + "%{w}");
            if (target->poison()) damage_str_vec.push_back("a poison bonus of {U}" + std::to_string(target->poison()) + "%{w}");
            stat_string += StrX::comma_list(damage_str_vec, StrX::CL_FLAG_USE_AND | StrX::CL_FLAG_OXFORD_COMMA) + ". ";

            break;
        }
        case ItemType::ARMOUR:
        {
            switch (target->subtype())
            {
                case ItemSub::CLOTHING: stat_string = "This is {U}clothing {w}that can be worn"; break;
                case ItemSub::HEAVY: stat_string = "This is {U}heavy armour {w}that can be worn"; break;
                case ItemSub::LIGHT: stat_string = "This is {U}lightweight armour {w}that can be worn"; break;
                case ItemSub::MEDIUM: stat_string = "This is {U}medium armour {w}that can be worn"; break;
                default: break;
            }
            std::string slot;
            switch (target->equip_slot())
            {
                case EquipSlot::ABOUT_BODY: slot = "about the body"; break;
                case EquipSlot::ARMOUR: slot = "over your body"; break;
                case EquipSlot::BODY: slot = "on your body"; break;
                case EquipSlot::FEET: slot = "on your feet"; break;
                case EquipSlot::HANDS: slot = "on your hands"; break;
                case EquipSlot::HEAD: slot = "on your head"; break;
                default: break;
            }
            stat_string += " {U}" + slot + "{w}. It has an armour value of {U}" + std::to_string(target->power());
            const int warmth = target->warmth();
            if (warmth) stat_string += "{w}, and a warmth rating of {U}" + std::to_string(warmth);
            stat_string += "{w}. ";
            break;
        }
        case ItemType::DRINK:
        {
            switch (target->subtype())
            {
                case ItemSub::WATER_CONTAINER: stat_string = "This is a {U}water container{w}. "; break;
                default: break;
            }
            const int capacity = target->capacity(), charge = target->charge();
            stat_string += "It has a capacity of {U}" + std::to_string(capacity) + (capacity > 1 ? " units" : " unit") + "{w}";
            if (charge)
            {
                stat_string += ", and currently holds {U}" + std::to_string(charge) + (charge > 1 ? " units of " : " unit of ") + target->liquid_type() + "{w}, and will take {U}" + StrX::time_string_rough(target->speed()) + " {w}to drink. ";
            }
            else stat_string += ", and is currently {U}empty{w}. ";
            break;
        }
        case ItemType::FOOD:
        {
            stat_string = "This is something you can {U}consume{w}. ";
            stat_string += "It has a food value of {U}" + std::to_string(target->power()) + "{w}, and will take {U}" + StrX::time_string_rough(target->speed()) + " {w}to eat. ";
            break;
        }
        case ItemType::KEY: stat_string = "This is a {U}key {w}which can unlock certain doors. "; break;
        case ItemType::LIGHT: stat_string = "This is a {U}light source {w}which can be held. It provides a brightness level of {Y}" + std::to_string(target->power()) + "{w} when used. "; break;
        case ItemType::NONE: break;
        case ItemType::SHIELD: stat_string = "This is a {U}shield {w}which can be wielded. It has an armour value of {U}" + std::to_string(target->power()) + "{w}. "; break;
        case ItemType::WEAPON:
        {
            switch (target->subtype())
            {
                case ItemSub::MELEE: stat_string = "This is a {U}melee weapon {w}which can be wielded. "; break;
                case ItemSub::RANGED: stat_string = "This is a {U}ranged weapon {w}which can be widled. "; break;
                default: break;
            }
            if (target->tag(ItemTag::TwoHanded)) stat_string += "It is heavy and requires {U}two hands {w}to wield. ";
            else if (target->tag(ItemTag::HandAndAHalf)) stat_string += "It is versatile and can be wielded in {U}either one or two hands{w}. ";
            std::string damage_type_str;
            switch (target->damage_type())
            {
                case DamageType::ACID: damage_type_str = "acid"; break;
                case DamageType::BALLISTIC: damage_type_str = "ballistic"; break;
                case DamageType::CRUSHING: damage_type_str = "crushing"; break;
                case DamageType::EDGED: damage_type_str = "edged"; break;
                case DamageType::ENERGY: damage_type_str = "energy"; break;
                case DamageType::EXPLOSIVE: damage_type_str = "explosive"; break;
                case DamageType::KINETIC: damage_type_str = "kinetic"; break;
                case DamageType::PIERCING: damage_type_str = "piercing"; break;
                case DamageType::PLASMA: damage_type_str = "plasma"; break;
                case DamageType::POISON: damage_type_str = "poison"; break;
                case DamageType::RENDING: damage_type_str = "randing"; break;
                default: core()->guru()->nonfatal("Unable to determine item damage type: " + target->name(), Guru::ERROR);
            }
            std::vector<std::string> damage_str_vec;
            damage_str_vec.push_back("It has a damage value of {U}" + std::to_string(target->power()) + " " + damage_type_str + "{w}");
            damage_str_vec.push_back("a speed of {U}" + StrX::ftos(target->speed(), true) + "{w}");
            if (target->crit()) damage_str_vec.push_back("a critical hit chance of {U}" + std::to_string(target->crit()) + "%{w}");
            if (target->bleed()) damage_str_vec.push_back("a {U}" + std::to_string(target->bleed()) + "%{w} chance to cause bleeding wounds");
            if (target->poison()) damage_str_vec.push_back("a {U}" + std::to_string(target->poison()) + "%{w} chance to inflict poison");
            stat_string += StrX::comma_list(damage_str_vec, StrX::CL_FLAG_USE_AND | StrX::CL_FLAG_OXFORD_COMMA) + ". ";

            if (target->tag(ItemTag::AmmoArrow)) stat_string += "It uses {U}arrows {w}for ammunition. ";
            if (target->tag(ItemTag::AmmoBolt)) stat_string += "It uses {U}bolts {w}for ammunition. ";
            break;
        }
    }

    const int dodge_mod = target->dodge_mod();
    if (dodge_mod > 0) stat_string += "It {G}boosts your chance to dodge {w}by {G}" + std::to_string(dodge_mod) + "%{w}. ";
    else if (dodge_mod < 0) stat_string += "It {Y}reduces your chance to dodge {w}by {R}" + std::to_string(-dodge_mod) + "%{w}. ";

    const int parry_mod = target->parry_mod();
    if (parry_mod > 0) stat_string += "It {G}boosts your chance to parry {w}by {G}" + std::to_string(parry_mod) + "%{w}. ";
    else if (parry_mod < 0) stat_string += "It {Y}reduces your chance to parry {w}by {R}" + std::to_string(-parry_mod) + "%{w}. ";

    const int block_mod = target->block_mod();
    if (block_mod > 0) stat_string += "It {G}boosts your chance to shield-block {w}by {G}" + std::to_string(block_mod) + "%{w}. ";
    else if (block_mod < 0) stat_string += "It {Y}reduces your chance to shield-block {w}by {R}" + std::to_string(-block_mod) + "%{w}. ";

    const bool stackable = target->tag(ItemTag::Stackable);
    if (stackable) stat_string += "It can be {U}stacked {w}with other identical items. ";

    uint32_t weight = MathX::fuzz(target->weight());
    stat_string += (stackable ? "{w}The stack weighs around {U}" : "{w}It weighs around {U}") + StrX::intostr_pretty(weight) + (weight == 1 ? " pac" : " pacs");
    const int actual_value = target->value();
    const int appraised_value = MathX::fuzz(actual_value);
    if (!appraised_value) stat_string += (stackable ? "{w}, and {y}aren't worth anything{w}." : "{w}, and {y}isn't worth anything{w}. ");
    else stat_string += (stackable ? "{w}, and they are worth around {U}" : "{w}, and is worth around {U}") + StrX::mgsc_string(appraised_value, StrX::MGSC::LONG) + "{w}. ";

    if (stat_string.size())
    {
        stat_string.pop_back();
        core()->message("{0}" + stat_string);
    }
}

// Examines a Mobile.
void ActionLook::examine_mobile(std::shared_ptr<Mobile> target)
{
    core()->message("You are looking at: " + target->name());
}

// Take a look around at your surroundings.
void ActionLook::look()
{
    const auto player = core()->world()->player();
    const auto room = core()->world()->get_room(player->location());

    if (room->light() < Room::LIGHT_VISIBLE)
    {
        core()->message("{U}Darkness");
        core()->message("{0}```{u}It is {B}pitch black{u}, and you can see {B}nothing{u}. You are likely to be eaten by a grue.");
        return;
    }

    room->set_tag(RoomTag::Explored);

    // Room name, description, and obvious exits.
    core()->message("{G}" + room->name());
    core()->message("{0}```" + room->desc());

    // Show any scar effects on the room.
    const std::string scar_desc = room->scar_desc();
    if (scar_desc.size()) core()->message("{0}```" + scar_desc);

    // Show the obvious exits.
    obvious_exits(true);

    // Items nearby.
    if (room->inv()->count())
    {
        std::vector<std::string> items_nearby;
        for (size_t i = 0; i < room->inv()->count(); i++)
            items_nearby.push_back(room->inv()->get(i)->name(Item::NAME_FLAG_CORE_STATS) + "{w}");
        core()->message("{0}{g}```Items: {w}" + StrX::comma_list(items_nearby, StrX::CL_FLAG_USE_AND));
    }

    // Mobiles nearby.
    std::vector<std::string> mobs_nearby;
    for (size_t i = 0; i < core()->world()->mob_count(); i++)
    {
        const auto world_mob = core()->world()->mob_vec(i);
        if (!world_mob) continue; // Ignore any nullptr Mobiles.
        if (world_mob->location() != player->location()) continue;  // Ignore any Mobiles not in this Room.
        mobs_nearby.push_back((world_mob->is_hostile() ? "{R}" : "{Y}") + world_mob->name(Mobile::NAME_FLAG_NO_COLOUR) + "{w}");
    }
    if (mobs_nearby.size()) core()->message("{0}{g}```Nearby: {w}" + StrX::comma_list(mobs_nearby, StrX::CL_FLAG_USE_AND));
}

// Lists the exits from this area.
void ActionLook::obvious_exits(bool indent)
{
    const auto player = core()->world()->player();
    const auto room = core()->world()->get_room(player->location());
    if (room->light() < Room::LIGHT_VISIBLE)
    {
        core()->message(std::string(indent ? "{0}" : "") + "{u}It's so {B}dark{u}, you can't see where the exits are!");
        return;
    }

    std::vector<std::string> exits_vec;
    for (int e = 0; e < Room::ROOM_LINKS_MAX; e++)
    {
        uint32_t room_link = room->link(e);
        if (!room_link || room_link == Room::BLOCKED) continue;
        if (room->link_tag(e, LinkTag::Hidden)) continue;   // Never list hidden exits.
        std::string exit_name = "{c}" + StrX::dir_to_name(e);
        const std::string door_name = room->door_name(e);

        if (room_link == Room::UNFINISHED)  // An exit that is due to be finished later.
            exit_name = "{r}(" + StrX::dir_to_name(e) + "){c}";
        else if (room_link == Room::FALSE_ROOM)
        {
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else exit_name += " {u}[closed<>]{c}";
        }
        else
        {
            const std::shared_ptr<Room> link_room = core()->world()->get_room(room_link);
            if (link_room->tag(RoomTag::Explored) && !link_room->tag(RoomTag::Maze)) exit_name += " {B}(" + link_room->name(true) + "){c}";
            if (room->link_tag(e, LinkTag::KnownLocked)) exit_name += " {m}[locked<>]{c}";
            else if (room->link_tag(e, LinkTag::Openable))
            {
                if (room->link_tag(e, LinkTag::Open)) exit_name += " {u}[open<>]{c}";
                else exit_name += " {u}[closed<>]{c}";
            }
        }

        if (door_name == "door" || door_name == "metal door") StrX::find_and_replace(exit_name, "<>", "");   // Don't specify 'door' if it's just a door.
        else StrX::find_and_replace(exit_name, "<>", " " + door_name);  // Any other 'door' types (windows, grates, etc.) should be specified.

        exits_vec.push_back(exit_name);
    }
    if (exits_vec.size()) core()->message(std::string(indent ? "{0}```" : "") + "{g}Obvious exits: " + StrX::comma_list(exits_vec, StrX::CL_FLAG_USE_AND));
}
