#ifndef GAME_SERVER_UPGRADELIST_H
#define GAME_SERVER_UPGRADELIST_H

#include <cstring>
#include <game/generated/protocol.h>

#define MEDIC_ABILITY_COOLDOWN 600
#define COMMANDO_ABILITY_COOLDOWN 600
#define ENGINEER_ABILITY_COOLDOWN 400
#define PIONEER_ABILITY_COOLDOWN 400
#define BERSERKER_ABILITY_COOLDOWN 800

#define MAX_GRENADES 5

enum PlayerClasses
{
	CLASS_COMMANDO,
	CLASS_MEDIC,
	CLASS_ENGINEER,
	CLASS_PIONEER,
	CLASS_BERSERKER
};

enum Tips
{
	TIP_BUY,
	TIP_CLIPS,
	TIP_COMMANDO,
	TIP_MEDIC,
	TIP_ENGINEER,
	TIP_PIONEER,
	TIP_BERSERKER,
	NUM_TIPS
};

const int TipTimer[NUM_TIPS] =
{
	2000, // TIP_BUY
	5000, // TIP_CLIPS
	4500, // TIP_COMMANDO
	4500, // TIP_MEDIC
	4500, // TIP_ENGINEER
	4500, // TIP_PIONEER
	4500, // TIP_BERSERKER
};

static const char *TipText[NUM_TIPS] =
{
	"Check vote menu to buy & upgrade weapons", // TIP_BUY
	"Armor shows your remaining clips, collect armor to get more ammo", // TIP_ARMOR
	//"You can throw grenades using heart emoticon", // TIP_COMMANDO
	"You can create clips by using any emoticon", // TIP_COMMANDO
	"You can heal by using any emoticon", // TIP_MEDIC
	"You can put electro mines by using any emoticon", // TIP_ENGINEER
	"You can put land mines by using any emoticon", // TIP_PIONEER
	"You can activate bloodlust by using any emoticon" // TIP_PIONEER
};




struct CWeapon
{
	char m_Name[32];
	char m_BuyCmd[32];
	int m_ParentWeapon;
	int m_ProjectileType;
	int m_Sound;
	int m_Cost;
	int m_Damage;
	bool m_FullAuto;
	int m_ShotSpread;
	int m_ClipSize;
	int m_PowerupSize;
	int m_MaxAmmo;
	float m_BulletLife;
	int m_BulletReloadTime;
	int m_ClipReloadTime;
	float m_Knockback;
	int m_Extra1;
	int m_Require;


	CWeapon(const char *Name,
			const char *BuyCmd,
			int ParentWeapon,
			int ProjectileType,
			int Sound,
			int Require,
			int Cost,
			int Damage,
			int Extra1,
			bool FullAuto,
			int ShotSpread,
			int ClipSize,
			int PowerupSize,
			int MaxAmmo,
			float BulletLife,
			int BulletReloadTime,
			int ClipReloadTime,
			float Knockback)
	{
	    str_copy(m_Name, Name, sizeof(m_Name));
	    str_copy(m_BuyCmd, BuyCmd, sizeof(m_BuyCmd));
		m_ParentWeapon = ParentWeapon;
		m_ProjectileType = ProjectileType;
		m_Sound = Sound;
		m_Require = Require;
		m_Cost = Cost;
		m_Damage = Damage;
		m_Extra1 = Extra1;
		m_FullAuto = FullAuto;
		m_ShotSpread = ShotSpread;
		m_ClipSize = ClipSize;
		m_PowerupSize = PowerupSize;
		m_MaxAmmo = MaxAmmo;
		m_BulletLife = BulletLife;
		m_BulletReloadTime = BulletReloadTime;
		m_ClipReloadTime = ClipReloadTime;
		m_Knockback = Knockback;
	}
};

