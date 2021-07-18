// actions/combat.cc -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "actions/combat.h"
#include "actions/abilities.h"
#include "core/core.h"
#include "core/mathx.h"
#include "core/strx.h"

#include <cmath>


// Weapon type damage modifiers to unarmoured, light, medium and heavy armour targets.
const float Combat::DAMAGE_MODIFIER_ACID[4] =       { 1.8f, 1.3f, 1.2f, 1.0f };
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

const std::map<DamageType, const float*>    Combat::DAMAGE_TYPE_MAP = { { DamageType::ACID, DAMAGE_MODIFIER_ACID }, { DamageType::BALLISTIC, DAMAGE_MODIFIER_BALLISTIC }, { DamageType::CRUSHING, DAMAGE_MODIFIER_CRUSHING }, { DamageType::EDGED, DAMAGE_MODIFIER_EDGED }, { DamageType::ENERGY, DAMAGE_MODIFIER_ENERGY }, { DamageType::KINETIC, DAMAGE_MODIFIER_KINETIC }, { DamageType::PIERCING, DAMAGE_MODIFIER_PIERCING }, { DamageType::PLASMA, DAMAGE_MODIFIER_PLASMA }, { DamageType::POISON, DAMAGE_MODIFIER_POISON },  { DamageType::RENDING, DAMAGE_MODIFIER_RENDING }, { DamageType::EXPLOSIVE, DAMAGE_MODIFIER_EXPLOSIVE } };


// Applies damage modifiers based on weapon type.
float Combat::apply_damage_modifiers(float damage, std::shared_ptr<Item> weapon, std::shared_ptr<Mobile> defender, EquipSlot slot)
{
    if (!damage || !weapon || !defender) return damage;
    const DamageType dt = weapon->damage_type();
    const auto it = DAMAGE_TYPE_MAP.find(dt);
    if (it == DAMAGE_TYPE_MAP.end()) throw std::runtime_error("Unknown damage type: " + std::to_string(static_cast<int>(dt)));
    const float *damage_modifier = it->second;

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

// A basic attack, no special moves being used.
bool Combat::attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender)
{
    if (attacker->is_dead() || defender->is_dead()) return false;

    // We'll need to check the equipment for both attacker and defender, so we can determine how both are wielding their weapons.
    std::shared_ptr<Item> main_hand[2], off_hand[2];
    bool main_can_attack[2], off_can_attack[2];
    WieldType wield_type[2];

    for (int i = 0; i < 2; i++)
    {
        std::shared_ptr<Mobile> mob = (i == 0 ? attacker : defender);
        main_hand[i] = mob->equ()->get(EquipSlot::HAND_MAIN);
        off_hand[i] = mob->equ()->get(EquipSlot::HAND_OFF);
        determine_wield_type(mob, &wield_type[i], &main_can_attack[i], &off_can_attack[i]);
        if (i == 0 && wield_type[i] == WieldType::NONE) return false;   // Just give up here if the attacker can't attack.
    }
    if (!main_can_attack[0] && !off_can_attack[0]) return false;        // Should be impossible, but can't hurt to be safe.

    const bool unarmed_only = (wield_type[0] == WieldType::UNARMED || wield_type[0] == WieldType::UNARMED_PLUS_SHIELD);
    float attack_speed = attacker->attack_speed();
    if (attacker->tag(MobileTag::RapidStrike)) attack_speed *= (Abilities::RAPID_STRIKE_ATTACK_SPEED / 100.0f);
    if (attacker->tag(MobileTag::SnapShot)) attack_speed *= (Abilities::SNAP_SHOT_ATTACK_SPEED / 100.0f);
    if (attacker->tag(MobileTag::HeadlongStrike)) attack_speed *= (Abilities::HEADLONG_STRIKE_ATTACK_SPEED / 100.0f);

    bool attacked = false;

    // RapidStrike is only for melee weapons.
    if (attacker->tag(MobileTag::RapidStrike) || attacker->tag(MobileTag::HeadlongStrike))
    {
        if (main_hand[0] && main_hand[0]->subtype() == ItemSub::RANGED) main_can_attack[0] = false;
        if (off_hand[0] && off_hand[0]->subtype() == ItemSub::RANGED) off_can_attack[0] = false;
    }

    // Conversely, SnapShot is only for ranged weapons.
    if (attacker->tag(MobileTag::SnapShot))
    {
        if (!main_hand[0] || main_hand[0]->subtype() != ItemSub::RANGED) main_can_attack[0] = false;
        if (!off_hand[0] || off_hand[0]->subtype() != ItemSub::RANGED) off_can_attack[0] = false;
    }

    if (main_can_attack[0])
    {
        perform_attack(attacker, defender, EquipSlot::HAND_MAIN, wield_type[0], wield_type[1]);
        attacked = true;
    }
    if (off_can_attack[0] && !attacker->is_dead() && !defender->is_dead() && !unarmed_only)
    {
        perform_attack(attacker, defender, EquipSlot::HAND_OFF, wield_type[0], wield_type[1]);
        attacked = true;
    }

    if (attacker->tag(MobileTag::FreeAttack)) attacker->clear_tag(MobileTag::FreeAttack);
    else attacker->pass_time(attack_speed, false);

    attacker->clear_buff(Buff::Type::CAREFUL_AIM);
    if (attacker->tag(MobileTag::Success_EFAE))
    {
        attacker->clear_tag(MobileTag::Success_EFAE);
        attacker->clear_buff(Buff::Type::EYE_FOR_AN_EYE);
    }

    return attacked;
}

