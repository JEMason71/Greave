// combat/combat.cpp -- Generic combat routines that apply to multiple types of combat.
// Copyright (c) 2021 Raine "Gravecat" Simmons. Licensed under the GNU Affero General Public License v3 or any later version.

#include "combat/combat.hpp"
#include "core/strx.hpp"
#include "world/mobile.hpp"


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
