//SRRaces.h
#ifndef RACES_H_
#define RACES_H_

#include "system/lang.h"

const static char* Species[] = {
    "human", // human male
    "trandoshan", // trandoshan male
    "twilek", // twilek male
    "bothan", // bothan male
    "zabrak", // zabrak male
    "rodian", // rodian male
    "moncal", // moncal male
    "wookiee", // wookiee male
    "sullustan", // sullustan male
    "ithorian", // ithorian male
    "human", // human female
    "trandoshan", // trandoshan female
    "twilek", // twilek female
    "bothan", // bothan female
    "zabrak", // zabrak female
    "rodian", // rodian female
    "moncal", // moncal female
    "wookiee", // wookiee female
    "sullustan", // sullustan female
    "ithorian", // ithorian female
    "nautolan", // male Nautolan
    "chiss", // male chiss
    "chiss", // female chiss
    "devaronian", // male devaronian
    "gran", // male gran
    "ishi_tib", // male ishi tib
    "nightsister", // female nightsister
    "nikto", // male nikto
    "quarren", // male quarren
    "smc", // female singing mountain clan witch
    "weequay", // male weequay
    "aqualish", // male aqualish
    "aqualish", // female aqualish
    "bith", // male bith
    "bith", // female bith
    "gotal", // male gotal
};

const static int TemplateSpecies[] = {
    0,    // human male
    2,    // trandoshan male  
    6,    // twilek male
    5,    // bothan male
    7,    // zabrak male
    1,    // rodian male
    3,    // moncal male
    4,    // wookiee male
    0x31, // sullustan male
    0x21, // ithorian male
    0,    // human female
    2,    // trandoshan female
    6,    // twilek female
    5,    // bothan female
    7,    // zabrak female
    1,    // rodian female
    3,    // moncal female
    4,    // wookiee female
    0x31, // sullustan female
    0x21, // ithorian female
    0x2c, // nautolan male
    0x34, // chiss male
    0x34, // chiss female
    0x35, // devaronian male
    0x36, // gran male
    0x37, // ishi tib male
    0x38, // nightsister female
    0x39, // nikto male
    0x3a, // quarren male
    0x3b, // smc female
    0x3c, // weequay male
    0x3d, // aqualish male
    0x3d, // aqualish female
    0x3e, // bith male
    0x3e, // bith female
    0x3f, // gotal male
};

const static char* Gender[] = {
    "male",   // human male
    "male",   // trandoshan male
    "male",   // twilek male
    "male",   // bothan male
    "male",   // zabrak male
    "male",   // rodian male
    "male",   // moncal male
    "male",   // wookiee male
    "male",   // sullustan male
    "male",   // ithorian male
    "female", // human female
    "female", // trandoshan female
    "female", // twilek female
    "female", // bothan female
    "female", // zabrak female
    "female", // rodian female
    "female", // moncal female
    "female", // wookiee female
    "female", // sullustan female    
    "female", // ithorian female
    "male",   // nautolan male
    "male",   // chiss male
    "female", // chiss female
    "male",   // devaronian male
    "male",   // gran male
    "male",   // ishi tib male
    "female", // nightsister female
    "male",   // nikto male
    "male",   // quarren male
    "female", // smc female
    "male",   // weequay male
    "male",   // aqualish male
    "female", // aqualish female
    "male",   // bith male
    "female", // bith female
    "male",   // gotal male
};

const static char* RaceStrs[] = {
    "object/creature/player/shared_human_male.iff",
    "object/creature/player/shared_trandoshan_male.iff",
    "object/creature/player/shared_twilek_male.iff",
    "object/creature/player/shared_bothan_male.iff",
    "object/creature/player/shared_zabrak_male.iff",
    "object/creature/player/shared_rodian_male.iff",
    "object/creature/player/shared_moncal_male.iff",
    "object/creature/player/shared_wookiee_male.iff",
    "object/creature/player/shared_sullustan_male.iff",
    "object/creature/player/shared_ithorian_male.iff",
    "object/creature/player/shared_human_female.iff",
    "object/creature/player/shared_trandoshan_female.iff",
    "object/creature/player/shared_twilek_female.iff",
    "object/creature/player/shared_bothan_female.iff",
    "object/creature/player/shared_zabrak_female.iff",
    "object/creature/player/shared_rodian_female.iff",
    "object/creature/player/shared_moncal_female.iff",
    "object/creature/player/shared_wookiee_female.iff",
    "object/creature/player/shared_sullustan_female.iff",    
    "object/creature/player/shared_ithorian_female.iff",
    "object/creature/player/shared_nautolan_male.iff",
    "object/creature/player/shared_chiss_male.iff",
    "object/creature/player/shared_chiss_female.iff",
    "object/creature/player/shared_devaronian_male.iff",
    "object/creature/player/shared_gran_male.iff",
    "object/creature/player/shared_ishi_tib_male.iff",
    "object/creature/player/shared_nightsister_female.iff",
    "object/creature/player/shared_nikto_male.iff",
    "object/creature/player/shared_quarren_male.iff",
    "object/creature/player/shared_smc_female.iff",
    "object/creature/player/shared_weequay_male.iff",
    "object/creature/player/shared_aqualish_male.iff",
    "object/creature/player/shared_aqualish_female.iff",
    "object/creature/player/shared_bith_male.iff",
    "object/creature/player/shared_bith_female.iff",
    "object/creature/player/shared_gotal_male.iff",
};

