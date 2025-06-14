/*
				Copyright <SWGEmu>
		See file COPYING for copying conditions.*/

#ifndef RACES_H_
#define RACES_H_

#include "system/lang.h"

const static char* Species[] = {
	//Males
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
    "nautolan", 	// male Nautolan
	"chiss", 	// male chiss
	"devaronian", 	// male devaronian
	"gran", 	// male gran
	"ishi_tib", 	// male ishi tib
	"nikto", 	// male nikto
	"quarren", 	// male quarren
	"weequay" ,	// male weequay
	"aqualish", // male aqualish
	"bith", // male bith
	"gotal", // male gotal

	//Females
    "human", // human female
    "trandoshan", // trandoshan female
    "twilek", // twilek female
    "bothan", // bothan female
    "zabrak", // zabrak female
    "rodian", // rodian female
    "moncal", // moncal female
    "wookiee", // wookiee female
    "sullustan", // sullustan female
    "ithorian",  // DA E7   -   ithorian female
	"chiss", 	// chiss female
	"togruta", 	// togruta female
	"nightsister", 	// nightsister female
	"smc", 		// smc female
	"aqualish", // aqualish female
	"bith", // bith female
};

const static int TemplateSpecies[] = {
	// Males
    0,      // Human
    2,      // Trandoshan
    6,      // Twilek
    5,      // Bothan
    7,      // Zabrak
    1,      // Rodian
    3,      // Moncal
    4,      // Wookiee
    0x31,   // Sullustan
    0x21,   // Ithorian
    0xe4,   // Nautolan
    0xe5,   // Chiss
    0x11,   // Devaronian
    0x1c,   // gran
    0x20,   // ishi tib
    0x2a,   // nikto
    0x2e,   // quarren
    0x37,   // weequay
    9,      // aqualish
    12,     // bith
    0x1b,   // gotal
    
    // Females
    0,      // Human
    2,      // Trandoshan
    6,      // Twilek
    5,      // Bothan
    7,      // Zabrak
    1,      // Rodian
    3,      // Moncal
    4,      // Wookiee
    0x31,   // Sullustan
    0x21,   // Ithorian
    0xe5,      // Chiss
    0xe6,      // Togruta
    0xe7,      // Nightsister
    0xe8,      // SMC
    9,      // Aqualish
    0x0c,   // Bith
};

const static char* Gender[] = {
    // Males (indices 0-20)
    "male",     // human male
    "male",     // trandoshan male
    "male",     // twilek male
    "male",     // bothan male
    "male",     // zabrak male
    "male",     // rodian male
    "male",     // moncal male
    "male",     // wookiee male
    "male",     // sullustan male
    "male",     // ithorian male
    "male",     // nautolan male
    "male",     // chiss male
    "male",     // devaronian male
    "male",     // gran male
    "male",     // ishi tib male
    "male",     // nikto male
    "male",     // quarren male
    "male",     // weequay male
    "male",     // aqualish male
    "male",     // bith male
    "male",     // gotal male
    
    // Females (indices 21+)
    "female",   // human female
    "female",   // trandoshan female
    "female",   // twilek female
    "female",   // bothan female
    "female",   // zabrak female
    "female",   // rodian female
    "female",   // moncal female
    "female",   // wookiee female
    "female",   // sullustan female
    "female",   // ithorian female
    "female",   // chiss female
    "female",   // togruta female
    "female",   // nightsister female
    "female",   // smc female
    "female",   // aqualish female
    "female",   // bith female

};

