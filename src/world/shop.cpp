// world/shop.cpp -- The Shop class handles everything to do with shops that where the player can buy and sell items.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "3rdparty/SQLiteCpp/SQLiteCpp.h"
#include "core/core.hpp"
#include "core/list.hpp"
#include "core/mathx.hpp"
#include "core/parser.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/player.hpp"
#include "world/room.hpp"
#include "world/shop.hpp"
#include "world/world.hpp"


// SQL table construction string.
const std::string Shop::SQL_SHOPS = "CREATE TABLE shops ( id INTEGER PRIMARY KEY UNIQUE NOT NULL, inventory_id INTEGER UNIQUE NOT NULL )";


// Constructor, sets up a blank shop by default.
Shop::Shop(uint32_t room_id) : m_inventory(std::make_shared<Inventory>(Inventory::TagPrefix::SHOP)), m_room_id(room_id) { }

// Adds an item to this shop's inventory.
void Shop::add_item(std::shared_ptr<Item> item, bool sort)
{
    item->set_meta("appraised_value", item->value(true));
    m_inventory->add_item(item, true);
    if (sort) m_inventory->sort();
}

// Browses the wares on sale.
void Shop::browse() const
{
    if (!m_inventory->count())
    {
        core()->message("{u}There doesn't seem to be anything for sale here.");
        return;
    }
    core()->message("{c}The following is available to purchase:");
    for (size_t i = 0; i < m_inventory->count(); i++)
    {
        const auto item = m_inventory->get(i);
        std::string item_name = "{W}" + item->name(Item::NAME_FLAG_A | Item::NAME_FLAG_FULL_STATS) + " {W}(";
        item_name += StrX::mgsc_string(item->value(true), StrX::MGSC::SHORT);
        if (item->stack() > 1) item_name += " {W}each)";
        else item_name += "{W})";
        core()->message("{0}" + item_name);
    }
}

// Attempts to purchase something.
void Shop::buy(uint32_t id, int quantity)
{
    const auto player = core()->world()->player();
    if (quantity == -1) quantity = 1;
    else if (quantity < 1)
    {
        core()->message("{c}Please specify an actual number.");
        return;
    }

    const auto item = m_inventory->get(id);
    const bool stackable = item->tag(ItemTag::Stackable);
    const uint32_t stack_size = item->stack();

    if (static_cast<uint32_t>(quantity) > stack_size)
    {
        core()->message("{c}There aren't enough {C}" + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_PLURAL | Item::NAME_FLAG_NO_COUNT) + " {c}for you to buy {C}" + StrX::number_to_word(quantity) + "{c}!");
        return;
    }

    const uint32_t cost = item->value(true) * quantity;
    const uint32_t player_money = player->money();
    if (cost > player_money)
    {

        core()->message((quantity > 1 ? "{C}" + StrX::capitalize_first_letter(StrX::number_to_word(quantity)) + " " : "{C}") + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT | (quantity > 1 ? Item::NAME_FLAG_PLURAL : Item::NAME_FLAG_CAPITALIZE_FIRST | Item::NAME_FLAG_THE)) + " {c}would cost {C}" + StrX::strip_ansi(StrX::mgsc_string(cost, StrX::MGSC::LONG_COINS)) + (player_money ? "{c}, but you only have {C}" + StrX::strip_ansi(StrX::mgsc_string(player_money, StrX::MGSC::LONG_COINS)) + "{c}." : "{c}, but you have no coin at all."));
        return;
    }

    // Here's where it gets a bit awkward, with un-stacking normally unstackable objects. If the player is just buying *one* thing, and if the item they're buying is *either* unstackable or a single stackable item on its own, this is the easiest possible outcome. This also works if the player is buying *all* of a stack.
    if ((quantity == 1 && !stackable && stack_size == 1) || (stackable && static_cast<uint32_t>(quantity) == stack_size))
    {
        player->inv()->add_item(item);
        m_inventory->remove_item(id);
    }

    // We'll handle stackable and normally-unstackable items separately here. First, stackable items.
    else if (stackable)
    {
        auto split_item = std::make_shared<Item>(*item);
        split_item->set_stack(quantity);
        item->set_stack(item->stack() - quantity);
        player->inv()->add_item(split_item);
    }

    // Unstackable items are a similar deal, except we have to unstack them *one at a time*.
    else
    {
        item->set_stack(item->stack() - quantity);
        for (int i = 0; i < quantity; i++)
        {
            auto split_item = std::make_shared<Item>(*item);
            split_item->set_stack(1);
            player->inv()->add_item(split_item);
        }
        if (!item->stack()) m_inventory->erase(id);
    }

    core()->message("{g}You buy " + StrX::number_to_word(quantity) + " {G}" + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT | (quantity > 1 ? Item::NAME_FLAG_PLURAL : 0)) + " {g}for {G}" + StrX::strip_ansi(StrX::mgsc_string(cost, StrX::MGSC::LONG_COINS)) + "{g}.");
    player->remove_money(cost);
}