// Changes to a specified combat stance.
void Combat::change_stance(std::shared_ptr<Mobile> mob, CombatStance stance)
{
    if (stance == mob->stance() && !mob->is_player()) return;   // Do nothing if the stance doesn't change, for NPCs.
    mob->set_stance(stance);
    std::string stance_str;
    switch (stance)
    {
        case CombatStance::AGGRESSIVE: stance_str = "an {R}aggressive stance"; break;
        case CombatStance::BALANCED: stance_str = "a {G}balanced stance"; break;
        case CombatStance::DEFENSIVE: stance_str = "a {U}defensive stance"; break;
    }
    if (mob->is_player()) core()->message("{W}You assume " + stance_str + "{W}.");
    else if (mob->location() == core()->world()->player()->location() && core()->world()->get_room(mob->location())->light() >= Room::LIGHT_VISIBLE)
        core()->message("{W}" + mob->name(Mobile::NAME_FLAG_THE | Mobile::NAME_FLAG_CAPITALIZE_FIRST) + " {W}assumes " + stance_str + "{W}!");
    if (mob->has_buff(Buff::Type::CAREFUL_AIM) && stance == CombatStance::AGGRESSIVE) mob->clear_buff(Buff::Type::CAREFUL_AIM);
    if (mob->has_buff(Buff::Type::EYE_FOR_AN_EYE) && stance != CombatStance::AGGRESSIVE) mob->clear_buff(Buff::Type::EYE_FOR_AN_EYE);
    if (mob->has_buff(Buff::Type::GRIT) && stance != CombatStance::DEFENSIVE) mob->clear_buff(Buff::Type::GRIT);
    mob->pass_time(STANCE_CHANGE_TIME, false);
}

// Generates a standard-format damage number string.
std::string Combat::damage_number_str(uint32_t damage, uint32_t blocked, bool crit, bool bleed, bool poison)
{
    std::string dmg_str;
    if (crit) dmg_str = "{w}[{m}*{M}"; else dmg_str = "{w}[{R}";
    if (damage > 0) dmg_str += "-";
    dmg_str += StrX::intostr_pretty(damage);
    if (bleed && !crit) dmg_str += "B";
    if (poison && !crit) dmg_str += "P";
    if (crit) dmg_str += "{m}*";
    dmg_str += "{w}]";
    if (blocked > 0) dmg_str += " {w}<{U}" + StrX::intostr_pretty(blocked) + "{w}>";
    return dmg_str;
}

// Returns an appropriate damage string.
std::string Combat::damage_str(uint32_t damage, std::shared_ptr<Mobile> def, bool heat)
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
        if (percentage >= 15) return "{Y}scorches";
        else if (percentage >= 10) return "{Y}chars";
        else if (percentage >= 5) return "{y}sears";
        else if (percentage >= 1) return "{y}scalds";
        else return "{y}singes";
    }
    else
    {
        if (percentage >= 15) return "{Y}nicks";
        else if (percentage >= 10) return "{Y}grazes";
        else if (percentage >= 5) return "{y}scratches";
        else if (percentage >= 1) return "{y}bruises";
        else return "{y}tickles";
    }
}

