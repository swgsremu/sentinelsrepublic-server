includeFile("default.lua")
includeFile("escort.lua")
includeFile("attackableSpaceStation.lua")
includeFile("spaceStations.lua")
includeFile("turretship.lua")
includeFile("waveAttack.lua")

customMap = {
	{ "spaceStations", {
		{NONESPACE, "rootSpacestations"}
	}},
}

bitmaskLookup = {
	{NONE, {
		{NONESPACE, "rootDefault"},
		{AWARESPACE, "awareDefault"},
		{EVADESPACE, "evadeDefault"},
		{IDLESPACE, "idleDefault"},
		{ATTACKSPACE, "attackDefault"},
		{TARGETSPACE, "targetDefault"},
		{MOVESPACE, "moveDefault"},
		{LOOKATSPACE, "lookDefault"},
		{AGGROSPACE, "aggroDefault"}
	}},

	{ESCORT, {
		{NONESPACE, "rootEscort"},
		{IDLESPACE, "idleEscort"},
		{MOVESPACE, "moveEscort"},
		{TARGETSPACE, "targetEscort"},
	}},

	{TURRETSHIP, {
		{ATTACKSPACE, "attackTurretship"},
	}},

	{WAVE_ATTACK, {
		{MOVESPACE, "moveWaveattack"},
	}},

	{ATTACKABLE_SPACE_STATION, {
		{IDLESPACE, "idleAttackablespacestation"},
	}},

	{TEST, {
		{AWARESPACE, "failTest"},
		{IDLESPACE, "succeedTest"},
		{ATTACKSPACE, "failTest"},
		{TARGETSPACE, "failTest"},
		{MOVESPACE, "failTest"},
		{LOOKATSPACE, "failTest"},
		{AGGROSPACE, "failTest"}
	}}
}