// Returns a pointer to the shop's inventory.
const std::shared_ptr<Inventory> Shop::inv() const { return m_inventory; }

// Loads a shop from the save file.
void Shop::load(std::shared_ptr<SQLite::Database> save_db)
{
    SQLite::Statement query(*save_db, "SELECT * FROM shops WHERE id = :id");
    query.bind(":id", m_room_id);
    if (query.executeStep())
    {
        m_inventory->load(save_db, query.getColumn("inventory_id").getUInt());
    }
    else throw std::runtime_error("Could not load shop data (ID: " + std::to_string(m_room_id) + ")");
}

// Restocks the contents of this shop.
void Shop::restock()
{
    const auto world = core()->world();
    m_inventory->clear();
    const std::string shop_list = "SHOP_" + StrX::str_toupper(world->get_room(m_room_id)->meta("shop_type"));
    auto list = world->get_list(shop_list);
    auto always_stock_list = world->get_list(shop_list + "_ALWAYS_STOCK");
    auto size_list = world->get_list(shop_list + "_SIZE");
    const int shop_size = MathX::mixup(size_list->at(0).count, 2);

    for (size_t i = 0; i < always_stock_list->size(); i++)
    {
        const auto new_item = world->get_item(always_stock_list->at(i).str, always_stock_list->at(i).count);
        add_item(new_item, false);
    }
    for (int i = 0; i < shop_size; i++)
    {
        const auto random_item = list->rnd();
        const auto new_item = world->get_item(random_item.str, random_item.count);
        add_item(new_item, false);
    }
    m_inventory->sort();
}

// Saves this Shop to the save file.
void Shop::save(std::shared_ptr<SQLite::Database> save_db) const
{
    const uint32_t inv_id = m_inventory->save(save_db);
    SQLite::Statement query(*save_db, "INSERT INTO shops ( id, inventory_id ) VALUES ( :id, :inventory_id )");
    query.bind(":id", m_room_id);
    query.bind(":inventory_id", inv_id);
    query.exec();
}

// Offers an item to the shop to sell.
void Shop::sell(uint32_t id, int quantity, bool confirm)
{
    const auto player = core()->world()->player();
    if (quantity == -1) quantity = 1;
    else if (quantity < 1)
    {
        core()->message("{c}Please specify an actual number.");
        return;
    }

    const auto item = player->inv()->get(id);
    const uint32_t stack_size = item->stack();

    if (static_cast<uint32_t>(quantity) > stack_size)
    {
        core()->message("{c}You don't have that many {C}" + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT | Item::NAME_FLAG_PLURAL) + "{c}.");
        return;
    }

    uint32_t value = item->value(true) * quantity;
    if (!value)
    {
        core()->message("{c}The shopkeeper isn't at all interested in your {C}" + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT | (quantity > 1 ? Item::NAME_FLAG_PLURAL : 0)) + "{c}.");
        return;
    }

    std::string sell_msg = (confirm ? "{g}The shopkeeper pays you {G}" : "{g}The shopkeeper offers you {G}") + StrX::strip_ansi(StrX::mgsc_string(value, StrX::MGSC::LONG_COINS)) + " {g}for " + (item->tag(ItemTag::ProperNoun) ? "" : "the ") + (quantity > 1 ? StrX::number_to_word(quantity) + " {G}" : "{G}") + item->name(Item::NAME_FLAG_NO_COLOUR | Item::NAME_FLAG_NO_COUNT | (quantity > 1 ? Item::NAME_FLAG_PLURAL : 0)) + "{g}.";
    if (!confirm) StrX::find_and_replace(sell_msg, "{g}", "{c}");
    if (!confirm) StrX::find_and_replace(sell_msg, "{G}", "{C}");
    core()->message(sell_msg);
    if (!confirm)
    {
        core()->parser()->confirm_message();
        return;
    }

    player->add_money(value);
    if (static_cast<uint32_t>(quantity) == stack_size)
    {
        add_item(item);
        player->inv()->remove_item(id);
    }
    else
    {
        auto item_split = std::make_shared<Item>(*item);
        item->set_stack(stack_size - quantity);
        item_split->set_stack(quantity);
        add_item(item_split);
    }
}