// Determines type of weapons wielded by a Mobile.
void Combat::determine_wield_type(std::shared_ptr<Mobile> mob, WieldType* wield_type, bool* can_main_attack, bool* can_off_attack)
{
    std::shared_ptr<Item> main_hand = mob->equ()->get(EquipSlot::HAND_MAIN);
    std::shared_ptr<Item> off_hand = mob->equ()->get(EquipSlot::HAND_OFF);
    *can_main_attack = main_hand && main_hand->type() == ItemType::WEAPON;
    *can_off_attack = off_hand && off_hand->type() == ItemType::WEAPON;
    const bool off_shield = (off_hand && off_hand->type() == ItemType::SHIELD);

    // If both hands are empty, it's a melee attack.
    if (!main_hand && !off_hand)
    {
        *wield_type = WieldType::UNARMED;
        *can_main_attack = true;
        *can_off_attack = true;
    }

    // Dual-wielding is an easy one to detect. One melee weapon in each hand.
    else if (*can_main_attack && *can_off_attack) *wield_type = WieldType::DUAL_WIELD;

    // Good old sword and board: melee weapon in one hand, shield in the other.
    else if (*can_main_attack && off_shield) *wield_type = WieldType::ONE_HAND_PLUS_SHIELD;

    // Two-handers can only be equipped in the main hand.
    else if (*can_main_attack && main_hand->tag(ItemTag::TwoHanded)) *wield_type = WieldType::TWO_HAND;

    // Single-wielding a one-handed weapon isn't the best choice, but it can be done.
    else if ((*can_main_attack && !off_hand) || (*can_off_attack && !main_hand))
    {
        // Check if we're using a hand-and-a-half weapon with the other hand free, or just a regular one-hander on its own.
        if ((*can_main_attack && main_hand->tag(ItemTag::HandAndAHalf)) || (*can_off_attack && off_hand->tag(ItemTag::HandAndAHalf))) *wield_type = WieldType::HAND_AND_A_HALF_2H;
        else *wield_type = WieldType::SINGLE_WIELD;
    }

    // We've already checked for sword-and-board above, so the only option left if one hand is holding a weapon is that the other hand is holding something
    // non-combat related. Yay for the process of elimination!
    else if (*can_main_attack || *can_off_attack) *wield_type = WieldType::ONE_HAND_PLUS_EXTRA;

    // Now we're getting into the silly options, but gotta cover every base. Is the Mobile wielding a shield in one hand, and nothing in the other?
    // As ridiculous as that is for a loadout, punching while holding a shield should be allowed.
    else if (off_shield && !main_hand) *wield_type = WieldType::UNARMED_PLUS_SHIELD;

    // If either hand is now free, by process of elimination, it must be an unarmed attack with a non-combat item in the other hand.
    else if (!main_hand || !off_hand)
    {
        *wield_type = WieldType::UNARMED;
        if (!main_hand) *can_main_attack = true;
        if (!off_hand) *can_off_attack = true;
    }

    // The only other possible configurations, through process of elimination, is shield+misc:
    else if (off_shield) *wield_type = WieldType::SHIELD_ONLY;

    // ...or the final option, which is the only thing that remains now, both hands occupied by non-combat items:
    else *wield_type = WieldType::NONE;
}

