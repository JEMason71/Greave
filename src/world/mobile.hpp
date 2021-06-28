// world/mobile.hpp -- The Mobile class defines entities that can move and interact with the game world. Derived classes are used for more specific entities.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#pragma once
#include "core/greave.hpp"

class Inventory;    // defined in world/inventory.hpp
namespace SQLite { class Database; }    // defined in 3rdparty/SQLiteCpp/Database.h


class Mobile
{
public:
    enum class Type : uint8_t { NPC, PLAYER };  // enum used in type() below, to easily check if we're dealing with an NPC or Player when using a Mobile pointer.

    static const std::string    SQL_MOBILES;    // The SQL table construction string for Mobiles.

                        Mobile();                                   // Constructor, sets default values.
    const std::shared_ptr<Inventory>    inv() const;                // Returns a pointer to the Mobile's Inventory.
    virtual void        load(std::shared_ptr<SQLite::Database> save_db, unsigned int sql_id);   // Loads a Mobile.
    uint32_t            location() const;                           // Retrieves the location of this Mobile, in the form of a Room ID.
    virtual uint32_t    save(std::shared_ptr<SQLite::Database> save_db);    // Saves this Mobile.
    void                set_location(uint32_t room_id);             // Sets the location of this Mobile with a Room ID.
    void                set_location(const std::string &room_id);   // As above, but with a string Room ID.
    virtual Type        type() = 0;                                 // Mobile should never be instantiated directly, only NPC or Player.

private:
    std::shared_ptr<Inventory>  m_inventory;    // The Items being carried by this Mobile.
    uint32_t    m_location; // The Room that this Mobile is currently located in.
};