enum CustomWeapons
{
	SWORD_KATANA,
	SWORD_PIKATANA,
	HAMMER_BOTBASIC,
	HAMMER_BOTFLYHAMMER,
	HAMMER_BOTSUPERFLYHAMMER,
	HAMMER_BASIC,
	HAMMER_SANDMAN,
	HAMMER_MJOLNIR,
	SHOTGUN_BOTSHOTGUN,
	SHOTGUN_DOUBLEBARREL,
	SHOTGUN_COMBAT,
	SHOTGUN_LIGHTNING,
	GUN_BOTPISTOL,
	GUN_PISTOL,
	GUN_MAGNUM,
	GUN_SLEEPRAY,
	RIFLE_ASSAULTRIFLE,
	RIFLE_HEAVYRIFLE,
	RIFLE_LASERRIFLE,
	RIFLE_LIGHTNINGRIFLE,
	RIFLE_BOTLIGHTNING,
	RIFLE_BOTLASER,
	GRENADE_GRENADELAUNCHER,
	GRENADE_HEAVYLAUNCHER,
	GRENADE_DOOMLAUNCHER,
	GRENADE_GRENADE,
	NUM_CUSTOMWEAPONS
};

const int BotAttackRange[NUM_CUSTOMWEAPONS] =
{
	120, // SWORD_KATANA,
	120, // SWORD_PIKATANA,
	120, // HAMMER_BOTBASIC,
	400, // HAMMER_BOTFLYHAMMER,
	120, // HAMMER_BOTSUPERFLYHAMMER,
	120, // HAMMER_BASIC,
	120, // HAMMER_SANDMAN,
	120, // HAMMER_MJOLNIR,
	120, // SHOTGUN_BOTSHOTGUN,
	120, // SHOTGUN_DOUBLEBARREL,
	120, // SHOTGUN_COMBAT,
	120, // SHOTGUN_LIGHTNING,
	700, // GUN_BOTPISTOL,
	120, // GUN_PISTOL,
	120, // GUN_MAGNUM,
	120, // GUN_SLEEPRAY,
	120, // RIFLE_ASSAULTRIFLE,
	120, // RIFLE_HEAVYRIFLE,
	120, // RIFLE_LASERRIFLE,
	120, // RIFLE_LIGHTNINGRIFLE,
	600, // RIFLE_BOTLIGHTNING,
	800, // RIFLE_BOTLASER,
	120, // GRENADE_GRENADELAUNCHER,
	120, // GRENADE_HEAVYLAUNCHER,
	120, // GRENADE_DOOMLAUNCHER,
	120, // GRENADE_GRENADE
};

enum ProjectileTypes
{
	PROJTYPE_NONE,
	PROJTYPE_HAMMER,
	PROJTYPE_FLYHAMMER,
	PROJTYPE_SWORD,
	PROJTYPE_BULLET,
	PROJTYPE_PELLET,
	PROJTYPE_LASER,
	PROJTYPE_LIGHTNING,
	PROJTYPE_GRENADE,
	PROJTYPE_MINE
};

enum WeaponExtraFeature
{
	NO_EXTRA1,
	MEGAROCKETS,
	DOOMROCKETS,
	BULLETSPREAD,
	BIGBULLETSPREAD,
	SLEEPEFFECT,
	ELECTRIC,
};