const static char* CCRaceStrs[] = {
    "object/creature/player/human_male.iff",
    "object/creature/player/trandoshan_male.iff",
    "object/creature/player/twilek_male.iff",
    "object/creature/player/bothan_male.iff",
    "object/creature/player/zabrak_male.iff",
    "object/creature/player/rodian_male.iff",
    "object/creature/player/moncal_male.iff",
    "object/creature/player/wookiee_male.iff",
    "object/creature/player/sullustan_male.iff",
    "object/creature/player/ithorian_male.iff",
    "object/creature/player/human_female.iff",
    "object/creature/player/trandoshan_female.iff",
    "object/creature/player/twilek_female.iff",
    "object/creature/player/bothan_female.iff",
    "object/creature/player/zabrak_female.iff",
    "object/creature/player/rodian_female.iff",
    "object/creature/player/moncal_female.iff",
    "object/creature/player/wookiee_female.iff",
    "object/creature/player/sullustan_female.iff",    
    "object/creature/player/ithorian_female.iff",
    "object/creature/player/nautolan_male.iff",
    "object/creature/player/chiss_male.iff",
    "object/creature/player/chiss_female.iff",
    "object/creature/player/devaronian_male.iff",
    "object/creature/player/gran_male.iff",
    "object/creature/player/ishi_tib_male.iff",
    "object/creature/player/nightsister_female.iff",
    "object/creature/player/nikto_male.iff",
    "object/creature/player/quarren_male.iff",
    "object/creature/player/smc_female.iff",
    "object/creature/player/weequay_male.iff",
    "object/creature/player/aqualish_male.iff",
    "object/creature/player/aqualish_female.iff",
    "object/creature/player/bith_male.iff",
    "object/creature/player/bith_female.iff",
    "object/creature/player/gotal_male.iff",
};

static uint32 SharedRace[] = {
    0xAF1DC1A1, // human male
    0x50C45B8F, // trandoshan male
    0xF280E27B, // twi'lek male
    0x5BF77F33, // bothan male
    0xE204A556, // zabrak male
    0x0BF9CD9C, // rodian male
    0xB9C855A8, // moncal male
    0x0564791D, // wookiee male
    0x0B9399A4, // sullustan male
    0x38BAC7C4, // ithorian male
    0xFFFFBBE9, // human female
    0x183C24C6, // trandoshan female
    0x849752DC, // twi'lek female
    0x1D52730E, // bothan female
    0xA9E35FFD, // zabrak female
    0xC264245B, // rodian female
    0x982FBFDE, // moncal female
    0x0DAB65E2, // wookiee female
    0x1573341A, // sullustan female    
    0xB3E08013, // ithorian female
    0xAB03ACB1, // nautolan male
    0x02F60EAC, // chiss male
    0x614A1B68, // chiss female
    0x68731058, // devaronian male
    0xC2872D34, // gran male
    0x809EF398, // ishi tib male
    0xC56D9790, // nightsister female
    0x59ECB38B, // nikto male
    0x3435F686, // quarren male
    0x82A138F9, // smc female
    0x4F0718B5, // weequay male
    0xCEF4B4A1, // aqualish male
    0xDE63F629, // aqualish female
    0x73C992FF, // bith male
    0xB2C9CBE0, // bith female
    0x30CFFED2, // gotal male
};

