// combat/combat.cpp -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "combat/combat.hpp"
#include "core/random.hpp"
#include "core/strx.hpp"
#include "world/inventory.hpp"
#include "world/item.hpp"
#include "world/mobile.hpp"


// Weapon type damage modifiers to unarmoured, light, medium and heavy armour targets.
const float	Combat::DAMAGE_MODIFIER_ACID[4] =       { 1.8f, 1.3f, 1.2f, 1.0f };
const float Combat::DAMAGE_MODIFIER_BALLISTIC[4] =  { 1.3f, 1.3f, 1.2f, 1.0f };
const float Combat::DAMAGE_MODIFIER_CRUSHING[4] =   { 1.0f, 1.0f, 1.0f, 1.2f };
const float Combat::DAMAGE_MODIFIER_EDGED[4] =      { 1.5f, 1.3f, 1.2f, 1.0f };
const float Combat::DAMAGE_MODIFIER_EXPLOSIVE[4] =  { 1.1f, 1.1f, 1.1f, 1.5f };
const float Combat::DAMAGE_MODIFIER_ENERGY[4] =     { 1.1f, 1.0f, 1.0f, 1.2f };
const float Combat::DAMAGE_MODIFIER_KINETIC[4] =    { 1.0f, 1.0f, 1.0f, 1.2f };
const float Combat::DAMAGE_MODIFIER_PIERCING[4] =   { 1.2f, 1.2f, 1.2f, 1.0f };
const float Combat::DAMAGE_MODIFIER_PLASMA[4] =     { 1.5f, 1.2f, 1.0f, 1.2f };
const float Combat::DAMAGE_MODIFIER_POISON[4] =     { 1.8f, 1.2f, 1.0f, 0.8f };
const float Combat::DAMAGE_MODIFIER_RENDING[4] =    { 1.5f, 1.3f, 1.1f, 1.1f };

const std::map<DamageType, const float*>    Combat::DAMAGE_TYPE_MAP = { { DamageType::ACID, DAMAGE_MODIFIER_ACID }, { DamageType::BALLISTIC, DAMAGE_MODIFIER_BALLISTIC },
    { DamageType::CRUSHING, DAMAGE_MODIFIER_CRUSHING }, { DamageType::EDGED, DAMAGE_MODIFIER_EDGED }, { DamageType::ENERGY, DAMAGE_MODIFIER_ENERGY },
    { DamageType::KINETIC, DAMAGE_MODIFIER_KINETIC }, { DamageType::PIERCING, DAMAGE_MODIFIER_PIERCING }, { DamageType::PLASMA, DAMAGE_MODIFIER_PLASMA },
    { DamageType::POISON, DAMAGE_MODIFIER_POISON },  { DamageType::RENDING, DAMAGE_MODIFIER_RENDING }, { DamageType::EXPLOSIVE, DAMAGE_MODIFIER_EXPLOSIVE } };


// Applies damage modifiers based on weapon type.
float Combat::apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot)
{
    if (!damage || !weapon || !defender) return damage;
    const DamageType dt = weapon->damage_type();
    if (DAMAGE_TYPE_MAP.find(dt) == DAMAGE_TYPE_MAP.end())
        throw std::runtime_error("Unknown damage type: " + std::to_string(static_cast<int>(dt)));
    const float *damage_modifier = DAMAGE_TYPE_MAP.at(dt);

    int armour_type = 0;    // 0 = unarmoured, 1 = light armour, 2 = medium armour, 3 = heavy armour.
    std::shared_ptr<Item> armour = defender->equ()->get(slot);
    if (armour && armour->type() == ItemType::ARMOUR)
    {
        switch (armour->subtype())
        {
            case ItemSub::LIGHT: armour_type = 1; break;
            case ItemSub::MEDIUM: armour_type = 2; break;
            case ItemSub::HEAVY: armour_type = 3; break;
            default: break;
        }
    }
    if (!armour_type) return damage;
    else return damage * damage_modifier[armour_type];
}