const CWeapon aCustomWeapon[NUM_CUSTOMWEAPONS] =
{
	CWeapon(
		"Katana",
		"/buy katana",
		WEAPON_HAMMER,
		PROJTYPE_SWORD,
		SOUND_HAMMER_FIRE,
		-1, // require
		70, // cost
		3, // damage
		0, // extra1
		false, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0.65f, // bullet life - vanilla ninja life * bulletlife
		500, // bullet reload time
		0, // clip reload time
		0.0f // knockback
		),
	CWeapon(
		"Pikatanachu - Electric sword",
		"/upg pika",
		WEAPON_HAMMER,
		PROJTYPE_SWORD,
		SOUND_HAMMER_FIRE,
		SWORD_KATANA, // require
		100, // cost
		3, // damage
		ELECTRIC, // extra1
		false, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0.75f, // bullet life - vanilla ninja life * bulletlife
		620, // bullet reload time
		0, // clip reload time
		0.0f // knockback
		),
	CWeapon(
		"Bot Hammer",
		"/asd bothammer",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		2, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		300, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Bot Flyhammer",
		"/asd botflyhammer",
		WEAPON_HAMMER,
		PROJTYPE_FLYHAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		2, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		1.2f, // bullet life
		800, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Bot Superflyhammer",
		"/asd botsuperflyhammer",
		WEAPON_HAMMER,
		PROJTYPE_FLYHAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		2, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		2.0f, // bullet life
		800, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Hammer",
		"/buy hammer",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		-1, // require
		0, // cost
		3, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		200, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Sandman hammer",
		"/upg sandman",
		WEAPON_HAMMER,
		PROJTYPE_HAMMER,
		SOUND_HAMMER_FIRE,
		HAMMER_BASIC, // require
		50, // cost
		2, // damage
		SLEEPEFFECT, // extra1
		true, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		200, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Mjölnir, The hammer of Thor",
		"/upg thor",
		WEAPON_HAMMER,
		PROJTYPE_FLYHAMMER,
		SOUND_HAMMER_FIRE,
		HAMMER_BASIC, // require
		80, // cost
		3, // damage
		ELECTRIC, // extra1
		false, // autofire
		1, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		1.3f, // bullet life
		800, // bullet reload time
		0, // clip reload time
		1.0f // knockback
		),
	CWeapon(
		"Bot shotgun",
		"/asdasd botshotgun",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		-1, // require
		0, // cost
		1, // damage
		0, // extra1
		true, // autofire
		4, // bullet spread
		2, // clip size
		0, // powerup size
		0, // max ammo
		0.16f, // bullet life
		300, // bullet reload time
		800, // clip reload time
		3.0f // bullet knockback
		),
	CWeapon(
		"Double barrel shotgun",
		"/buy double",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		-1, // require
		75, // cost
		1, // damage
		0, // extra1
		false, // autofire
		6, // bullet spread
		2, // clip size
		6, // powerup size
		60, // max ammo
		0.17f, // bullet life
		225, // bullet reload time
		600, // clip reload time
		7.0f // bullet knockback
		),
	CWeapon(
		"Combat shotgun",
		"/upg combat",
		WEAPON_SHOTGUN,
		PROJTYPE_PELLET,
		SOUND_SHOTGUN_FIRE,
		SHOTGUN_DOUBLEBARREL, // require
		100, // cost
		1, // damage
		0, // extra1
		true, // autofire
		5, // bullet spread
		10, // clip size
		10, // powerup size
		100, // max ammo
		0.185f, // bullet life
		215, // bullet reload time
		1200, // clip reload time
		6.0f // bullet knockback
		),
	CWeapon(
		"Lightning shotgun",
		"/upg lightshot",
		WEAPON_SHOTGUN,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE, //SOUND_SHOTGUN_FIRE,
		SHOTGUN_DOUBLEBARREL, // require
		170, // cost
		2, // damage
		0, // extra1
		false, // autofire
		4, // bullet spread
		4, // clip size
		8, // powerup size
		40, // max ammo
		0.0f, // bullet life
		225, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Bot pistol",
		"/asdasd botpistol",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		0, // cost
		1, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		3, // clip size
		0, // powerup size
		0, // max ammo
		0.24f, // bullet life
		180, // bullet reload time
		700, // clip reload time
		1.0f // bullet knockback
		),
	CWeapon(
		"Pistol",
		"/buy pistol",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		0, // cost
		2, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		7, // clip size
		14, // powerup size
		70, // max ammo
		0.3f, // bullet life
		150, // bullet reload time
		650, // clip reload time
		10.0f // bullet knockback
		),
	CWeapon(
		"Magnum .9000",
		"/upg magnum",
		WEAPON_GUN,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		GUN_PISTOL, // require
		50, // cost
		3, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		12, // powerup size
		60, // max ammo
		0.3f, // bullet life
		160, // bullet reload time
		700, // clip reload time
		15.0f // bullet knockback
		),
	CWeapon(
		"Sleep ray",
		"/buy sleep",
		WEAPON_GUN,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		-1, // require
		0, // cost
		2, // damage
		0, // extra1
		false, // autofire
		1, // bullet spread
		10, // clip size
		10, // powerup size
		50, // max ammo
		0, // bullet life
		150, // bullet reload time
		150, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Assault rifle",
		"/buy rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		-1, // require
		80, // cost
		1, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		30, // clip size
		30, // powerup size
		210, // max ammo
		0.6f, // bullet life
		110, // bullet reload time
		1200, // clip reload time
		12.0f // bullet knockback
		),
	CWeapon(
		"Heavy assault rifle",
		"/upg rifle",
		WEAPON_RIFLE,
		PROJTYPE_BULLET,
		SOUND_GUN_FIRE,
		RIFLE_ASSAULTRIFLE, // require
		80, // cost
		2, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		30, // clip size
		30, // powerup size
		210, // max ammo
		0.6f, // bullet life
		110, // bullet reload time
		1200, // clip reload time
		17.0f // bullet knockback
		),
	CWeapon(
		"Laser rifle",
		"/buy laser",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		-1, // require
		100, // cost
		1, // damage
		BULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		30, // clip size
		30, // powerup size
		210, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Lightning rifle",
		"/upg lightning",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		RIFLE_LASERRIFLE, // require
		120, // cost
		2, // damage
		BIGBULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		30, // clip size
		30, // powerup size
		210, // max ammo
		0, // bullet life
		125, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Bot lightning",
		"/asdasd botlightning",
		WEAPON_RIFLE,
		PROJTYPE_LIGHTNING,
		SOUND_RIFLE_FIRE,
		-1, // require
		0, // cost
		1, // damage
		BIGBULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		125, // bullet reload time
		800, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Bot rifle",
		"/asdasd botlaser",
		WEAPON_RIFLE,
		PROJTYPE_LASER,
		SOUND_RIFLE_FIRE,
		-1, // require
		0, // cost
		1, // damage
		BIGBULLETSPREAD, // extra1
		true, // autofire
		1, // bullet spread
		4, // clip size
		0, // powerup size
		0, // max ammo
		0, // bullet life
		150, // bullet reload time
		1000, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Grenade launcher",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		75, // cost
		2, // damage
		0, // extra1
		true, // autofire
		1, // bullet spread
		6, // clip size
		12, // powerup size
		60, // max ammo
		1.4f, // bullet life
		300, // bullet reload time
		800, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Heavy launcher",
		"/upg heavy",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_GRENADELAUNCHER, // require
		100, // cost
		2, // damage
		MEGAROCKETS, // extra1
		true, // autofire
		1, // bullet spread
		4, // clip size
		8, // powerup size
		40, // max ammo
		1.3f, // bullet life
		300, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Doom launcher",
		"/upg doom",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		GRENADE_HEAVYLAUNCHER, // require
		120, // cost
		3, // damage
		DOOMROCKETS, // extra1
		true, // autofire
		1, // bullet spread
		3, // clip size
		6, // powerup size
		30, // max ammo
		1.2f, // bullet life
		300, // bullet reload time
		1200, // clip reload time
		0.0f // bullet knockback
		),
	CWeapon(
		"Grenade",
		"/buy grenade",
		WEAPON_GRENADE,
		PROJTYPE_GRENADE,
		SOUND_GRENADE_FIRE,
		-1, // require
		0, // cost
		1, // damage
		MEGAROCKETS, // extra1
		false, // autofire
		0, // bullet spread
		0, // clip size
		0, // powerup size
		0, // max ammo
		1.0f, // bullet life
		0, // bullet reload time
		0, // clip reload time
		0.0f // bullet knockback
		)
};



#endif