// Attribute limits based on your CSV data (36 races total)
// Format: [min_attr1, max_attr1, min_attr2, max_attr2, ... , total_ham]
static unsigned int attributeLimits[36][19] = {
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // human male
    {550, 1250, 600, 800, 700, 800, 300, 1000, 300, 450, 300, 400, 300, 1000, 300, 500, 300, 600, 5550}, // trandoshan male
    {300, 1000, 300, 500, 550, 650, 550, 1250, 600, 750, 300, 400, 400, 1100, 300, 500, 300, 500, 5400}, // twilek male
    {300, 1000, 300, 500, 300, 400, 600, 1300, 600, 750, 400, 500, 400, 1100, 400, 600, 300, 500, 5400}, // bothan male
    {500, 1200, 300, 500, 300, 400, 600, 1300, 300, 450, 300, 400, 300, 1000, 300, 500, 700, 900, 5400}, // zabrak male
    {300, 1000, 300, 500, 300, 400, 300, 1200, 300, 650, 450, 850, 300, 1000, 300, 500, 350, 550, 5400}, // rodian male
    {300, 1000, 300, 500, 300, 400, 300, 1000, 300, 450, 450, 550, 600, 1300, 600, 800, 450, 650, 5400}, // moncal male
    {650, 1350, 650, 850, 450, 550, 500, 1200, 400, 550, 400, 500, 400, 1100, 450, 650, 400, 600, 6100}, // wookiee male
    {300, 1200, 300, 500, 300, 400, 600, 1400, 300, 750, 300, 500, 400, 1200, 400, 600, 300, 600, 5400}, // sullustan male
    {300, 1400, 300, 600, 300, 500, 600, 1100, 300, 750, 300, 500, 400, 1300, 400, 600, 300, 500, 5400}, // ithorian male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // human female
    {550, 1250, 600, 800, 700, 800, 300, 1000, 300, 450, 300, 400, 300, 1000, 300, 500, 300, 600, 5550}, // trandoshan female
    {300, 1000, 300, 500, 550, 650, 550, 1250, 600, 750, 300, 400, 400, 1100, 300, 500, 300, 500, 5400}, // twilek female
    {300, 1000, 300, 500, 300, 400, 600, 1300, 600, 750, 400, 500, 400, 1100, 400, 600, 300, 500, 5400}, // bothan female
    {500, 1200, 300, 500, 300, 400, 600, 1300, 300, 450, 300, 400, 300, 1000, 300, 500, 700, 900, 5400}, // zabrak female
    {300, 1000, 300, 500, 300, 400, 300, 1200, 300, 650, 450, 850, 300, 1000, 300, 500, 350, 550, 5400}, // rodian female
    {300, 1000, 300, 500, 300, 400, 300, 1000, 300, 450, 450, 550, 600, 1300, 600, 800, 450, 650, 5400}, // moncal female
    {650, 1350, 650, 850, 450, 550, 500, 1200, 400, 550, 400, 500, 400, 1100, 450, 650, 400, 600, 6100}, // wookiee female
    {300, 1200, 300, 500, 300, 400, 600, 1400, 300, 750, 300, 500, 400, 1200, 400, 600, 300, 600, 5400}, // sullustan female
    {300, 1400, 300, 600, 300, 500, 600, 1100, 300, 750, 300, 500, 400, 1300, 400, 600, 300, 500, 5400}, // ithorian female
    {500, 1200, 300, 500, 300, 400, 600, 1300, 300, 450, 300, 400, 300, 1000, 300, 500, 700, 900, 5400}, // nautolan male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 6100}, // chiss male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 6100}, // chiss female
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // devaronian male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // gran male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // ishi tib male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 6100}, // nightsister female
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // nikto male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // quarren male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // smc female
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 6000}, // weequay male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // aqualish male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5400}, // aqualish female
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // bith male
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // bith female
    {400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 400, 1100, 5800}, // gotal male
};

class Races {
public:
    inline const static char* getRace(int raceid) {
        if (raceid < 0 || raceid >= sizeof(Species)/sizeof(Species[0]))
            return "";

        return RaceStrs[raceid];
    }

    inline static int getSpeciesID(int raceid) {
        if (raceid < 0 || raceid >= sizeof(Species)/sizeof(Species[0]))
            return 0;

        return TemplateSpecies[raceid];
    }

    inline const static char* getSpecies(int raceid) {
        if (raceid < 0 || raceid >= sizeof(Species)/sizeof(Species[0]))
            return "";

        return Species[raceid];
    }

    inline const static char* getGender(int raceid) {
        if (raceid < 0 || raceid >= sizeof(Species)/sizeof(Species[0]))
            return "";

        return Gender[raceid];
    }

    inline static uint32 getRaceCRC(int raceid) {
        if (raceid < 0 || raceid >= sizeof(Species)/sizeof(Species[0]))
            return 0;

        return SharedRace[raceid];
    }

    inline static const char* getCompleteRace(uint32 sharedRaceCRC) {
        int race = -1;
        for (int i = 0; i < sizeof(SharedRace)/sizeof(SharedRace[0]); ++i) {
            if (SharedRace[i] == sharedRaceCRC) {
                race = i;
                break;
            }
        }

        if (race == -1)
            return "";
        else
            return CCRaceStrs[race];
    }

    inline static int getRaceID(const String& name) {
        for (int i = 0; i < sizeof(Species)/sizeof(Species[0]); i++) {
            if (strcmp(name.toCharArray(), CCRaceStrs[i]) == 0)
                return i;
        }

        return 0;
    }

    inline static unsigned int * getAttribLimits(int raceid) {
        if (raceid < 0 || raceid >= sizeof(attributeLimits)/sizeof(attributeLimits[0]))
            return attributeLimits[0]; // Default to human male if invalid
            
        return attributeLimits[raceid];
    }
};

#endif /*RACES_H_*/