// Performs an attack with a single weapon.
void Combat::perform_attack(std::shared_ptr<Mobile> attacker, std::shared_ptr<Mobile> defender, EquipSlot weapon, WieldType wield_type_attacker, WieldType wield_type_defender)
{
    const auto world = core()->world();
    const auto player = world->player();
    const auto rng = core()->rng();
    if (defender->is_player())
    {
        // If the player does not yet have an automatic mob target, set it now.
        if (!player->mob_target()) player->set_mob_target(attacker->id());
    }
    else defender->add_hostility(attacker->id());   // Only do this on NON-player defenders, because obviously the player doesn't use the hostility vector.

    std::shared_ptr<Item> weapon_ptr = attacker->equ()->get(weapon);
    if (!weapon_ptr) weapon_ptr = world->get_item("UNARMED_ATTACK");
    const std::shared_ptr<Item> def_weapon_main = defender->equ()->get(EquipSlot::HAND_MAIN);
    const std::shared_ptr<Item> def_weapon_off = defender->equ()->get(EquipSlot::HAND_OFF);
    const bool ranged_attack = (weapon_ptr->subtype() == ItemSub::RANGED);
    const bool attacker_is_player = attacker->is_player();
    const bool defender_is_player = defender->is_player();
    const bool no_ammo = (ranged_attack && weapon_ptr->tag(ItemTag::NoAmmo));
    const bool eye_for_an_eye = (attacker->has_buff(Buff::Type::EYE_FOR_AN_EYE) && !ranged_attack);
    const bool snake_eyes = (defender->tag(MobileTag::SnakeEyes));
    const bool boxcars = (attacker->tag(MobileTag::Boxcars));

    const size_t ammo_pos = (ranged_attack ? attacker->inv()->ammo_pos(weapon_ptr) : SIZE_MAX);
    if (ranged_attack && !no_ammo && ammo_pos == SIZE_MAX)
    {
        if (attacker_is_player)
        {
            std::string ammo_name = "ammunition";
            if (weapon_ptr->tag(ItemTag::AmmoArrow)) ammo_name = "arrows";
            else if (weapon_ptr->tag(ItemTag::AmmoBolt)) ammo_name = "bolts";
            core()->message("{y}You do not have any {Y}" + ammo_name + " {y}to fire your " + weapon_ptr->name() + "{y}!");
        }
        return;
    }
    const std::shared_ptr<Item> ammo_ptr = (ranged_attack && !no_ammo ? attacker->inv()->get(ammo_pos) : nullptr);

    const bool player_is_here = (player->location() == attacker->location());
    const bool is_dark_here = world->get_room(attacker->location())->light() < Room::LIGHT_VISIBLE;
    const bool player_can_see_attacker = attacker_is_player || !is_dark_here;
    const bool player_can_see_defender = defender_is_player || !is_dark_here;
    const bool defender_melee = (wield_type_defender != WieldType::UNARMED && wield_type_defender != WieldType::UNARMED_PLUS_SHIELD) && ((def_weapon_main && def_weapon_main->type() == ItemType::WEAPON && def_weapon_main->subtype() == ItemSub::MELEE) || (def_weapon_off && def_weapon_off->type() == ItemType::WEAPON && def_weapon_off->subtype() == ItemSub::MELEE));

    const std::string attacker_name = (attacker_is_player ? "you" : (player_can_see_attacker ? attacker->name(Mobile::NAME_FLAG_THE) : "something"));
    const std::string defender_name = (defender_is_player ? "you" : (player_can_see_defender ? defender->name(Mobile::NAME_FLAG_THE) : "something"));
    const std::string defender_name_c = StrX::capitalize_first_letter(defender_name);
    const std::string defender_name_s = (defender_is_player ? "your" : StrX::possessive_string(defender_name));
    const std::string defender_your_string = (defender_is_player ? "your" : defender->his_her());
    const std::string defender_your_string_c = StrX::capitalize_first_letter(defender_your_string);
    const std::string attacker_your_string = (attacker_is_player ? "your" : StrX::possessive_string(attacker_name));
    const std::string attacker_your_string_c = StrX::capitalize_first_letter(attacker_your_string);
    const std::string weapon_name = (ammo_ptr ? ammo_ptr->name(Item::NAME_FLAG_NO_COUNT) : weapon_ptr->name());

    const CombatStance attacker_stance = attacker->stance();
    const CombatStance defender_stance = defender->stance();

    std::string weapon_skill;   // If the player is involved in this fight, they will be using combat skills.
    if (attacker_is_player || defender_is_player)
    {
        const WieldType wt = (attacker_is_player ? wield_type_attacker : wield_type_defender);
        switch (wt)
        {
            case WieldType::NONE: case WieldType::SHIELD_ONLY: case WieldType::UNARMED: case WieldType::UNARMED_PLUS_SHIELD: weapon_skill = "UNARMED"; break;
            case WieldType::DUAL_WIELD: weapon_skill = "DUAL_WIELD"; break;
            case WieldType::ONE_HAND_PLUS_EXTRA: case WieldType::ONE_HAND_PLUS_SHIELD: case WieldType::SINGLE_WIELD: weapon_skill = "ONE_HANDED"; break;
            case WieldType::TWO_HAND: case WieldType::HAND_AND_A_HALF_2H: weapon_skill = "TWO_HANDED"; break;
        }
        if (weapon_ptr->subtype() == ItemSub::RANGED) weapon_skill = "ARCHERY";
    }

    // Roll to hit!
    float hit_multiplier = 1.0f;
    switch (wield_type_attacker)
    {
        case WieldType::DUAL_WIELD: hit_multiplier = HIT_CHANCE_MULTIPLIER_DUAL_WIELD; break;
        case WieldType::SINGLE_WIELD: hit_multiplier = HIT_CHANCE_MULTIPLIER_SINGLE_WIELD; break;
        case WieldType::ONE_HAND_PLUS_SHIELD: case WieldType::UNARMED_PLUS_SHIELD: case WieldType::ONE_HAND_PLUS_EXTRA: hit_multiplier = HIT_CHANCE_MULTIPLIER_SWORD_AND_BOARD; break;
        default: break;
    }
    float to_hit = BASE_HIT_CHANCE_MELEE;
    if (attacker->has_buff(Buff::Type::CAREFUL_AIM)) to_hit += attacker->buff_power(Buff::Type::CAREFUL_AIM);
    if (attacker->tag(MobileTag::RapidStrike)) to_hit -= Abilities::RAPID_STRIKE_ACCURACY_PENALTY;
    if (attacker->tag(MobileTag::SnapShot)) to_hit -= Abilities::SNAP_SHOT_ACCURACY_PENALTY;
    if (defender->has_buff(Buff::Type::QUICK_ROLL))
    {
        to_hit -= defender->buff_power(Buff::Type::QUICK_ROLL);
        defender->set_tag(MobileTag::Success_QuickRoll);
    }
    if (attacker_is_player) to_hit += (WEAPON_SKILL_TO_HIT_PER_LEVEL * player->skill_level(weapon_skill));
    else if (defender_is_player) to_hit -= (EVASION_SKILL_BONUS_PER_LEVEL * player->skill_level("EVASION"));
    to_hit *= hit_multiplier;

    // Check if the defender can attempt to block or parry.
    bool can_block = (wield_type_defender == WieldType::ONE_HAND_PLUS_SHIELD || wield_type_defender == WieldType::SHIELD_ONLY || wield_type_defender == WieldType::UNARMED_PLUS_SHIELD) && !defender->tag(MobileTag::CannotBlock);
    bool can_parry = wield_type_defender != WieldType::UNARMED && wield_type_defender != WieldType::SHIELD_ONLY && wield_type_defender != WieldType::UNARMED_PLUS_SHIELD && defender_melee && !defender->tag(MobileTag::CannotParry);
    if ((def_weapon_main && def_weapon_main->subtype() == ItemSub::RANGED) || (def_weapon_off && def_weapon_off->subtype() == ItemSub::RANGED) || (weapon_ptr->subtype() == ItemSub::RANGED)) can_parry = false;

    // Check for Agile or Clumsy defender.
    if (defender->tag(MobileTag::Agile)) to_hit *= DEFENDER_TO_HIT_MODIFIER_AGILE;
    else if (defender->tag(MobileTag::Clumsy)) to_hit *= DEFENDER_TO_HIT_MODIFIER_CLUMSY;

    // Adjust to-hit chance based on stance matchup.
    const int stance_favour = stance_compare(attacker_stance, defender_stance);
    if (stance_favour > 0) to_hit += STANCE_TO_HIT_MODIFIER_FAVOURABLE;
    else if (stance_favour < 0) to_hit += STANCE_TO_HIT_MODIFIER_UNFAVOURABLE;

    // Defenders that cannot dodge always get hit.
    if (defender->tag(MobileTag::CannotDodge)) to_hit = 100;
    else to_hit *= defender->dodge_mod();

    // Check for the Eye for an Eye, Snake Eyes or Boxcars buffs.
    if (eye_for_an_eye || snake_eyes || boxcars)
    {
        to_hit = 100;
        can_block = can_parry = false;
        if (eye_for_an_eye) attacker->set_tag(MobileTag::Success_EFAE);
    }

    bool evaded = false, blocked = false, parried = false;
    if (rng->frnd(100) <= to_hit)  // Evasion failed; the target was hit.
    {
        // Now to check if the defender can successfully parry this attack.
        if (can_parry)
        {
            float parry_chance = BASE_PARRY_CHANCE;
            if (wield_type_attacker == WieldType::TWO_HAND || wield_type_attacker == WieldType::HAND_AND_A_HALF_2H) parry_chance *= PARRY_PENALTY_TWO_HANDED;
            if (defender_is_player) parry_chance += (PARRY_SKILL_BONUS_PER_LEVEL * player->skill_level("PARRY"));
            parry_chance *= defender->parry_mod();
            if (defender->tag(MobileTag::Agile) || attacker->tag(MobileTag::Clumsy)) parry_chance *= DEFENDER_PARRY_MODIFIER_AGILE;
            else if (defender->tag(MobileTag::Clumsy) || attacker->tag(MobileTag::Agile)) parry_chance *= DEFENDER_PARRY_MODIFIER_CLUMSY;
            if (rng->frnd(100) <= parry_chance) parried = true;
        }

        // If evasion and parry both fail, we can try to block. Parrying is better than blocking (parrying negates damage entirely) so there's no reason to run a block check after a successful parry.
        if (!parried && can_block)
        {
            float block_chance = BASE_BLOCK_CHANCE_MELEE;
            if (defender_is_player) block_chance += (BLOCK_SKILL_BONUS_PER_LEVEL * player->skill_level("BLOCK"));
            if (defender->has_buff(Buff::Type::SHIELD_WALL))
            {
                block_chance += defender->buff_power(Buff::Type::SHIELD_WALL);
                defender->set_tag(MobileTag::Success_ShieldWall);
            }
            block_chance *= defender->block_mod();
            if (rng->frnd(100) <= block_chance) blocked = true;
        }
    }
    else evaded = true;

    // Determine where the defender was hit.
    EquipSlot def_location_hit_es;
    std::string def_location_hit_str;
    pick_hit_location(defender, &def_location_hit_es, &def_location_hit_str);

    if (parried || evaded)
    {
        if (parried)
        {
            if (player_can_see_attacker || player_can_see_defender)
            {
                if (defender_is_player) core()->message("{G}You parry the " + attacker_your_string + " " + weapon_name + "!");
                else core()->message((attacker_is_player ? "{Y}" : "{U}") + attacker_your_string_c + " " + weapon_name + " is parried by " + defender_name + ".");
            }
            if (defender_is_player) player->gain_skill_xp("PARRY", XP_PER_PARRY);
        }
        else
        {
            if (player_can_see_attacker || player_can_see_defender)
                core()->message((attacker_is_player ? "{Y}" : (defender_is_player ? "{U}" : "{U}")) + attacker_your_string_c + " " + weapon_name + " misses " + defender_name + ".");
            if (defender_is_player) player->gain_skill_xp("EVASION", XP_PER_EVADE);
        }
    }
    else
    {
        float damage = weapon_ptr->power() * BASE_MELEE_DAMAGE_MULTIPLIER;
        if (ammo_ptr) damage *= ammo_ptr->ammo_power();
        if (attacker_is_player) damage += (damage * (WEAPON_SKILL_DAMAGE_MODIFIER * player->skill_level(weapon_skill)));
        switch (attacker_stance)
        {
            case CombatStance::AGGRESSIVE: damage *= STANCE_DAMAGE_MULTIPLIER_AGGRESSIVE; break;
            case CombatStance::DEFENSIVE: damage *= STANCE_DAMAGE_MULTIPLIER_DEFENSIVE; break;
            case CombatStance::BALANCED: break;
        }
        switch (defender_stance)
        {
            case CombatStance::AGGRESSIVE: damage *= STANCE_DAMAGE_TAKEN_MULTIPLIER_AGGRESSIVE; break;
            case CombatStance::DEFENSIVE: damage *= STANCE_DAMAGE_TAKEN_MULTIPLIER_DEFENSIVE; break;
            case CombatStance::BALANCED: break;
        }
        if (wield_type_attacker == WieldType::HAND_AND_A_HALF_2H) damage *= WEAPON_DAMAGE_MODIFIER_HAAH_2H;

        bool critical_hit = false, bleed = false, poison = false;
        float crit_chance = weapon_ptr->crit();
        if (wield_type_attacker == WieldType::SINGLE_WIELD) crit_chance *= CRIT_CHANCE_MULTIPLIER_SINGLE_WIELD;
        if (snake_eyes || boxcars) crit_chance = 100;
        if (crit_chance >= 100.0f || rng->frnd(100) <= crit_chance)
        {
            critical_hit = true;
            bleed = true;
            damage *= 3;
        }
        const float poison_chance = weapon_ptr->poison() + (ammo_ptr ? ammo_ptr->poison() : 0);
        const float bleed_chance = weapon_ptr->bleed() + (ammo_ptr ? ammo_ptr->bleed() : 0);
        if (poison_chance >= 100.0f || rng->frnd(100) <= poison_chance) poison = true;
        if (bleed_chance >= 100.0f || rng->frnd(100) <= bleed_chance) bleed = true;

        if (attacker->tag(MobileTag::Anemic)) damage *= ATTACKER_DAMAGE_MULTIPLIER_ANEMIC;
        else if (attacker->tag(MobileTag::Feeble)) damage *= ATTACKER_DAMAGE_MULTIPLIER_FEEBLE;
        else if (attacker->tag(MobileTag::Puny)) damage *= ATTACKER_DAMAGE_MULTIPLIER_PUNY;
        else if (attacker->tag(MobileTag::Strong)) damage *= ATTACKER_DAMAGE_MULTIPLIER_STRONG;
        else if (attacker->tag(MobileTag::Brawny)) damage *= ATTACKER_DAMAGE_MULTIPLIER_BRAWNY;
        else if (attacker->tag(MobileTag::Vigorous)) damage *= ATTACKER_DAMAGE_MULTIPLIER_VIGOROUS;
        else if (attacker->tag(MobileTag::Mighty)) damage *= ATTACKER_DAMAGE_MULTIPLIER_MIGHTY;

        // Bonus damage for Eye for an Eye.
        if (eye_for_an_eye)
        {
            const float bonus = (1.0f - (attacker->hp() / attacker->hp(true))) * attacker->buff_power(Buff::Type::EYE_FOR_AN_EYE);
            damage *= bonus;
        }

        if (defender->tag(MobileTag::ImmunityBleed)) bleed = false;
        if (defender->tag(MobileTag::ImmunityPoison)) poison = false;

        float damage_blocked = 0;
        if (def_location_hit_es == EquipSlot::BODY)
        {
            const std::shared_ptr<Item> body_armour = defender->equ()->get(EquipSlot::BODY);
            const std::shared_ptr<Item> outer_armour = defender->equ()->get(EquipSlot::ARMOUR);
            const EquipSlot outer_layer = (outer_armour ? EquipSlot::ARMOUR : EquipSlot::BODY);
            if (body_armour && outer_armour) damage_blocked = damage * body_armour->armour(outer_armour->power());
            else if (body_armour) damage_blocked = damage * body_armour->armour();
            else if (outer_armour) damage_blocked = damage * outer_armour->armour();
            damage_blocked = apply_damage_modifiers(damage_blocked, (ammo_ptr ? ammo_ptr : weapon_ptr), defender, outer_layer);
        }
        else
        {
            EquipSlot hit_loc = def_location_hit_es;
            if (defender->tag(MobileTag::Beast)) hit_loc = EquipSlot::BODY;
            const std::shared_ptr<Item> armour_piece_hit = defender->equ()->get(hit_loc);
            if (armour_piece_hit) damage_blocked = damage * armour_piece_hit->armour();
            damage_blocked = apply_damage_modifiers(damage_blocked, (ammo_ptr ? ammo_ptr : weapon_ptr), defender, hit_loc);
        }

        // Reduced damage for Grit.
        if (defender->has_buff(Buff::Type::GRIT) && damage >= 1.0f)
        {
            const float grit_power = defender->buff_power(Buff::Type::GRIT);
            const float damage_reduced = std::min(damage, damage * (grit_power / 100.0f));
            if (damage_reduced >= 1.0f)
            {
                damage -= damage_reduced;
                damage_blocked += damage_reduced;
                player->set_tag(MobileTag::Success_Grit);
            }
        }

        if (blocked)
        {
            const std::shared_ptr<Item> shield_item = defender->equ()->get(EquipSlot::HAND_OFF);
            if (shield_item) damage_blocked += damage * shield_item->armour();
        }

        if (damage > 1) damage = MathX::mixup(std::round(damage), BASE_DAMAGE_VARIANCE);
        else if (damage > 0) damage = 1;
        if (damage_blocked > 1) damage_blocked = MathX::mixup(std::round(damage_blocked), BASE_ABSORPTION_VARIANCE);
        else if (damage_blocked > 0) damage_blocked = 1;
        if (damage_blocked >= damage) damage_blocked = damage;
        damage -= damage_blocked;

        const bool fatal = (damage >= defender->hp());
        if (player_is_here && (player_can_see_attacker || player_can_see_defender))
        {
            std::string damage_word = damage_str(damage, defender, false);
            std::string threshold_string = threshold_str(defender, damage, (attacker_is_player ? "{G}" : (defender_is_player ? "{R}" : "{U}")), (defender_is_player ? "{Y}" : (attacker_is_player ? "{y}" : "{U}")));
            std::string damage_colour = (attacker_is_player ? (damage > 0.0f ? "{G}" : "{y}") : (defender_is_player ? (damage > 0.0f ? "{R}" : "{Y}") : "{U}"));
            std::string absorb_str, block_str, death_str;
            if (damage_blocked)
            {
                std::shared_ptr<Item> armour_piece_hit = defender->equ()->get(blocked ? EquipSlot::HAND_OFF : (defender->tag(MobileTag::Beast) ? EquipSlot::BODY : def_location_hit_es));
                if (def_location_hit_es == EquipSlot::BODY && !blocked && defender->equ()->get(EquipSlot::ARMOUR))
                    armour_piece_hit = defender->equ()->get(EquipSlot::ARMOUR);
                std::string lessens_str, lessens_plural_str, lessening_str;
                if (damage < 1.0f)
                {
                    lessens_str = "absorbs";
                    lessens_plural_str = "absorb";
                    lessening_str = "absorbing";
                }
                else switch (rng->rnd(10))
                {
                    case 1: lessens_str = "mitigates"; lessens_plural_str = "mitigate"; lessening_str = "mitigating"; break;
                    case 2: lessens_str = "diminishes"; lessens_plural_str = "diminish"; lessening_str = "diminishing"; break;
                    case 3: lessens_str = "alleviates"; lessens_plural_str = "alleviate"; lessening_str = "alleviating"; break;
                    case 4: lessens_str = "deadens"; lessens_plural_str = "deaden"; lessening_str = "deadening"; break;
                    case 5: lessens_str = "dampens"; lessens_plural_str = "dampen"; lessening_str = "dampening"; break;
                    case 6: lessens_str = "dulls"; lessens_plural_str = "dull"; lessening_str = "dulling"; break;
                    case 7: lessens_str = "lessens"; lessens_plural_str = "lessen"; lessening_str = "lessening"; break;
                    case 8: lessens_str = "withstands"; lessens_plural_str = "withstand"; lessening_str = "withstanding"; break;
                    case 9: lessens_str = "endures"; lessens_plural_str = "endure"; lessening_str = "enduring"; break;
                    case 10: lessens_str = "takes"; lessens_plural_str = "take"; lessening_str = "taking"; break;
                }
                if (armour_piece_hit->tag(ItemTag::PluralName)) lessens_str = lessens_plural_str;
                if (blocked)
                {
                    const std::string blocks_str = (defender_is_player ? "block" : "blocks");
                    block_str = "{U}" + defender_name_c + " " + blocks_str + " with " + defender_your_string + " " + armour_piece_hit->name() + ", " + lessening_str + " the blow. ";
                }
                else absorb_str = " {U}" + defender_your_string_c + " " + armour_piece_hit->name() + " " + lessens_str + " the blow.";
            }

            if (fatal)
            {
                if (defender_is_player)
                {
                    death_str = " {M}You are slain!";
                    player->set_death_reason("slain by " + attacker->name(Mobile::NAME_FLAG_A | Mobile::NAME_FLAG_NO_COLOUR));
                }
                else death_str = " {U}" + defender_name_c + (defender->tag(MobileTag::Unliving) ? " is destroyed!" : " is slain!");
                if (!defender->tag(MobileTag::ImmunityBleed)) core()->world()->get_room(defender->location())->add_scar(ScarType::BLOOD, SCAR_BLEED_INTENSITY_FROM_DEATH);
            }
            core()->message(block_str + damage_colour + attacker_your_string_c + " " + weapon_name + " " + damage_word + " " + damage_colour + (blocked ? defender_name : defender_name_s + " " + def_location_hit_str) + "!" + threshold_string + absorb_str + " " + damage_number_str(damage, damage_blocked, critical_hit, bleed, poison) + death_str);
        }
        if (bleed) weapon_bleed_effect(defender, damage);
        if (poison) weapon_poison_effect(defender, damage);
        defender->reduce_hp(damage, false);
        if (attacker_is_player) player->gain_skill_xp(weapon_skill, (critical_hit ? XP_PER_CRITICAL_HIT : XP_PER_SUCCESSFUL_HIT));
        else if (defender_is_player && blocked) player->gain_skill_xp("BLOCK", XP_PER_BLOCK);
    }

    // Remove ammo if we're using a ranged weapon.
    if (!no_ammo && ammo_ptr && attacker_is_player)
    {
        if (ammo_ptr->stack() > 1) ammo_ptr->set_stack(ammo_ptr->stack() - 1);
        else
        {
            core()->message("{m}You have fired the last of your " + ammo_ptr->name(Item::NAME_FLAG_PLURAL) + ".");
            attacker->inv()->erase(ammo_pos);
        }
    }
}