const static char* RaceStrs[] = {
    // Males (indices 0-20)
    "object/creature/player/shared_human_male.iff",        // human male
    "object/creature/player/shared_trandoshan_male.iff",   // trandoshan male
    "object/creature/player/shared_twilek_male.iff",       // twilek male
    "object/creature/player/shared_bothan_male.iff",       // bothan male
    "object/creature/player/shared_zabrak_male.iff",       // zabrak male
    "object/creature/player/shared_rodian_male.iff",       // rodian male
    "object/creature/player/shared_moncal_male.iff",       // moncal male
    "object/creature/player/shared_wookiee_male.iff",      // wookiee male
    "object/creature/player/shared_sullustan_male.iff",    // sullustan male
    "object/creature/player/shared_ithorian_male.iff",     // ithorian male
    "object/creature/player/shared_nautolan_male.iff",     // nautolan male
    "object/creature/player/shared_chiss_male.iff",        // chiss male
    "object/creature/player/shared_devaronian_male.iff",   // devaronian male
    "object/creature/player/shared_gran_male.iff",         // gran male
    "object/creature/player/shared_ishi_tib_male.iff",     // ishi tib male
    "object/creature/player/shared_nikto_male.iff",        // nikto male
    "object/creature/player/shared_quarren_male.iff",      // quarren male
    "object/creature/player/shared_weequay_male.iff",      // weequay male
    "object/creature/player/shared_aqualish_male.iff",     // aqualish male
    "object/creature/player/shared_bith_male.iff",         // bith male
    "object/creature/player/shared_gotal_male.iff",        // gotal male
    
    // Females (indices 21+)
    "object/creature/player/shared_human_female.iff",      // human female
    "object/creature/player/shared_trandoshan_female.iff", // trandoshan female
    "object/creature/player/shared_twilek_female.iff",     // twilek female
    "object/creature/player/shared_bothan_female.iff",     // bothan female
    "object/creature/player/shared_zabrak_female.iff",     // zabrak female
    "object/creature/player/shared_rodian_female.iff",     // rodian female
    "object/creature/player/shared_moncal_female.iff",     // moncal female
    "object/creature/player/shared_wookiee_female.iff",    // wookiee female
    "object/creature/player/shared_sullustan_female.iff",  // sullustan female
    "object/creature/player/shared_ithorian_female.iff",   // ithorian female
    "object/creature/player/shared_chiss_female.iff",      // chiss female
    "object/creature/player/shared_togruta_female.iff",    // togruta female
    "object/creature/player/shared_nightsister_female.iff", // nightsister female
    "object/creature/player/shared_smc_female.iff",        // smc female
    "object/creature/player/shared_aqualish_female.iff",   // aqualish female
    "object/creature/player/shared_bith_female.iff",       // bith female
};

const static char* CCRaceStrs[] = {
    // Males (indices 0-20)
    "object/creature/player/human_male.iff",        // human male
    "object/creature/player/trandoshan_male.iff",   // trandoshan male
    "object/creature/player/twilek_male.iff",       // twilek male
    "object/creature/player/bothan_male.iff",       // bothan male
    "object/creature/player/zabrak_male.iff",       // zabrak male
    "object/creature/player/rodian_male.iff",       // rodian male
    "object/creature/player/moncal_male.iff",       // moncal male
    "object/creature/player/wookiee_male.iff",      // wookiee male
    "object/creature/player/sullustan_male.iff",    // sullustan male
    "object/creature/player/ithorian_male.iff",     // ithorian male
    "object/creature/player/nautolan_male.iff",     // nautolan male
    "object/creature/player/chiss_male.iff",        // chiss male
    "object/creature/player/devaronian_male.iff",   // devaronian male
    "object/creature/player/gran_male.iff",         // gran male
    "object/creature/player/ishi_tib_male.iff",     // ishi tib male
    "object/creature/player/nikto_male.iff",        // nikto male
    "object/creature/player/quarren_male.iff",      // quarren male
    "object/creature/player/weequay_male.iff",      // weequay male
    "object/creature/player/aqualish_male.iff",     // aqualish male
    "object/creature/player/bith_male.iff",         // bith male
    "object/creature/player/gotal_male.iff",        // gotal male
    
    // Females (indices 21+)
    "object/creature/player/human_female.iff",      // human female
    "object/creature/player/trandoshan_female.iff", // trandoshan female
    "object/creature/player/twilek_female.iff",     // twilek female
    "object/creature/player/bothan_female.iff",     // bothan female
    "object/creature/player/zabrak_female.iff",     // zabrak female
    "object/creature/player/rodian_female.iff",     // rodian female
    "object/creature/player/moncal_female.iff",     // moncal female
    "object/creature/player/wookiee_female.iff",    // wookiee female
    "object/creature/player/sullustan_female.iff",  // sullustan female
    "object/creature/player/ithorian_female.iff",   // ithorian female
    "object/creature/player/chiss_female.iff",      // chiss female
    "object/creature/player/togruta_female.iff",    // togruta female
    "object/creature/player/nightsister_female.iff", // nightsister female
    "object/creature/player/smc_female.iff",        // smc female
    "object/creature/player/aqualish_female.iff",   // aqualish female
    "object/creature/player/bith_female.iff",       // bith female
};