// Generates a standard-format damage number string.
std::string Combat::damage_number_str(int damage, int blocked, bool crit, bool bleed, bool poison)
{
    std::string dmg_str;
    if (crit) dmg_str = "{w}[{m}*{M}-"; else dmg_str = "{w}[{R}-";
    dmg_str += StrX::intostr_pretty(damage);
    if (bleed && !crit) dmg_str += "B";
    if (poison && !crit) dmg_str += "P";
    if (crit) dmg_str += "{m}*";
    dmg_str += "{w}]";
    if (blocked > 0) dmg_str += " {w}<{U}" + StrX::intostr_pretty(blocked) + "{w}>";
    return dmg_str;
}

// Returns an appropriate damage string.
std::string Combat::damage_str(unsigned int damage, std::shared_ptr<Mobile> def, bool heat)
{
    const float percentage = (static_cast<float>(damage) / static_cast<float>(def->hp(true))) * 100;
    if (percentage >= 200000) return StrX::rainbow_text("SUPERNOVAS", "RYW");
    else if (percentage >= 150000) return StrX::rainbow_text("METEORITES", "UMm");
    else if (percentage >= 125000) return StrX::rainbow_text("GLACIATES", "CUW");
    else if (percentage >= 100000) return StrX::rainbow_text("NUKES", "WYR");
    else if (percentage >= 80000) return StrX::rainbow_text("RUPTURES", "RMr");
    else if (percentage >= 65000) return StrX::rainbow_text("SLAUGHTERS", "MRm");
    else if (percentage >= 50000) return StrX::rainbow_text("SHATTERS", "GCU");
    else if (percentage >= 40000) return StrX::rainbow_text("EXTERMINATES", "GYC");
    else if (percentage >= 30000) return StrX::rainbow_text("IMPLODES", "UMR");
    else if (percentage >= 20000) return StrX::rainbow_text("ANNIHILATES", "RGU");
    else if (percentage >= 15000) return StrX::rainbow_text("CREMATES", "YyR");
    else if (percentage >= 12500) return StrX::rainbow_text("WASTES", "mUC");
    else if (percentage >= 10000) return StrX::rainbow_text("TEARS INTO", "mRM");
    else if (percentage >= 9000) return StrX::rainbow_text("SUNDERS", "umr");
    else if (percentage >= 8000) return StrX::rainbow_text("EVAPORATES", "YCU");
    else if (percentage >= 7000) return StrX::rainbow_text("LIQUIDATES", "CUW");
    else if (percentage >= 6000) return StrX::rainbow_text("FISSURES", "UMR");
    else if (percentage >= 5000) return StrX::rainbow_text("RAVAGES", "mry");
    else if (percentage >= 4000) return StrX::rainbow_text("ASPHYXIATES", "MUC");
    else if (percentage >= 3000) return StrX::rainbow_text("ATOMIZES", "CYG");
    else if (percentage >= 2500) return StrX::rainbow_text("VAPORIZES", "YCU");
    else if (percentage >= 2000) return StrX::rainbow_text("PULVERIZES", "mRM");
    else if (percentage >= 1800) return StrX::rainbow_text("DESTROYS", "UMm");
    else if (percentage >= 1600) return StrX::rainbow_text("SHREDS", "MRm");
    else if (percentage >= 1400) return StrX::rainbow_text("DEMOLISHES", "UmM");
    else if (percentage >= 1200) return StrX::rainbow_text("BLASTS", "RyY");
    else if (percentage >= 1000) return StrX::rainbow_text("RENDS", "mrM");
    else if (percentage >= 900) return StrX::rainbow_text("DISMEMBERS", "RrM");
    else if (percentage >= 800) return StrX::rainbow_text("MASSACRES", "MRm");
    else if (percentage >= 700) return StrX::rainbow_text("DISEMBOWELS", "mRr");
    else if (percentage >= 600) return StrX::rainbow_text("MUTILATES", "mRM");
    else if (percentage >= 500) return StrX::rainbow_text("INCINERATES", "rYR");
    else if (percentage >= 400) return StrX::rainbow_text("EXTIRPATES", "GCU");
    else if (percentage >= 300) return StrX::rainbow_text("OBLITERATES", "mMU");
    else if (percentage >= 200) return StrX::rainbow_text("ERADICATES", "UmM");
    else if (percentage >= 150) return StrX::rainbow_text("DEVASTATES", "YGC");
    else if (percentage >= 100) return StrX::rainbow_text("DECIMATES", "yYR");
    else if (percentage >= 90) return StrX::rainbow_text("LACERATES", "mRM");
    else if (percentage >= 80) return "{R}mars";
    else if (percentage >= 70) return "{R}mangles";
    else if (percentage >= 60) return "{R}maims";
    else if (percentage >= 50) return "{R}mauls";
    else if (percentage >= 40) return "{R}wounds";
    else if (percentage >= 30) return "{Y}injures";
    else if (percentage >= 25) return "{Y}damages";
    else if (percentage >= 20) return "{Y}scars";
    else if (heat)
    {
        if (percentage >= 15) return "{y}scorches";
        else if (percentage >= 10) return "{y}chars";
        else if (percentage >= 5) return "{y}sears";
        else if (percentage >= 1) return "{y}scalds";
        else return "{w}singes";
    }
    else
    {
        if (percentage >= 15) return "{y}nicks";
        else if (percentage >= 10) return "{y}grazes";
        else if (percentage >= 5) return "{y}scratches";
        else if (percentage >= 1) return "{y}bruises";
        else return "{w}tickles";
    }
}