// Picks a random hit location, returns an EquipSlot and the name of the anatomy part that was hit.
void Combat::pick_hit_location(std::shared_ptr<Mobile> mob, EquipSlot* slot, std::string* slot_name)
{
    const auto body_parts = mob->get_anatomy();
    const int bp_roll = core()->rng()->rnd(100);
    *slot = EquipSlot::NONE;
    for (size_t i = 0; i < body_parts.size(); i++)
    {
        if (bp_roll < body_parts.at(i)->hit_chance) continue;
        *slot = body_parts.at(i)->slot;
        *slot_name = body_parts.at(i)->name;
        break;
    }
    if (*slot == EquipSlot::NONE) throw std::runtime_error("Could not determine body hit location for " + mob->name());
}

// Compares two combat stances; returns -1 for an unfavourable match-up, 0 for neutral, 1 for favourable.
int Combat::stance_compare(CombatStance atk, CombatStance def)
{
    switch (atk)
    {
        case CombatStance::AGGRESSIVE:
            if (def == CombatStance::DEFENSIVE) return 1;
            else if (def == CombatStance::BALANCED) return -1;
            break;
        case CombatStance::BALANCED:
            if (def == CombatStance::AGGRESSIVE) return 1;
            else if (def == CombatStance::DEFENSIVE) return -1;
            break;
        case CombatStance::DEFENSIVE:
            if (def == CombatStance::BALANCED) return 1;
            else if (def == CombatStance::AGGRESSIVE) return -1;
            break;
    }
    return 0;
}