static uint32 SharedRace[] = {
    // Males (indices 0-20)
    0xAF1DC1A1,    // human male
    0x50C45B8F,    // trandoshan male
    0xF280E27B,    // twilek male
    0x5BF77F33,    // bothan male
    0xE204A556,    // zabrak male
    0x0BF9CD9C,    // rodian male
    0xB9C855A8,    // moncal male
    0x0564791D,    // wookiee male
    0x0B9399A4,    // sullustan male
    0x38BAC7C4,    // ithorian male
    0xAB03ACB1,    // nautolan male
    0x02F60EAC,    // chiss male
    0x68731058,    // devaronian male
    0xC2872D34,    // gran male
    0x809EF398,    // ishi tib male
    0x59ECB38B,    // nikto male
    0x3435F686,    // quarren male
    0x4F0718B5,    // weequay male
    0xCEF4B4A1,    // aqualish male
    0x73C992FF,    // bith male
    0x30CFFED2,    // gotal male
    
    // Females (indices 21+) - Need female equivalents
    0xFFFFBBE9,    // human female (using value from position 10 in original)
    0x183C24C6,    // trandoshan female (using value from position 11 in original)
    0x849752DC,    // twilek female (using value from position 12 in original)
    0x1D52730E,    // bothan female (using value from position 13 in original)
    0xA9E35FFD,    // zabrak female (using value from position 14 in original)
    0xC264245B,    // rodian female (using value from position 15 in original)
    0x982FBFDE,    // moncal female (using value from position 16 in original)
    0x0DAB65E2,    // wookiee female (using value from position 17 in original)
    0x1573341A,    // sullustan female (using value from position 18 in original)
    0xB3E08013,    // ithorian female (using value from position 19 in original)
    0x614A1B68,    // chiss female
    0x3D953B50,    // togruta female
    0xC56D9790,    // nightsister female
    0x82A138F9,    // smc female
    0xDE63F629,    // aqualish female
    0xB2C9CBE0,    // bith female
};

//SR2 Edit: SR-24-Add-New-Species-except-Hutt-Talz - The following If conditions were increased to account for larger race array index values.
// from 19 to 39. This is slightly padded.
class Races {
public:
	inline const static char* getRace(int raceid) {
		if (raceid < 0 || raceid > 39)
			return "";

		return RaceStrs[raceid];
	}

	inline static int getSpeciesID(int raceid) {
		if (raceid < 0 || raceid > 39)
			return 0;

		return TemplateSpecies[raceid];
	}

	inline const static char* getSpecies(int raceid) {
		if (raceid < 0 || raceid > 39)
			return "";

		return Species[raceid];
	}

	inline const static char* getGender(int raceid) {
		if (raceid < 0 || raceid > 39)
			return "";

		return Gender[raceid];
	}

	inline static uint32 getRaceCRC(int raceid) {
		if (raceid < 0 || raceid > 39)
			return 0;

		return SharedRace[raceid];
	}

	inline static const char* getCompleteRace(uint32 sharedRaceCRC) {
		int race = -1;
		for (int i = 0; i < 40; ++i) {
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
    	for (int i = 0; i < 40; i++) {
        	if (strcmp(name.toCharArray(), CCRaceStrs[i]) == 0)
            	return i;
    	}

    	return 0;
	}

};

#endif /*RACES_H_*/