// Determines type of weapons wielded by a Mobile.
void Combat::determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_melee, bool* can_off_melee)
{
    std::shared_ptr<Item> main_hand = mob->equ()->get(EquipSlot::HAND_MAIN);
    std::shared_ptr<Item> off_hand = mob->equ()->get(EquipSlot::HAND_OFF);
    *can_main_melee = (main_hand && main_hand->type() == ItemType::WEAPON && main_hand->subtype() == ItemSub::MELEE);
    *can_off_melee = (off_hand && off_hand->type() == ItemType::WEAPON && off_hand->subtype() == ItemSub::MELEE);
    const bool off_shield = (off_hand && off_hand->type() == ItemType::SHIELD);

    // If both hands are empty, it's a melee attack.
    if (!main_hand && !off_hand)
    {
        *wield_type = WieldType::UNARMED;
        *can_main_melee = true;
        *can_off_melee = true;
    }

    // Dual-wielding is an easy one to detect. One melee weapon in each hand.
    else if (*can_main_melee && *can_off_melee) *wield_type = WieldType::DUAL_WIELD;

    // Good old sword and board: melee weapon in one hand, shield in the other.
    else if (*can_main_melee && off_shield) *wield_type = WieldType::ONE_HAND_PLUS_SHIELD;

    // Two-handers can only be equipped in the main hand.
    else if (*can_main_melee && main_hand->tag(ItemTag::TwoHanded)) *wield_type = WieldType::TWO_HAND;

    // Single-wielding a one-handed weapon isn't the best choice, but it can be done.
    else if ((*can_main_melee && !off_hand) || (*can_off_melee && !main_hand))
    {
        // Check if we're using a hand-and-a-half weapon with the other hand free, or just a regular one-hander on its own.
        if ((*can_main_melee && main_hand->tag(ItemTag::HandAndAHalf)) || (*can_off_melee && off_hand->tag(ItemTag::HandAndAHalf))) *wield_type = WieldType::HAND_AND_A_HALF_2H;
        else *wield_type = WieldType::SINGLE_WIELD;
    }

    // We've already checked for sword-and-board above, so the only option left if one hand is holding a weapon is that the other hand is holding something
    // non-combat related. Yay for the process of elimination!
    else if (*can_main_melee || *can_off_melee) *wield_type = WieldType::ONE_HAND_PLUS_EXTRA;

    // Now we're getting into the silly options, but gotta cover every base. Is the Mobile wielding a shield in one hand, and nothing in the other?
    // As ridiculous as that is for a loadout, punching while holding a shield should be allowed.
    else if (off_shield && !main_hand) *wield_type = WieldType::UNARMED_PLUS_SHIELD;

    // If either hand is now free, by process of elimination, it must be an unarmed attack with a non-combat item in the other hand.
    else if (!main_hand || !off_hand)
    {
        *wield_type = WieldType::UNARMED;
        if (main_hand) *can_main_melee = true;
        if (off_hand) *can_off_melee = true;
    }

    // The only other possible configurations, through process of elimination, is shield+misc:
    else if (off_shield) *wield_type = WieldType::SHIELD_ONLY;

    // ...or the final option, which is the only thing that remains now, both hands occupied by non-combat items:
    else *wield_type = WieldType::NONE;
}

// Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
void Combat::pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name)
{
    const auto body_parts = mob->get_anatomy();
    const int bp_roll = core()->rng()->rnd(100);
    *slot = EquipSlot::NONE;
    for (unsigned int i = 0; i < body_parts.size(); i++)
    {
        if (bp_roll < body_parts.at(i)->hit_chance) continue;
        *slot = body_parts.at(i)->slot;
        *slot_name = body_parts.at(i)->name;
        break;
    }
    if (*slot == EquipSlot::NONE) throw std::runtime_error("Could not determine body hit location for " + mob->name());
}

// Returns a threshold string, if a damage threshold has been passed.
std::string Combat::threshold_str(std::shared_ptr<Mobile> defender, int damage, const std::string& good_colour, const std::string& bad_colour)
{
    const bool is_player = defender->is_player();
    const bool alive = !defender->tag(MobileTag::Unliving);
    const bool plural = defender->tag(MobileTag::PluralName) || is_player;
    const std::string name = (is_player ? " You " : (plural ? " They " : " " + StrX::capitalize_first_letter(defender->he_she()) + " "));

    const float old_perc = defender->hp() / static_cast<float>(defender->hp(true));
    float new_perc = (defender->hp() - damage) / static_cast<float>(defender->hp(true));
    if (defender->hp() <= damage) new_perc = 0;

    if (old_perc >= 0.99f && new_perc >= 0.95f) return bad_colour + name + (alive ? (plural ? "barely notice." : "barely notices.") :
        (plural ? "are barely scratched." : "is barely scratched."));
    if (old_perc >= 0.99f && new_perc >= 0.95f) return bad_colour + name + (alive ? (plural ? "barely notice." : "barely notices.") :
        (plural ? "are barely scratched." : "is barely scratched."));
    if (old_perc >= 0.95f && new_perc >= 0.90f) return bad_colour + name + (alive ? (plural ? "shrug it off." : "shrugs it off.") :
        (plural ? "are hardly damaged." : "is hardly damaged."));
    if (old_perc >= 0.9f && new_perc == 0.0f) return good_colour + name + (plural ? "are utterly annihilated!" : "is utterly annihilated!");
    if (old_perc >= 0.9f && new_perc <= 0.2f) return good_colour + name + (plural ? "almost collapse!" : "almost collapses!");
    if (old_perc >= 0.9f && new_perc <= 0.4f) return good_colour + name + (plural ? "reel from the blow!" : "reels from the blow!");
    if (!new_perc) return "";
    if (old_perc > 0.1f && new_perc <= 0.1f) return good_colour + name + (alive ? (plural ? "are very close to death!" : "is very close to death!") :
        (plural ? "are very close to collapse!" : "is very close to collapse!"));
    if (old_perc > 0.2f && new_perc <= 0.2f) return good_colour + name + (alive ? (plural ? "look badly injured!" : "looks badly injured!") :
        (plural ? "look badly damaged!" : "looks badly damaged!"));
    if (old_perc > 0.5f && new_perc <= 0.5f) return good_colour + name + (alive ? (plural ? "have a few cuts and bruises." : "has a few cuts and bruises.") :
        (plural ? "have a few scratches and dents." : "has a few scratches and dents."));
    return "";
}