// Returns a threshold string, if a damage threshold has been passed.
std::string Combat::threshold_str(std::shared_ptr<Mobile> defender, uint32_t damage, const std::string& good_colour, const std::string& bad_colour)
{
    const bool is_player = defender->is_player();
    const bool alive = !defender->tag(MobileTag::Unliving);
    const bool plural = defender->tag(MobileTag::PluralName) || is_player;
    const std::string name = (is_player ? " You " : (plural ? " They " : " " + StrX::capitalize_first_letter(defender->he_she()) + " "));

    const float old_perc = defender->hp() / static_cast<float>(defender->hp(true));
    float new_perc = (defender->hp() - damage) / static_cast<float>(defender->hp(true));
    if (defender->hp() <= static_cast<int32_t>(damage)) new_perc = 0;

    if (old_perc >= 0.99f && new_perc >= 0.95f) return bad_colour + name + (alive ? (plural ? "barely notice." : "barely notices.") : (plural ? "are barely scratched." : "is barely scratched."));
    if (old_perc >= 0.99f && new_perc >= 0.95f) return bad_colour + name + (alive ? (plural ? "barely notice." : "barely notices.") : (plural ? "are barely scratched." : "is barely scratched."));
    if (old_perc >= 0.95f && new_perc >= 0.90f) return bad_colour + name + (alive ? (plural ? "shrug it off." : "shrugs it off.") : (plural ? "are hardly damaged." : "is hardly damaged."));
    if (old_perc >= 0.9f && new_perc == 0.0f) return good_colour + name + (plural ? "are utterly annihilated!" : "is utterly annihilated!");
    if (old_perc >= 0.9f && new_perc <= 0.2f) return good_colour + name + (plural ? "almost collapse!" : "almost collapses!");
    if (old_perc >= 0.9f && new_perc <= 0.4f) return good_colour + name + (plural ? "reel from the blow!" : "reels from the blow!");
    if (!new_perc) return "";
    if (old_perc > 0.1f && new_perc <= 0.1f) return good_colour + name + (alive ? (plural ? "are very close to death!" : "is very close to death!") : (plural ? "are very close to collapse!" : "is very close to collapse!"));
    if (old_perc > 0.2f && new_perc <= 0.2f) return good_colour + name + (alive ? (plural ? "look badly injured!" : "looks badly injured!") : (plural ? "look badly damaged!" : "looks badly damaged!"));
    if (old_perc > 0.5f && new_perc <= 0.5f) return good_colour + name + (alive ? (plural ? "have a few cuts and bruises." : "has a few cuts and bruises.") : (plural ? "have a few scratches and dents." : "has a few scratches and dents."));
    return "";
}

// Applies a weapon bleed debuff and applies room scars.
void Combat::weapon_bleed_effect(std::shared_ptr<Mobile> defender, uint32_t damage)
{
    if (defender->tag(MobileTag::ImmunityBleed)) return;
    const int bleed_time = core()->rng()->rnd(BLEED_TIME_RANGE);
    int bleed_severity = damage / (BLEED_SEVERITY_BASE + core()->rng()->rnd(BLEED_SEVERITY_RANGE));
    if (!bleed_severity) bleed_severity = 1;
    defender->set_buff(Buff::Type::BLEED, bleed_time, bleed_severity, false);
    core()->world()->get_room(defender->location())->add_scar(ScarType::BLOOD, SCAR_BLEED_INTENSITY_FROM_BLEED_ATTACK);
}

// Applies a weapon poison debuff.
void Combat::weapon_poison_effect(std::shared_ptr<Mobile> defender, uint32_t damage)
{
    if (defender->tag(MobileTag::ImmunityPoison)) return;
    const int poison_time = core()->rng()->rnd(POISON_TIME_RANGE);
    int poison_severity = damage / (POISON_SEVERITY_BASE + core()->rng()->rnd(POISON_SEVERITY_RANGE));
    if (!poison_severity) poison_severity = 1;
    defender->set_buff(Buff::Type::POISON, poison_time, poison_severity, true);
}
