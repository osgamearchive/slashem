/*	SCCS Id: @(#)u_init.c	3.3	2000/06/11	*/
/* Copyright (c) Stichting Mathematisch Centrum, Amsterdam, 1985. */
/* NetHack may be freely redistributed.  See license for details. */

#include "hack.h"

struct trobj {
	short trotyp;
	schar trspe;
	char trclass;
	Bitfield(trquan,6);
	Bitfield(trbless,2);
};

static void FDECL(ini_inv, (struct trobj *));
static void FDECL(knows_object,(int));
static void FDECL(knows_class,(CHAR_P));
static boolean FDECL(restricted_spell_discipline, (int));

#define UNDEF_TYP       0
#define UNDEF_SPE       '\177'
#define UNDEF_BLESS     2

/*
 *      Initial inventory for the various roles.
 */

static struct trobj Archeologist[] = {
#define A_BOOK          4
	/* if adventure has a name...  idea from tan@uvm-gen */
	{ BULLWHIP, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_JACKET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FEDORA, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },        
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 2, UNDEF_BLESS },
	{ PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ TINNING_KIT, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Barbarian[] = {
#define B_MAJOR 0       /* two-handed sword or battle-axe  */
#define B_MINOR 1       /* matched with axe or short sword */
	{ TWO_HANDED_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ AXE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ RING_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Cave_man[] = {
#define C_AMMO	2
	{ CLUB, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SLING, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ FLINT, 0, GEM_CLASS, 15, UNDEF_BLESS },	/* quan is variable */
	{ ROCK, 0, GEM_CLASS, 3, 0 },			/* yields 18..33 */
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};

#if 0 /* Races */
static struct trobj Doppelganger[] = {
#define D_RING          2
	{ SILVER_DAGGER, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 1 },
	{ SPE_STONE_TO_FLESH, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef DWARF
static struct trobj Dwarf[] = {
	{ DWARVISH_SHORT_SWORD, 1, WEAPON_CLASS, 1, 1 },
	{ DWARVISH_MATTOCK, 0, WEAPON_CLASS, 1, 0 },
	{ DWARVISH_IRON_HELM, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CHAIN_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ DWARVISH_CLOAK, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj Elf[] = {
#define E_ARROWS_ONE    2
#define E_ARROWS_TWO    3
#define E_ARMOR         4
#define E_MAGIC		6
	{ ELVEN_SHORT_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_ARROW, 0, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ ELVEN_ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ UNDEF_TYP, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEMBAS_WAFER, 0, FOOD_CLASS, 4, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },        
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },        
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Drow[] = {
#define DROW_BOOK1 7
#define DROW_BOOK2 8
	{ DARK_ELVEN_SHORT_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DARK_ELVEN_ARROW, 0, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ DARK_ELVEN_MITHRIL_COAT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
#endif /* Races */

static struct trobj Flame_Mage[] = {
#define F_BOOK          9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_FIRE, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_FLAME_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_FIREBALL, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
  	{ 0, 0, 0, 0, 0 }
};

#if 0 /* Races */
static struct trobj Gnome[] = {
#define G_MAIN_WEAPON    0
#define G_RANGED_WEAPON  1
#define G_AMMO           2
	/* KMH -- Gnomes no longer start with half of a weapons store... */
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CROSSBOW, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ CROSSBOW_BOLT, 0, WEAPON_CLASS, 30, UNDEF_BLESS },        
	{ LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 3, 0 },
	{ PICK_AXE, 0, TOOL_CLASS, 1, UNDEF_BLESS },
	{ AGATE, 0, GEM_CLASS, 5, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif

static struct trobj Healer[] = {
	{ SCALPEL, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ STETHOSCOPE, 0, TOOL_CLASS, 1, 0 },
	{ MEDICAL_KIT, UNDEF_SPE, TOOL_CLASS, 1, 0 },        
	{ POT_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 4, UNDEF_BLESS },
	{ WAN_SLEEP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* [Tom] they might as well have a wand of healing, too */        
	{ WAN_HEALING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	/* always blessed, so it's guaranteed readable */
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_STONE_TO_FLESH, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
#if 0
static struct trobj Hobbit[] = {
#define HOB_SUIT	1
#define HOB_CLOAK	2
	{ DAGGER, 2, WEAPON_CLASS, 1, 1 },
	{ ELVEN_MITHRIL_COAT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ELVEN_CLOAK, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEMBAS_WAFER, 0, FOOD_CLASS, 3, 0 },
	{ CRAM_RATION, 0, FOOD_CLASS, 3, 0 },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj Ice_Mage[] = {
#define I_BOOK          9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ STUDDED_LEATHER_ARMOR, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ WAN_COLD, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 1, UNDEF_BLESS },
	{ SPE_FREEZE_SPHERE, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ SPE_CONE_OF_COLD, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Knight[] = {
	{ LONG_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SPEAR, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ PLATE_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LARGE_SHIELD, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ CARROT, 0, FOOD_CLASS, 10, 0 },
	{ 0, 0, 0, 0, 0 }
};
#if 0
static struct trobj Lycanthrope[] = {
	{ ORCISH_SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 2, 0 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 2, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#endif /* Race */
static struct trobj Monk[] = {
#define M_BOOK          2
	{ LEATHER_GLOVES, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ ROBE, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, UNDEF_BLESS },
	{ POT_HEALING, 0, POTION_CLASS, 3, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 3, 0 },
	{ APPLE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	{ ORANGE, 0, FOOD_CLASS, 5, UNDEF_BLESS },
	/* Yes, we know fortune cookies aren't really from China.  They were
	 * invented by George Jung in Los Angeles, California, USA in 1916.
	 */
	{ FORTUNE_COOKIE, 0, FOOD_CLASS, 3, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Necromancer[] = {
#define N_BOOK          8
/* pretty much like Wizard, except with pick-axe instead of magic resist. */
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },        /* for dealing with ghosts */
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ SPE_SUMMON_UNDEAD, 0, SPBOOK_CLASS, 1, 1 },
	{ SPE_COMMAND_UNDEAD, 0, SPBOOK_CLASS, 1, 1 },
	{ SPE_DRAIN_LIFE, 0, SPBOOK_CLASS, 1, 1 },
		/* WAC -- gave him drain life rather than turn undead */
	{ WAN_DRAINING, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
		/* KMH -- ...and the matching wand for the inexperienced */
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ PICK_AXE, UNDEF_SPE, TOOL_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Priest[] = {
#define P_BOOK          7
	{ MACE, 0, WEAPON_CLASS, 1, 1 },
	{ ROBE, 0, ARMOR_CLASS, 1, 1 },
	{ SMALL_SHIELD, 2, ARMOR_CLASS, 1, 1 },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },   /* holy water */
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 1, 1 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 1, 1 },
	{ SPE_HEALING, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Ranger[] = {
#define RAN_BOW			1
#define RAN_TWO_ARROWS	2
#define RAN_ZERO_ARROWS	3
	{ DAGGER, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ BOW, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ ARROW, 2, WEAPON_CLASS, 50, UNDEF_BLESS },
	{ ARROW, 0, WEAPON_CLASS, 30, UNDEF_BLESS },
	{ CLOAK_OF_DISPLACEMENT, 2, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CRAM_RATION, 0, FOOD_CLASS, 4, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Rogue[] = {
#define R_DAGGERS       1
#define R_DARTS         2
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ DAGGER, 0, WEAPON_CLASS, 10, 0 },     /* quan is variable */
	{ DART, 0, WEAPON_CLASS, 25, UNDEF_BLESS },
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ POT_SICKNESS, 0, POTION_CLASS, 1, 0 },
	{ SCR_GOLD_DETECTION, 0, SCROLL_CLASS, 4, 1 },
	{ SCR_TELEPORTATION, 0, SCROLL_CLASS, 4, 1 },
	{ LOCK_PICK, 9, TOOL_CLASS, 1, 0 },
	{ OILSKIN_SACK, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Samurai[] = {
#define S_ARROWS        3
	{ KATANA, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SHORT_SWORD, 0, WEAPON_CLASS, 1, UNDEF_BLESS }, /* wakizashi */
	{ YUMI, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ YA, 0, WEAPON_CLASS, 25, UNDEF_BLESS }, /* variable quan */
	{ SPLINT_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Tourist[] = {
#define T_DARTS         0
	{ DART, 2, WEAPON_CLASS, 25, UNDEF_BLESS },     /* quan is variable */
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 12, 0 },
	{ POT_EXTRA_HEALING, 0, POTION_CLASS, 2, UNDEF_BLESS },
	{ SCR_MAGIC_MAPPING, 0, SCROLL_CLASS, 6, UNDEF_BLESS },
	{ HAWAIIAN_SHIRT, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ EXPENSIVE_CAMERA, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ CREDIT_CARD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif
static struct trobj UndeadSlayer[] = {
#define U_MINOR 1       /* silver spear or whip [Castlevania] 25/25% */
                        /* crossbow 50% [Buffy] */
#define U_RANGE 2       /* silver daggers or crossbow bolts */
#define U_MISC  3       /* +1 boots [Buffy can kick] or helmet */
#define U_ARMOR 4       /* Tshirt/leather +1 or chain mail */
	{ WOODEN_STAKE, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SILVER_SPEAR, 0, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ SILVER_DAGGER, 0, WEAPON_CLASS, 5, UNDEF_BLESS },
	{ HELMET, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CHAIN_MAIL, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ CLOVE_OF_GARLIC, 0, FOOD_CLASS, 5, 1 },
	{ SPRIG_OF_WOLFSBANE, 0, FOOD_CLASS, 5, 1 },
	{ HOLY_WAFER, 0, FOOD_CLASS, 4, 0 },
	{ POT_WATER, 0, POTION_CLASS, 4, 1 },        /* holy water */
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Valkyrie[] = {
/*        { LONG_SWORD, 1, WEAPON_CLASS, 1, UNDEF_BLESS }, */
	{ SPEAR, 1, WEAPON_CLASS, 1, UNDEF_BLESS }, /* Traditional Weapon - WAC*/
	{ DAGGER, 0, WEAPON_CLASS, 5, UNDEF_BLESS },
	{ SMALL_SHIELD, 3, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ FOOD_RATION, 0, FOOD_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Wizard[] = {
#define W_MULTSTART	2
#define W_MULTEND	6
#define W_BOOK1         6
#define W_BOOK2         7
#define W_BOOK3         8
#define W_BOOK4         9
	{ QUARTERSTAFF, 1, WEAPON_CLASS, 1, 1 },
	{ CLOAK_OF_MAGIC_RESISTANCE, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, WAND_CLASS, 1, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, RING_CLASS, 2, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, POTION_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SCROLL_CLASS, 3, UNDEF_BLESS },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ UNDEF_TYP, UNDEF_SPE, SPBOOK_CLASS, 1, 1 },
	{ 0, 0, 0, 0, 0 }
};

#ifdef YEOMAN
static struct trobj Yeoman[] = {
	{ SHORT_SWORD, 2, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ PARTISAN, 1, WEAPON_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_ARMOR, 1, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ HIGH_BOOTS, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ LEATHER_GLOVES, 0, ARMOR_CLASS, 1, UNDEF_BLESS },
	{ APPLE, 0, FOOD_CLASS, 10, 0 },
	{ CARROT, 0, FOOD_CLASS, 10, 0 },
	{ POT_WATER, 0, POTION_CLASS, 3, 0 },
	{ FISHING_POLE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif

/*
 *      Optional extra inventory items.
 */

static struct trobj Tinopener[] = {
	{ TIN_OPENER, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Magicmarker[] = {
	{ MAGIC_MARKER, UNDEF_SPE, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Lamp[] = {
	{ OIL_LAMP, 1, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Blindfold[] = {
	{ BLINDFOLD, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Instrument[] = {
	{ WOODEN_FLUTE, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Xtra_food[] = {
	{ UNDEF_TYP, UNDEF_SPE, FOOD_CLASS, 2, 0 },
	{ 0, 0, 0, 0, 0 }
};
#ifdef TOURIST
static struct trobj Leash[] = {
	{ LEASH, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
static struct trobj Towel[] = {
	{ TOWEL, 0, TOOL_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};
#endif	/* TOURIST */
static struct trobj Wishing[] = {
	{ WAN_WISHING, 3, WAND_CLASS, 1, 0 },
	{ 0, 0, 0, 0, 0 }
};

/* race-based substitutions for initial inventory;
   the weaker cloak for elven rangers is intentional--they shoot better */
static struct inv_sub { short race_pm, item_otyp, subs_otyp; } inv_subs[] = {
    { PM_ELF,		DAGGER,			ELVEN_DAGGER	      },
    { PM_ELF,		SPEAR,			ELVEN_SPEAR	      },
    { PM_ELF,		SHORT_SWORD,		ELVEN_SHORT_SWORD     },
    { PM_ELF,		BOW,			ELVEN_BOW	      },
    { PM_ELF,		ARROW,			ELVEN_ARROW	      },
    { PM_ELF,		HELMET,			ELVEN_LEATHER_HELM    },
 /* { PM_ELF,		SMALL_SHIELD,		ELVEN_SHIELD	      }, */
    { PM_ELF,		CLOAK_OF_DISPLACEMENT,	ELVEN_CLOAK	      },
    { PM_ELF,		CRAM_RATION,		LEMBAS_WAFER	      },
    { PM_ORC,		DAGGER,			ORCISH_DAGGER	      },
    { PM_ORC,		SPEAR,			ORCISH_SPEAR	      },
    { PM_ORC,		SHORT_SWORD,		ORCISH_SHORT_SWORD    },
    { PM_ORC,		BOW,			ORCISH_BOW	      },
    { PM_ORC,		ARROW,			ORCISH_ARROW	      },
    { PM_ORC,		HELMET,			ORCISH_HELM	      },
    { PM_ORC,		SMALL_SHIELD,		ORCISH_SHIELD	      },
    { PM_ORC,		RING_MAIL,		ORCISH_RING_MAIL      },
    { PM_ORC,		CHAIN_MAIL,		ORCISH_CHAIN_MAIL     },
    { PM_DWARF, 	SPEAR,			DWARVISH_SPEAR	      },
    { PM_DWARF, 	SHORT_SWORD,		DWARVISH_SHORT_SWORD  },
    { PM_DWARF, 	HELMET,			DWARVISH_IRON_HELM    },
 /* { PM_DWARF, 	SMALL_SHIELD,		DWARVISH_ROUNDSHIELD  }, */
 /* { PM_DWARF, 	PICK_AXE,		DWARVISH_MATTOCK      }, */
    { PM_GNOME, 	BOW,			CROSSBOW	      },
    { PM_GNOME, 	ARROW,			CROSSBOW_BOLT	      },
    { PM_HUMAN_WEREWOLF,FOOD_RATION,		TRIPE_RATION	      },
    { PM_HUMAN_WEREWOLF,SILVER_SPEAR,		SPEAR		      },
    { PM_HUMAN_WEREWOLF,SILVER_DAGGER,		DAGGER		      },
    { PM_HUMAN_WEREWOLF,SILVER_BULLET,		BULLET		      },
    { PM_DROW,		DAGGER,			DARK_ELVEN_DAGGER     },
    { PM_DROW,		SHORT_SWORD,		DARK_ELVEN_SHORT_SWORD},
    { PM_DROW,		BOW,			DARK_ELVEN_BOW	      },
    { PM_DROW,		ARROW,			DARK_ELVEN_ARROW      },
    { NON_PM,		STRANGE_OBJECT,		STRANGE_OBJECT	      }
};

/* KMH -- Expectations for skills.
 *	1.	Starting equipment will start as basic, and should have a maximum
 *		of at least skilled.  If you enter the dungeon with it, you should
 *		know how to use it.  See weapon.c, skill_init().
 *	2.	First and second artifact gifts should have a maximum of at least
 *		skilled.  See artifact.c, init_artifacts().
 *	3.	Quest artifacts should have a maximum of expert.  See artifact.c,
 *		init_artifacts().
 *	4.	Races that construct weapons (elf, dwarf, orc) should have
 *		a maximum of expert in those weapons.
 *	5.	Spellcasters should get a maximum of at least basic in all spell
 *		categories.
 */

static struct def_skill Skill_A[] = {
    { P_DAGGER, P_BASIC },              { P_KNIFE,  P_BASIC },
    { P_PICK_AXE, P_EXPERT },           { P_SHORT_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_EXPERT },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },
#ifdef FIREARMS
	{ P_FIREARM, P_SKILLED },
#endif
    { P_SLING, P_SKILLED },             { P_DART, P_BASIC },
    { P_BOOMERANG, P_EXPERT },          { P_WHIP, P_EXPERT },
	{ P_UNICORN_HORN, P_SKILLED },
    { P_ATTACK_SPELL, P_BASIC },     { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT }, { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_PROTECTION_SPELL, P_BASIC },    { P_BODY_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },
    /*WAC- 'A' knows of all spells from studying,  but only really good
    at divination class*/
	{ P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_B[] = {
    { P_DAGGER, P_BASIC },              { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_EXPERT },           { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },       { P_LONG_SWORD, P_SKILLED },
    { P_TWO_HANDED_SWORD, P_EXPERT },   { P_SCIMITAR, P_SKILLED },
    { P_SABER, P_BASIC },               { P_CLUB, P_SKILLED },
    { P_PADDLE, P_SKILLED },
    { P_MACE, P_SKILLED },              { P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },        { P_SPEAR, P_SKILLED },
    { P_TRIDENT, P_SKILLED },           { P_BOW, P_BASIC },
    { P_ATTACK_SPELL, P_SKILLED },      { P_BODY_SPELL, P_SKILLED },
/*WAC - 'B' is good at attack spells and body manipulation spells
        -both combat oriented */
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_BASIC },
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static struct def_skill Skill_C[] = {
    { P_DAGGER, P_BASIC },              { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },               { P_PICK_AXE, P_BASIC },
    { P_CLUB, P_EXPERT },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_BASIC },        { P_FLAIL, P_SKILLED },
    { P_HAMMER, P_SKILLED },            { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },          { P_SPEAR, P_EXPERT },
    { P_JAVELIN, P_SKILLED },           { P_TRIDENT, P_SKILLED },
    { P_BOW, P_EXPERT },                { P_SLING, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },          { P_UNICORN_HORN, P_BASIC },
    { P_MATTER_SPELL, P_SKILLED },      { P_ENCHANTMENT_SPELL, P_BASIC },

/*WAC - 'C' is at one with the elements - matter spells, as well as
        basic enchantments - removed attack spell basic skill as
        it is now "dark" spells*/
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static struct def_skill Skill_D[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },        { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_BASIC },
    { P_MACE, P_SKILLED },              { P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_BASIC },            { P_SPEAR, P_BASIC },
#ifdef FIREARMS
	{ P_FIREARM, P_BASIC },
#endif
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_SKILLED },
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_BODY_SPELL, P_EXPERT },        { P_MATTER_SPELL, P_BASIC },

/*WAC   'D' are skilled in "body" spells as well as basic matter spells
        (self defense)*/
    { P_NONE, 0 }
};

#ifdef DWARF
static struct def_skill Skill_Dwa[] = {
	{ P_PICK_AXE, P_EXPERT },     { P_AXE, P_SKILLED },
	{ P_SHORT_SWORD, P_EXPERT },  { P_SCIMITAR, P_SKILLED },
	{ P_FLAIL, P_SKILLED },       { P_HAMMER, P_SKILLED },
	{ P_DAGGER, P_BASIC },        { P_SPEAR, P_EXPERT },
	{ P_JAVELIN, P_BASIC },       { P_CLUB, P_BASIC },
	{ P_LONG_SWORD, P_BASIC },    { P_SABER, P_BASIC },
	{ P_TWO_WEAPON_COMBAT, P_BASIC }, { P_BROAD_SWORD, P_EXPERT },

	/* Dwarves avoid magic */
	{ P_DIVINATION_SPELL, P_BASIC },
#ifdef STEED
	{ P_RIDING, P_BASIC },
#endif
        { P_TWO_WEAPON_COMBAT, P_SKILLED },
	{ P_BARE_HANDED_COMBAT, P_EXPERT },  { P_NONE, 0 }
};
#endif /* DWARF */

static struct def_skill Skill_E[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE, P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_EXPERT },
    { P_LONG_SWORD, P_SKILLED },        { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
        /* WAC removed shuriken, added quarterstaff - Drow quest arty */
    { P_QUARTERSTAFF, P_EXPERT },
    { P_SPEAR, P_EXPERT },              { P_JAVELIN, P_BASIC },
    { P_BOW, P_EXPERT },                { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },            /* { P_SHURIKEN, P_BASIC }, */

    { P_MATTER_SPELL, P_BASIC },        { P_HEALING_SPELL, P_SKILLED },
    { P_ENCHANTMENT_SPELL, P_EXPERT },
/*WAC      removed attack_spell skilled as attack spells include "evil spells
        and elves can now be lawful.  Replaced with Matter spells*/
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};

static struct def_skill Skill_F[] = {
/*Style: small-med edged weapons, blunt weapons*/
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_SKILLED },
    { P_AXE, P_BASIC },                 /*{ P_PICK_AXE, P_BASIC },*/
    { P_SHORT_SWORD, P_EXPERT },        /*{ P_BROAD_SWORD, P_BASIC },*/
    { P_LONG_SWORD, P_SKILLED },        /*{ P_TWO_HANDED_SWORD, P_BASIC },*/
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_PADDLE, P_BASIC },
    { P_MACE, P_BASIC },                /*{ P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },*/               { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_SKILLED },       /* { P_POLEARMS, P_BASIC },*/
/* Relies on spells for ranged attack
    { P_SPEAR, P_BASIC },               { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },                 { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },            { P_BOOMERANG, P_BASIC },
*/
    { P_WHIP, P_BASIC },                { P_UNICORN_HORN, P_SKILLED },

    { P_ATTACK_SPELL, P_SKILLED },     { P_HEALING_SPELL, P_BASIC },
	{ P_DIVINATION_SPELL, P_EXPERT }, { P_ENCHANTMENT_SPELL, P_BASIC },
	{ P_PROTECTION_SPELL, P_BASIC },    { P_BODY_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_EXPERT },
/*  Added expert matter spell (elements), skilled in attack, basic in rest
        He is a mage,  so knows the types.*/
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED }, { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};
static struct def_skill Skill_G[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
/* WAC added Pickaxe, Axe */
    { P_PICK_AXE, P_EXPERT },
    { P_AXE, P_EXPERT },                { P_SHORT_SWORD, P_EXPERT },
    /* { P_BROAD_SWORD, P_SKILLED },*/
/*WAC removed twohanded sword,  broadsword!*/
    { P_LONG_SWORD, P_SKILLED },      /*  { P_TWO_HANDED_SWORD, P_BASIC },*/
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_SKILLED },
    { P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_BASIC },        { P_FLAIL, P_BASIC },
/* WAC made hammer expert*/
    { P_HAMMER, P_EXPERT },             { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },               { P_BOW, P_SKILLED },
/* WAC added Sling Expert */
    { P_SLING, P_EXPERT },              { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_SKILLED },
    { P_DIVINATION_SPELL, P_BASIC }, { P_ENCHANTMENT_SPELL, P_EXPERT },
/*WAC - G must be good at enchant,  since his class skill is fear.
        also added a basic divination (from id gems) */
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_H[] = {
    { P_DAGGER, P_SKILLED },            { P_KNIFE, P_EXPERT },
    { P_SHORT_SWORD, P_SKILLED },       { P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },               { P_CLUB, P_SKILLED },
    { P_PADDLE, P_SKILLED },
    { P_MACE, P_BASIC },                { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_BASIC },            { P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },             { P_TRIDENT, P_BASIC },
    { P_SLING, P_SKILLED },             { P_DART, P_EXPERT },
    { P_SHURIKEN, P_SKILLED },          { P_UNICORN_HORN, P_EXPERT },

    { P_ATTACK_SPELL, P_BASIC },        { P_HEALING_SPELL, P_EXPERT },
	{ P_DIVINATION_SPELL, P_BASIC },    { P_ENCHANTMENT_SPELL, P_BASIC },
	{ P_PROTECTION_SPELL, P_BASIC },    { P_BODY_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static struct def_skill Skill_Hob[] = {
	{ P_DAGGER, P_EXPERT },             { P_KNIFE, P_BASIC },
	{ P_AXE, P_BASIC },                 { P_PICK_AXE, P_BASIC },
	{ P_SHORT_SWORD, P_SKILLED },       { P_BROAD_SWORD, P_BASIC },
	{ P_SCIMITAR, P_BASIC },            { P_HAMMER, P_BASIC },
	{ P_QUARTERSTAFF, P_BASIC },         { P_SPEAR, P_BASIC },
	{ P_SLING, P_EXPERT },              { P_CROSSBOW, P_BASIC },
	{ P_DART, P_BASIC },                { P_UNICORN_HORN, P_SKILLED },
#ifdef STEED
	{ P_RIDING, P_BASIC },
#endif
	{ P_TWO_WEAPON_COMBAT, P_EXPERT },
	{ P_BARE_HANDED_COMBAT, P_SKILLED },
	{ P_NONE, 0 }
};

static struct def_skill Skill_I[] = {
/*Resorts mostly to stabbing weapons*/
	{ P_DAGGER, P_EXPERT },             { P_KNIFE,  P_SKILLED },
/*	{ P_AXE, P_BASIC },                 { P_PICK_AXE, P_BASIC },
	{ P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_BASIC },*/
    { P_LONG_SWORD, P_SKILLED },      /*{ P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_MACE, P_BASIC },                { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_BASIC },*/
    { P_QUARTERSTAFF, P_SKILLED },      { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },               { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_LANCE, P_BASIC },
/*    { P_BOW, P_BASIC },                 { P_SLING, P_BASIC },
    { P_CROSSBOW, P_BASIC },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },            { P_BOOMERANG, P_BASIC },*/
    { P_WHIP, P_BASIC },                { P_UNICORN_HORN, P_SKILLED },

	{ P_ATTACK_SPELL, P_BASIC },        { P_HEALING_SPELL, P_SKILLED },
	{ P_DIVINATION_SPELL, P_BASIC },    { P_ENCHANTMENT_SPELL, P_EXPERT },
	{ P_PROTECTION_SPELL, P_SKILLED },  { P_BODY_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_EXPERT },
    /*WAC - same as Flame Mage*/
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED }, { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};
static struct def_skill Skill_K[] = {
    { P_DAGGER, P_BASIC },              { P_KNIFE, P_BASIC },
    { P_AXE, P_SKILLED },               { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_SKILLED },       { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },         { P_TWO_HANDED_SWORD, P_SKILLED },
    { P_SCIMITAR, P_BASIC },            { P_SABER, P_SKILLED },
    { P_CLUB, P_BASIC },                { P_PADDLE, P_BASIC },
    { P_MACE, P_SKILLED },
    { P_MORNING_STAR, P_SKILLED },      { P_FLAIL, P_BASIC },
    { P_HAMMER, P_BASIC },              { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },             { P_JAVELIN, P_SKILLED },
    { P_TRIDENT, P_BASIC },             { P_LANCE, P_EXPERT },
    { P_BOW, P_BASIC },                 { P_CROSSBOW, P_SKILLED },
    { P_HEALING_SPELL, P_SKILLED },
    { P_PROTECTION_SPELL, P_SKILLED },      { P_BODY_SPELL, P_BASIC },
/*WAC - removed the attack spells - no good having knights summoning
        undead!  Replaced with skilled body spells*/

#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED }, { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_L[] = {
    { P_DAGGER, P_BASIC },              { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },               { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_EXPERT },        { P_CLUB, P_EXPERT },
    { P_PADDLE, P_BASIC },
    { P_MACE, P_EXPERT },		{ P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_SKILLED },		{ P_HAMMER, P_SKILLED },
    { P_QUARTERSTAFF, P_EXPERT },	{ P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },		{ P_JAVELIN, P_SKILLED },
    { P_TRIDENT, P_SKILLED },		{ P_BOW, P_EXPERT },
    { P_SLING, P_SKILLED },		{ P_BOOMERANG, P_EXPERT },
    { P_UNICORN_HORN, P_BASIC },

    { P_HEALING_SPELL, P_BASIC },     { P_PROTECTION_SPELL, P_BASIC },    
/*WAC - added skilled body spells (he polymorphs), expert heal
        - regeneration and create familiar*/
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },
    { P_NONE, 0 }
};

static struct def_skill Skill_Mon[] = {
    { P_PADDLE, P_SKILLED },
    { P_QUARTERSTAFF, P_SKILLED },	{ P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },		{ P_BOW, P_BASIC },
    { P_SHURIKEN, P_BASIC },            { P_MARTIAL_ARTS, P_GRAND_MASTER },

    { P_ATTACK_SPELL, P_BASIC },	{ P_HEALING_SPELL, P_EXPERT },
    { P_DIVINATION_SPELL, P_BASIC },	{ P_ENCHANTMENT_SPELL, P_BASIC },
    { P_PROTECTION_SPELL, P_EXPERT },	{ P_BODY_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },
/*WAC - monks are good healers - expert healing - and expert protect*/
    { P_NONE, 0 }
};
static struct def_skill Skill_N[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },               { P_PICK_AXE, P_SKILLED },
    { P_CLUB, P_SKILLED },              { P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },       { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },               { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_BASIC },

    { P_ATTACK_SPELL, P_EXPERT },
    { P_MATTER_SPELL, P_EXPERT },
/*WAC-  expert of dark arts - attack spells,  skilled in matter
        -for fireball and cone of cold*/
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};
static struct def_skill Skill_P[] = {
	/* KMH -- Long sword for Sunsword */
	{ P_LONG_SWORD, P_SKILLED },
    { P_CLUB, P_EXPERT },               { P_PADDLE, P_EXPERT },
    { P_MACE, P_EXPERT },
    { P_MORNING_STAR, P_EXPERT },       { P_FLAIL, P_EXPERT },
    { P_HAMMER, P_EXPERT },             { P_QUARTERSTAFF, P_EXPERT },
    { P_POLEARMS, P_SKILLED },          { P_SPEAR, P_SKILLED },
    { P_JAVELIN, P_SKILLED },           { P_TRIDENT, P_SKILLED },
    { P_LANCE, P_BASIC },               { P_BOW, P_BASIC },
    { P_SLING, P_BASIC },               { P_CROSSBOW, P_BASIC },
    { P_DART, P_BASIC },                { P_SHURIKEN, P_BASIC },
    { P_BOOMERANG, P_BASIC },           { P_UNICORN_HORN, P_SKILLED },

    { P_ATTACK_SPELL, P_BASIC },        { P_HEALING_SPELL, P_EXPERT },
	{ P_DIVINATION_SPELL, P_EXPERT },   { P_ENCHANTMENT_SPELL, P_BASIC },
	{ P_PROTECTION_SPELL, P_EXPERT },   { P_BODY_SPELL, P_BASIC },
    { P_MATTER_SPELL, P_BASIC },

    { P_BARE_HANDED_COMBAT, P_BASIC },  /* the monk is added in slash */ 
    { P_NONE, 0 }
};

static struct def_skill Skill_R[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_EXPERT },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_SKILLED },        { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_SKILLED },
    { P_MACE, P_SKILLED },              { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_BASIC },
    { P_POLEARMS, P_BASIC },            { P_SPEAR, P_BASIC },
#ifdef FIREARMS
	{ P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },        { P_BODY_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED },
/*WAC Left as is*/
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_Ran[] = {
    { P_DAGGER, P_EXPERT },		 { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },        { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_BASIC },  { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_SKILLED },      { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC }, { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },      { P_JAVELIN, P_EXPERT },
    { P_TRIDENT, P_BASIC },      { P_BOW, P_EXPERT },
    { P_SLING, P_EXPERT },       { P_CROSSBOW, P_EXPERT },
    { P_DART, P_EXPERT },        { P_SHURIKEN, P_SKILLED },
    { P_BOOMERANG, P_EXPERT },   { P_WHIP, P_BASIC },
    { P_HEALING_SPELL, P_BASIC },
    { P_DIVINATION_SPELL, P_EXPERT },
    { P_BODY_SPELL, P_BASIC },
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

static struct def_skill Skill_S[] = {
    { P_DAGGER, P_BASIC },              { P_KNIFE,  P_SKILLED },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_SKILLED },
    { P_LONG_SWORD, P_EXPERT },         { P_TWO_HANDED_SWORD, P_EXPERT },
    { P_SCIMITAR, P_BASIC },            { P_SABER, P_BASIC },
    { P_PADDLE, P_BASIC },
    { P_FLAIL, P_SKILLED },             { P_QUARTERSTAFF, P_BASIC },
    { P_POLEARMS, P_SKILLED },          { P_SPEAR, P_BASIC },
    { P_JAVELIN, P_BASIC },             { P_LANCE, P_SKILLED },
    { P_BOW, P_EXPERT },                { P_SHURIKEN, P_EXPERT },
    { P_PROTECTION_SPELL, P_SKILLED },      { P_BODY_SPELL, P_BASIC },

/* WAC - removed the attack spells, replace with body.  clairvoyance, meditate*/
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_EXPERT },  { P_MARTIAL_ARTS, P_GRAND_MASTER },
    { P_NONE, 0 }
};

#ifdef TOURIST
static struct def_skill Skill_T[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_SKILLED },
    { P_AXE, P_BASIC },                 { P_PICK_AXE, P_BASIC },
    { P_SHORT_SWORD, P_EXPERT },        { P_BROAD_SWORD, P_BASIC },
    { P_LONG_SWORD, P_BASIC },          { P_TWO_HANDED_SWORD, P_BASIC },
    { P_SCIMITAR, P_SKILLED },          { P_SABER, P_SKILLED },
    { P_PADDLE, P_BASIC },
    { P_MACE, P_BASIC },                { P_MORNING_STAR, P_BASIC },
    { P_FLAIL, P_BASIC },               { P_HAMMER, P_BASIC },
    { P_QUARTERSTAFF, P_BASIC },        { P_POLEARMS, P_BASIC },
    { P_SPEAR, P_BASIC },               { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_LANCE, P_BASIC },
    { P_BOW, P_BASIC },                 { P_SLING, P_BASIC },
#ifdef FIREARMS
	{ P_FIREARM, P_BASIC },
#endif
    { P_CROSSBOW, P_BASIC },            { P_DART, P_EXPERT },
    { P_SHURIKEN, P_BASIC },            { P_BOOMERANG, P_BASIC },
    { P_WHIP, P_BASIC },                { P_UNICORN_HORN, P_SKILLED },
    { P_DIVINATION_SPELL, P_BASIC },  { P_ENCHANTMENT_SPELL, P_BASIC },
    { P_BODY_SPELL, P_SKILLED },
/*WAC left alone*/
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED }, { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};
#endif /* TOURIST */

static struct def_skill Skill_U[] = {
/*WAC
 * -made dagger skill expert too,  since it's a starting weapon
 * -made spear skill Expert rather than Skilled
 *      Slayer artifact is a spear,  after all
 * -made crossbow skill Expert - Dracula movies
 * -changed bare handed basic to martial arts master-Buffy the Vampire Slayer
 * -Added whip Expert - Castlevania
 * -made club, flail, mace, morning star, hammer, quarterstaff Skilled
        from Expert to balance
 * -removed Trident skill - from Skilled to Restricted
 * -removed Lance skill - from Basic to Restricted
 */
    { P_DAGGER, P_EXPERT },             { P_LONG_SWORD, P_SKILLED },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_SKILLED },
    { P_MACE, P_SKILLED },              { P_MORNING_STAR, P_SKILLED },
    { P_FLAIL, P_SKILLED },             { P_HAMMER, P_SKILLED },
    { P_QUARTERSTAFF, P_SKILLED },      { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_EXPERT },              { P_JAVELIN, P_SKILLED },
    { P_BOW, P_BASIC },			        { P_SLING, P_BASIC },
#ifdef FIREARMS
    { P_FIREARM, P_EXPERT },
#endif
    { P_CROSSBOW, P_EXPERT },           { P_DART, P_BASIC },
    { P_SHURIKEN, P_BASIC },		{ P_BOOMERANG, P_BASIC },
    { P_WHIP, P_EXPERT },               { P_UNICORN_HORN, P_SKILLED },

    { P_PROTECTION_SPELL, P_SKILLED },    { P_BODY_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_BASIC },
/*WAC - added PROTECTION spells,  body spells as skilled, basic
        matter spells - for the fire vs undead*/
    { P_BARE_HANDED_COMBAT, P_GRAND_MASTER },    { P_NONE, 0 }
};

static struct def_skill Skill_V[] = {
    { P_DAGGER, P_EXPERT },             { P_AXE, P_EXPERT },
    { P_PICK_AXE, P_SKILLED },          { P_SHORT_SWORD, P_SKILLED },
    { P_BROAD_SWORD, P_SKILLED },       { P_LONG_SWORD, P_EXPERT },
    { P_TWO_HANDED_SWORD, P_EXPERT },   { P_SCIMITAR, P_BASIC },
    { P_SABER, P_BASIC },               { P_HAMMER, P_EXPERT },
    { P_QUARTERSTAFF, P_BASIC },        { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_SKILLED },             { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_LANCE, P_SKILLED },
    { P_SLING, P_BASIC },
    { P_BODY_SPELL, P_BASIC },          { P_MATTER_SPELL, P_EXPERT },
/*  replace attack spell with matter spell - cone of cold, lightning
 *  Boosted to Expert
 */
#ifdef STEED
    { P_RIDING, P_SKILLED },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED },
    { P_BARE_HANDED_COMBAT, P_EXPERT },
    { P_NONE, 0 }
};

static struct def_skill Skill_W[] = {
    { P_DAGGER, P_EXPERT },             { P_KNIFE,  P_SKILLED },
    { P_AXE, P_SKILLED },               { P_SHORT_SWORD, P_BASIC },
    { P_CLUB, P_SKILLED },              { P_PADDLE, P_BASIC },
    { P_MACE, P_BASIC },
    { P_QUARTERSTAFF, P_EXPERT },       { P_POLEARMS, P_SKILLED },
    { P_SPEAR, P_BASIC },               { P_JAVELIN, P_BASIC },
    { P_TRIDENT, P_BASIC },             { P_SLING, P_SKILLED },
    { P_DART, P_EXPERT },               { P_SHURIKEN, P_BASIC },

    { P_ATTACK_SPELL, P_EXPERT },       { P_HEALING_SPELL, P_SKILLED },
    { P_DIVINATION_SPELL, P_SKILLED },  { P_ENCHANTMENT_SPELL, P_SKILLED },
    { P_PROTECTION_SPELL, P_SKILLED },  { P_BODY_SPELL, P_SKILLED },
    { P_MATTER_SPELL, P_SKILLED },
/*      added matter spell skilled,  as fireball and cone of cold are
        matter spells, but now specialty of F/I Mages*/
#ifdef STEED
    { P_RIDING, P_BASIC },
#endif
    { P_BARE_HANDED_COMBAT, P_BASIC },
    { P_NONE, 0 }
};

#ifdef YEOMAN
static struct def_skill Skill_Y[] = {
    { P_DAGGER, P_SKILLED },            { P_KNIFE, P_BASIC },
    { P_AXE, P_SKILLED },               { P_SHORT_SWORD, P_EXPERT },
    { P_BROAD_SWORD, P_BASIC },         { P_LONG_SWORD, P_SKILLED },
    { P_SABER, P_SKILLED },             { P_PADDLE, P_SKILLED },
    { P_HAMMER, P_BASIC },              { P_QUARTERSTAFF, P_SKILLED },
    { P_POLEARMS, P_EXPERT },           { P_SPEAR, P_BASIC },
    { P_JAVELIN, P_SKILLED },           { P_TRIDENT, P_SKILLED },
    { P_LANCE, P_SKILLED },             { P_BOW, P_EXPERT },
    { P_SLING, P_BASIC },
#ifdef FIREARMS
	{ P_FIREARM, P_SKILLED },
#endif
    { P_CROSSBOW, P_SKILLED },          { P_DART, P_BASIC },
    { P_WHIP, P_BASIC },                { P_UNICORN_HORN, P_BASIC },

    { P_ENCHANTMENT_SPELL, P_SKILLED }, { P_PROTECTION_SPELL, P_SKILLED },
    { P_BODY_SPELL, P_BASIC },
#ifdef STEED
    { P_RIDING, P_EXPERT },
#endif
    { P_TWO_WEAPON_COMBAT, P_SKILLED }, { P_BARE_HANDED_COMBAT, P_SKILLED },
    { P_NONE, 0 }
};
#endif


static void
knows_object(obj)
register int obj;
{
	discover_object(obj,TRUE,FALSE);
	objects[obj].oc_pre_discovered = 1;     /* not a "discovery" */
}

/* Know ordinary (non-magical) objects of a certain class,
 * like all gems except the loadstone and luckstone.
 */
static void
knows_class(sym)
register char sym;
{
	register int ct;
	for (ct = 1; ct < NUM_OBJECTS; ct++)
		if (objects[ct].oc_class == sym && !objects[ct].oc_magic)
			knows_object(ct);
}

void
u_init()
{
	register int i, temp, temp2;

	flags.female = flags.initgend;
	flags.beginner = 1;

	/* WAC -- Clear Tech List since adjabil will init the 1st level techs*/
	for (i = 0; i <= MAXTECH; i++) tech_list[i].t_id = NO_TECH;

	/* Initialize spells */
	for (i = 0; i <= MAXSPELL; i++) spl_book[i].sp_id = NO_SPELL;

	/* Initialize the "u" structure.
	 * Note that some values may have been incorrectly set by a failed restore.
	 */
	(void) memset((genericptr_t)&u, 0, sizeof(u));
	u.ustuck = (struct monst *)0;
#if 0   /* documentation of more zero values as desirable */
	u.usick_cause[0] = 0;
	u.uluck  = u.moreluck = 0;
# ifdef TOURIST
	uarmu = 0;
# endif
	uarm = uarmc = uarmh = uarms = uarmg = uarmf = 0;
	uwep = uball = uchain = uleft = uright = 0;
	/* WAC Added uswapwep, uquiver*/
	uswapwep = uquiver = 0;
	/* KMH -- added two-weapon combat */
	u.twoweap = 0;
	uswapwep = uquiver = 0;
	u.twoweap = 0;
	u.ublessed = 0;                         /* not worthy yet */
	u.ugangr   = 0;                         /* gods not angry */
	u.ugifts   = 0;				/* no divine gifts bestowed */
	/* WAC not needed - use techs */
/*	u.unextuse = 0;
	u.ulastuse = 0;*/
# ifdef ELBERETH
	u.uevent.uhand_of_elbereth = 0;
# endif
	u.uevent.uheard_tune = 0;
	u.uevent.uopened_dbridge = 0;
	u.uevent.udemigod = 0;          /* not a demi-god yet... */
	u.udg_cnt = 0;
	u.mh = u.mhmax = Upolyd = 0;
	u.uz.dnum = u.uz0.dnum = 0;
	u.utotype = 0;
#endif  /* 0 */

	u.uz.dlevel = 1;
	u.uz0.dlevel = 0;
	u.utolev = u.uz;

	u.umoved = FALSE;
	u.umortality = 0;
	u.ugrave_arise = NON_PM;

	u.umonnum = u.umonster = (flags.female &&
			urole.femalenum != NON_PM) ? urole.femalenum :
			urole.malenum;

	init_uasmon();

	u.ulevel = 0;   /* set up some of the initial attributes */
  	u.uhp = u.uhpmax = newhp();
 	u.uenmax = urole.enadv.infix + urace.enadv.infix;
	if (urole.enadv.inrnd > 0)
	    u.uenmax += rnd(urole.enadv.inrnd);
	if (urace.enadv.inrnd > 0)
	    u.uenmax += rnd(urace.enadv.inrnd);
	u.uen = u.uenmax;
	u.uspellprot = 0;  
	adjabil(0,1);
	u.ulevel = u.ulevelmax = 1;
	
	init_uhunger();
	u.ublesscnt = 300;                      /* no prayers just yet */
	u.ualignbase[A_CURRENT] = u.ualignbase[A_ORIGINAL] = u.ualign.type =
			aligns[flags.initalign].value;
	u.ulycn = NON_PM;
#ifdef BSD
	(void) time((long *)&u.ubirthday);
#else
	(void) time(&u.ubirthday);
#endif
	/*
	 *  For now, everyone but elves, cavemen and lycanthropes starts
	 *  out with a night vision range of 1 and their xray range disabled.
	 */
	u.nv_range   =  1;
	u.xray_range = -1;

	/* Role-specific initializations */
	switch (Role_switch) {
	case PM_ARCHEOLOGIST:
		switch (rnd(5)) {   
		    case 1: Archeologist[A_BOOK].trotyp = SPE_DETECT_FOOD; break;
		    case 2: Archeologist[A_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
		    case 3: Archeologist[A_BOOK].trotyp = SPE_LIGHT; break;
		    case 4: Archeologist[A_BOOK].trotyp = SPE_KNOCK; break;
		    case 5: Archeologist[A_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Archeologist);
		if(!rn2(4)) ini_inv(Blindfold);
		else if(!rn2(4)) ini_inv(Towel);
		if(!rn2(4)) ini_inv(Leash);
		if(!rn2(4)) ini_inv(Tinopener);
		if(!rn2(4)) ini_inv(Lamp);
		if(!rn2(8)) ini_inv(Magicmarker);
		knows_class(GEM_CLASS);
		knows_object(SACK);
		skill_init(Skill_A);
		break;
	case PM_BARBARIAN:
		if (rn2(100) >= 50) {   /* see Elf comment */
		    Barbarian[B_MAJOR].trotyp = BATTLE_AXE;
		    Barbarian[B_MINOR].trotyp = SHORT_SWORD;
		}
		ini_inv(Barbarian);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_B);
		break;
	case PM_CAVEMAN:
		u.nv_range = 2;
		Cave_man[C_AMMO].trquan = rn1(11, 10);	/* 10..20 */
		ini_inv(Cave_man);
		skill_init(Skill_C);
		break;
#if 0
	case PM_DOPPELGANGER:
		/* [Tom] D's get such a high amount for their polymorphing...
		    (they suck at spells) */
		u.uen = u.uenmax += 10;
		switch (rnd(16)) {
		    case 1:  Doppelganger[D_RING].trotyp = RIN_REGENERATION; break;
		    case 2:  Doppelganger[D_RING].trotyp = RIN_SEARCHING; break;
		    case 3:  Doppelganger[D_RING].trotyp = RIN_CONFLICT; break;
		    case 4:  Doppelganger[D_RING].trotyp = RIN_POISON_RESISTANCE; break;
		    case 5:  Doppelganger[D_RING].trotyp = RIN_FIRE_RESISTANCE; break;
		    case 6:  Doppelganger[D_RING].trotyp = RIN_COLD_RESISTANCE; break;
		    case 7:  Doppelganger[D_RING].trotyp = RIN_SHOCK_RESISTANCE; break;
		    case 8:  Doppelganger[D_RING].trotyp = RIN_TELEPORTATION; break;
		    case 9:  Doppelganger[D_RING].trotyp = RIN_TELEPORT_CONTROL; break;
		    case 10: Doppelganger[D_RING].trotyp = RIN_INVISIBILITY; break;
		    case 11: Doppelganger[D_RING].trotyp = RIN_SEE_INVISIBLE; break;
		    case 12: Doppelganger[D_RING].trotyp = RIN_PROTECTION;
			     Doppelganger[D_RING].trspe = rnd(5);
			     break;
		    case 13: Doppelganger[D_RING].trotyp = RIN_GAIN_STRENGTH;
			     Doppelganger[D_RING].trspe = rnd(5);
			     break;
		    case 14: Doppelganger[D_RING].trotyp = RIN_GAIN_CONSTITUTION;
			     Doppelganger[D_RING].trspe = rnd(5);
			     break;
		    case 15: Doppelganger[D_RING].trotyp = RIN_INCREASE_DAMAGE;
			     Doppelganger[D_RING].trspe = rnd(5);
			     break;
		    case 16: Doppelganger[D_RING].trotyp = RIN_ADORNMENT;
			     Doppelganger[D_RING].trspe = rnd(5);
			     break;
		    default: break;
		}
		ini_inv(Doppelganger);
		skill_init(Skill_D);
		break;
#ifdef DWARF
	case PM_DWARF:
		u.nv_range = 3;
		ini_inv(Dwarf);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		knows_class(GEM_CLASS);
		skill_init(Skill_Dwa);
		break;
#endif
	case PM_ELF:
	case PM_DROW:
		u.nv_range = 2;                
		if (Role_if(PM_ELF)) {
		  Elf[E_ARROWS_ONE].trquan = rn1(10, 50);
		  Elf[E_ARROWS_TWO].trquan = rn1(10, 30);
		  Elf[E_ARMOR].trotyp = ((rn2(100) >= 50)
				 ? ELVEN_MITHRIL_COAT : ELVEN_CLOAK);
			/* rn2(100) > 50 necessary because some random number
			 * generators are bad enough to seriously skew the
			 * results if we use rn2(2)...  --KAA
			 */
			 
		  /*
		   * Elves are people of music and song, or they are warriors.
		   * Warriors get mithril coats; non-warriors will get an
		   * instrument.  We use a kludge to get only non-magic
		   * instruments.
		   */

		  if (Elf[E_ARMOR].trotyp == ELVEN_CLOAK) {	  
		    static int trotyp[] = {
			WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
			BELL, BUGLE, LEATHER_DRUM
		    };
		    
		    Elf[E_MAGIC].trclass = SPBOOK_CLASS;
		    Elf[E_MAGIC].trquan = 1;
		    switch (rnd(2)) {                
			case 1: Elf[E_MAGIC].trotyp = SPE_CONFUSE_MONSTER; break;
			case 2: Elf[E_MAGIC].trotyp = SPE_CAUSE_FEAR; break;
			default: break;
		    }

		    ini_inv(Elf);
		    
		    Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		    ini_inv(Instrument);     
		    if(!rn2(5)) ini_inv(Magicmarker);
		    
		  } else ini_inv(Elf);
		  
		  if(!rn2(5)) ini_inv(Blindfold);
		  else if(!rn2(6)) ini_inv(Lamp);
		  knows_object(ELVEN_SHORT_SWORD);
		  knows_object(ELVEN_ARROW);
		  knows_object(ELVEN_BOW);
		  knows_object(ELVEN_SPEAR);
		  knows_object(ELVEN_DAGGER);
		  knows_object(ELVEN_BROADSWORD);
		  knows_object(ELVEN_MITHRIL_COAT);
		  knows_object(ELVEN_LEATHER_HELM);
		  knows_object(ELVEN_SHIELD);
		  knows_object(ELVEN_BOOTS);
		  knows_object(ELVEN_CLOAK);
		} else { /* is a dark elf */
		   switch (rnd(2)) {                
			case 1: Drow[DROW_BOOK1].trotyp = SPE_CONFUSE_MONSTER; break;
			case 2: Drow[DROW_BOOK1].trotyp = SPE_CAUSE_FEAR; break;
			default: break;
		   }

		   switch (rnd(3)) {                
			case 1: Drow[DROW_BOOK2].trotyp = SPE_DIG; break;
			case 2: Drow[DROW_BOOK2].trotyp = SPE_CURE_BLINDNESS; break;
			case 3: Drow[DROW_BOOK2].trotyp = SPE_CURE_SICKNESS; break;
			default: break;
		   }
		   
		   ini_inv(Drow);
		   knows_object(DARK_ELVEN_SHORT_SWORD);
		   knows_object(DARK_ELVEN_ARROW);
		   knows_object(DARK_ELVEN_BOW);
		   knows_object(DARK_ELVEN_DAGGER);
		   knows_object(DARK_ELVEN_MITHRIL_COAT);
		}
		skill_init(Skill_E);
		break;
#endif /* RACES */
	case PM_FLAME_MAGE:
		switch (rnd(2)) {                
			case 1: Flame_Mage[F_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
			case 2: Flame_Mage[F_BOOK].trotyp = SPE_LIGHT; break;
			default: break;
		}
		ini_inv(Flame_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_F);
		break;
#if 0 /* RACES */
	case PM_GNOME:
		u.nv_range = 2;
		switch (rn2(100)/25) {
			/* Main weapons */
			case 0:	/* Club */
				Gnome[G_MAIN_WEAPON].trotyp = CLUB;
				break;
			case 1:	/* Aklys */
				Gnome[G_MAIN_WEAPON].trotyp = AKLYS;
				break;
			case 2:	/* War Hammer */
				Gnome[G_MAIN_WEAPON].trotyp = WAR_HAMMER;
				break;
			default:/* Short sword */
				break;
		}
		switch (rn2(100)/20) {
			/* Ranged weapons */
			case 0:	/* Daggers and darts */
				Gnome[G_RANGED_WEAPON].trotyp = DAGGER;
				Gnome[G_RANGED_WEAPON].trquan = rn1(10, 6);
				Gnome[G_AMMO].trotyp = DART;
				Gnome[G_AMMO].trquan = rn1(10, 6);
				break;
			case 1:	/* Sling and rocks */
				Gnome[G_RANGED_WEAPON].trotyp = SLING;
				Gnome[G_AMMO].trotyp = ROCK;
				Gnome[G_AMMO].trclass = GEM_CLASS;
				Gnome[G_AMMO].trquan = rn1(1, 3);
				/*mksobj makes 6-12 rocks for one here...*/
				break;
			case 2:	/* Bow and arrows */
				Gnome[G_RANGED_WEAPON].trotyp = BOW;
				Gnome[G_AMMO].trotyp = ARROW;
				Gnome[G_AMMO].trquan = rn1(10, 30);
				break;
			default:/* Crossbow and bolts */
				Gnome[G_AMMO].trquan = rn1(10, 30);
				break;
		}
		ini_inv(Gnome);
		knows_class(GEM_CLASS);
		skill_init(Skill_G);
		break;
#endif /* RACES */
	case PM_HEALER:
		u.ugold = u.ugold0 = rn1(1000, 1001);
		ini_inv(Healer);
		knows_class(POTION_CLASS); /* WAC - remove? */
		knows_object(POT_SICKNESS);
		knows_object(POT_BLINDNESS);
		knows_object(POT_HALLUCINATION);
		knows_object(POT_RESTORE_ABILITY);
		knows_object(POT_FULL_HEALING);
		knows_object(HEALTHSTONE);	/* KMH */
		if(!rn2(5)) ini_inv(Lamp);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_H);
		break;
#if 0
	case PM_HOBBIT:
		if (rn2(100) < 50) {
			Hobbit[HOB_SUIT].trotyp = LEATHER_ARMOR;
			Hobbit[HOB_SUIT].trspe = 2;
			Hobbit[HOB_CLOAK].trspe = 1;
		}
		if (rn2(100) < 50)
			Hobbit[HOB_CLOAK].trotyp = DWARVISH_CLOAK;
		ini_inv(Hobbit);
		skill_init(Skill_Hob);
		break;
#endif
	case PM_ICE_MAGE:
		switch (rnd(2)) {                
			case 1: Ice_Mage[I_BOOK].trotyp = SPE_CONFUSE_MONSTER; break;
			case 2: Ice_Mage[I_BOOK].trotyp = SPE_SLOW_MONSTER; break;
			default: break;
		}
		ini_inv(Ice_Mage);
		if(!rn2(5)) ini_inv(Lamp);
		else if(!rn2(5)) ini_inv(Blindfold);
		else if(!rn2(5)) ini_inv(Magicmarker);
		skill_init(Skill_I);
		break;
	case PM_KNIGHT:
		ini_inv(Knight);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		/* give knights chess-like mobility
		 * -- idea from wooledge@skybridge.scl.cwru.edu */
		HJumping |= FROMOUTSIDE;
		skill_init(Skill_K);
		break;
	case PM_MONK:
		switch (rn2(90) / 30) {
		case 0: Monk[M_BOOK].trotyp = SPE_HEALING; break;
		case 1: Monk[M_BOOK].trotyp = SPE_PROTECTION; break;
		case 2: Monk[M_BOOK].trotyp = SPE_SLEEP; break;
		}
		ini_inv(Monk);
		if(!rn2(5)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Mon);
		break;
	case PM_NECROMANCER:
		switch (rnd(5)) {   
                    case 1: Necromancer[N_BOOK].trotyp = SPE_FORCE_BOLT; break;
                    case 2: Necromancer[N_BOOK].trotyp = SPE_KNOCK; break;
                    case 3: Necromancer[N_BOOK].trotyp = SPE_MAGIC_MISSILE; break;
                    case 4: Necromancer[N_BOOK].trotyp = SPE_CREATE_MONSTER; break;
                    case 5: Necromancer[N_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Necromancer);
		knows_class(SPBOOK_CLASS);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_N);
		break;
	case PM_PRIEST:
		switch (rnd(9)) {   
		    case 1: Priest[P_BOOK].trotyp = SPE_FORCE_BOLT; break;
		    case 2: Priest[P_BOOK].trotyp = SPE_SLEEP; break;
		    case 3: Priest[P_BOOK].trotyp = SPE_RESIST_POISON; break;
		    case 4: Priest[P_BOOK].trotyp = SPE_RESIST_SLEEP; break;
		    case 5: Priest[P_BOOK].trotyp = SPE_DETECT_FOOD; break;
		    case 6: Priest[P_BOOK].trotyp = SPE_DETECT_MONSTERS; break;
		    case 7: Priest[P_BOOK].trotyp = SPE_LIGHT; break;
		    case 8: Priest[P_BOOK].trotyp = SPE_KNOCK; break;
		    case 9: Priest[P_BOOK].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		ini_inv(Priest);
		if(!rn2(10)) ini_inv(Magicmarker);
		else if(!rn2(10)) ini_inv(Lamp);
		knows_object(POT_WATER);
		skill_init(Skill_P);
		/* KMH, conduct --
		 * Some may claim that this isn't agnostic, since they
		 * are literally "priests" and they have holy water.
		 * But we don't count it as such.  Purists can always
		 * avoid playing priests and/or confirm another player's
		 * role in their YAAP.
		 */
		break;
	case PM_RANGER:
#if 0		/* superseded by inv_subs[] */
		switch (rn2(100) / 20) {
		case 0:	/* Special racial bow */
		case 1:
		case 2:
		    switch (Race_switch) {
		    case PM_ELF:
			Ranger[RAN_BOW].trotyp = ELVEN_BOW;
			Ranger[RAN_TWO_ARROWS].trotyp =
			Ranger[RAN_ZERO_ARROWS].trotyp = ELVEN_ARROW;
			break;
		    case PM_GNOME:
			Ranger[RAN_BOW].trotyp = CROSSBOW;
			Ranger[RAN_TWO_ARROWS].trotyp =
			Ranger[RAN_ZERO_ARROWS].trotyp = CROSSBOW_BOLT;
			break;
		    case PM_ORC:
			Ranger[RAN_BOW].trotyp = ORCISH_BOW;
			Ranger[RAN_TWO_ARROWS].trotyp =
			Ranger[RAN_ZERO_ARROWS].trotyp = ORCISH_ARROW;
			break;
		    default: break;	/* Use default bow + arrow */
		    }
		    break;
		case 3:	/* Missiles */
		    Ranger[RAN_BOW].trotyp = BOOMERANG;
		    Ranger[RAN_TWO_ARROWS].trotyp =
		    Ranger[RAN_ZERO_ARROWS].trotyp = DART;
		    break;
		default: break;	/* Use default bow + arrow */
		}
#endif	/*0*/
		Ranger[RAN_TWO_ARROWS].trquan = rn1(10, 50);
		Ranger[RAN_ZERO_ARROWS].trquan = rn1(10, 30);
		ini_inv(Ranger);
		skill_init(Skill_Ran);
		break;
	case PM_ROGUE:
		Rogue[R_DAGGERS].trquan = rn1(10, 6);
		Rogue[R_DARTS].trquan = rn1(10, 25);
#ifdef FIREARMS
		if (rn2(100) < 30) {
			Rogue[R_DAGGERS].trotyp = PISTOL;
			Rogue[R_DAGGERS].trquan = 1;
			Rogue[R_DARTS].trotyp = BULLET;
		}
#endif
		u.ugold = u.ugold0 = rn1(500 ,1500);
		ini_inv(Rogue);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_object(OILSKIN_SACK);
		skill_init(Skill_R);
		break;
	case PM_SAMURAI:
		Samurai[S_ARROWS].trquan = rn1(20, 26);
		ini_inv(Samurai);
		if(!rn2(5)) ini_inv(Blindfold);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_S);
		break;
#ifdef TOURIST
	case PM_TOURIST:
		Tourist[T_DARTS].trquan = rn1(20, 21);
		u.ugold = u.ugold0 = rn1(500,1000);
		ini_inv(Tourist);
		if(!rn2(25)) ini_inv(Tinopener);
		else if(!rn2(25)) ini_inv(Leash);
		else if(!rn2(25)) ini_inv(Towel);
		else if(!rn2(25)) ini_inv(Magicmarker);
		skill_init(Skill_T);
		break;
#endif /* TOURIST */
	case PM_UNDEAD_SLAYER:
		switch (rn2(100) / 25) {
			case 0:	/* Pistol and silver bullets */
#ifdef FIREARMS
		        UndeadSlayer[U_MINOR].trotyp = PISTOL;
		        UndeadSlayer[U_RANGE].trotyp = SILVER_BULLET;
				UndeadSlayer[U_RANGE].trquan = rn1(10, 30);
		        break;
#endif
			case 1:	/* Crossbow and bolts */
				UndeadSlayer[U_MINOR].trotyp = CROSSBOW;
				UndeadSlayer[U_RANGE].trotyp = CROSSBOW_BOLT;
				UndeadSlayer[U_RANGE].trquan = rn1(10, 30);
				UndeadSlayer[U_MISC].trotyp = LOW_BOOTS;
				UndeadSlayer[U_MISC].trspe = 1;
				UndeadSlayer[U_ARMOR].trotyp = LEATHER_JACKET;
				UndeadSlayer[U_ARMOR].trspe = 1;
		        break;
			case 2:	/* Whip and daggers */
		        UndeadSlayer[U_MINOR].trotyp = BULLWHIP;
		        UndeadSlayer[U_MINOR].trspe = 2;
		        break;
			case 3:	/* Silver spear and daggers */
				break;
		}
		ini_inv(UndeadSlayer);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		if(!rn2(6)) ini_inv(Lamp);
		skill_init(Skill_U);
		break;
	case PM_VALKYRIE:
		flags.female = TRUE;
		ini_inv(Valkyrie);
		if(!rn2(6)) ini_inv(Lamp);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_V);
		break;
	case PM_WIZARD:
		switch (rnd(2)) {                
		    case 1: Wizard[W_BOOK1].trotyp = SPE_FORCE_BOLT; break;
		    case 2: Wizard[W_BOOK1].trotyp = SPE_SLEEP; break;
		    default: break;
		}
		switch (rnd(2)) {
		    case 1: Wizard[W_BOOK2].trotyp = SPE_RESIST_POISON; break;
		    case 2: Wizard[W_BOOK2].trotyp = SPE_RESIST_SLEEP; break;
		    default: break;
		}
		switch (rnd(5)) {   
		    case 1: Wizard[W_BOOK3].trotyp = SPE_DETECT_FOOD; break;
		    case 2: Wizard[W_BOOK3].trotyp = SPE_DETECT_MONSTERS; break;
		    case 3: Wizard[W_BOOK3].trotyp = SPE_LIGHT; break;
		    case 4: Wizard[W_BOOK3].trotyp = SPE_KNOCK; break;
		    case 5: Wizard[W_BOOK3].trotyp = SPE_WIZARD_LOCK; break;
		    default: break;
		}
		switch (rnd(9)) {
		    case 1: Wizard[W_BOOK4].trotyp = SPE_MAGIC_MISSILE; break;
		    case 2: Wizard[W_BOOK4].trotyp = SPE_CONFUSE_MONSTER; break;
		    case 3: Wizard[W_BOOK4].trotyp = SPE_SLOW_MONSTER; break;
		    case 4: Wizard[W_BOOK4].trotyp = SPE_CURE_BLINDNESS; break;
		    case 5: Wizard[W_BOOK4].trotyp = SPE_ENDURE_HEAT; break;
		    case 6: Wizard[W_BOOK4].trotyp = SPE_ENDURE_COLD; break;
		    case 7: Wizard[W_BOOK4].trotyp = SPE_INSULATE; break;
		    case 8: Wizard[W_BOOK4].trotyp = SPE_CREATE_MONSTER; break;
		    case 9: Wizard[W_BOOK4].trotyp = SPE_HEALING; break;
		    default: break;
		}
		ini_inv(Wizard);
		knows_class(SPBOOK_CLASS);
		if(!rn2(5)) ini_inv(Magicmarker);
		if(!rn2(5)) ini_inv(Blindfold);
		skill_init(Skill_W);
		break;

#ifdef YEOMAN
	case PM_YEOMAN:
		ini_inv(Yeoman);
		knows_class(WEAPON_CLASS);
		knows_class(ARMOR_CLASS);
		skill_init(Skill_Y);
		break;
#endif

	default:        /* impossible */
		break;
	}

	/*** Race-specific initializations ***/
	switch (Race_switch) {
	case PM_HUMAN:
	    /* Nothing special */
	    break;

	case PM_ELF:
	    /*
	     * Elves are people of music and song, or they are warriors.
	     * Non-warriors get an instrument.  We use a kludge to
	     * get only non-magic instruments.
	     */
	    if (Role_if(PM_PRIEST) || Role_if(PM_WIZARD)) {
		static int trotyp[] = {
		    WOODEN_FLUTE, TOOLED_HORN, WOODEN_HARP,
		    BELL, BUGLE, LEATHER_DRUM
		};
		Instrument[0].trotyp = trotyp[rn2(SIZE(trotyp))];
		ini_inv(Instrument);
	    }

	    /* Elves can recognize all elvish objects */
	    knows_object(ELVEN_SHORT_SWORD);
	    knows_object(ELVEN_ARROW);
	    knows_object(ELVEN_BOW);
	    knows_object(ELVEN_SPEAR);
	    knows_object(ELVEN_DAGGER);
	    knows_object(ELVEN_BROADSWORD);
	    knows_object(ELVEN_MITHRIL_COAT);
	    knows_object(ELVEN_LEATHER_HELM);
	    knows_object(ELVEN_SHIELD);
	    knows_object(ELVEN_BOOTS);
	    knows_object(ELVEN_CLOAK);
	    break;

	case PM_DWARF:
	    /* Dwarves can recognize all dwarvish objects */
	    knows_object(DWARVISH_SPEAR);
	    knows_object(DWARVISH_SHORT_SWORD);
	    knows_object(DWARVISH_MATTOCK);
	    knows_object(DWARVISH_IRON_HELM);
	    knows_object(DWARVISH_MITHRIL_COAT);
	    knows_object(DWARVISH_CLOAK);
	    knows_object(DWARVISH_ROUNDSHIELD);
	    break;

	case PM_GNOME:
	    break;
	case PM_HUMAN_WEREWOLF:
	    u.ulycn = PM_WEREWOLF;
/*	    u.nv_range = 2;
	    u.uen = u.uenmax += 6;
	    ini_inv(Lycanthrope);*/
	    break;

	case PM_ORC:
	    /* compensate for generally inferior equipment */
	    if (!Role_if(PM_WIZARD))
		ini_inv(Xtra_food);
	    /* Orcs can recognize all orcish objects */
	    knows_object(ORCISH_SHORT_SWORD);
	    knows_object(ORCISH_ARROW);
	    knows_object(ORCISH_BOW);
	    knows_object(ORCISH_SPEAR);
	    knows_object(ORCISH_DAGGER);
	    knows_object(ORCISH_CHAIN_MAIL);
	    knows_object(ORCISH_RING_MAIL);
	    knows_object(ORCISH_HELM);
	    knows_object(ORCISH_SHIELD);
	    knows_object(URUK_HAI_SHIELD);
	    knows_object(ORCISH_CLOAK);
	    break;
	case PM_VAMPIRE:
	    /* Vampires start off with gods not as pleased, luck penalty */
	    adjalign(-5); 
	    change_luck(-1);
	    break;
	default:	/* impossible */
		break;
	}
		
	if (discover)
		ini_inv(Wishing);

	u.ugold0 += hidden_gold();      /* in case sack has gold in it */

	find_ac();                      /* get initial ac value */
	temp = rn1(10,70);
	init_attr(temp);                 /* init attribute values */
	max_rank_sz();                  /* set max str size for class ranks */
/*
 *      Do we really need this?
 */
	for(i = 0; i < A_MAX; i++)
	    if(!rn2(20)) {
		register int xd = rn2(7) - 2;   /* biased variation */
		(void) adjattrib(i, xd, TRUE);
		if (ABASE(i) < AMAX(i)) AMAX(i) = ABASE(i);
	    }
#if 0
/* STEPHEN WHITE'S NEW CODE */
	/* give bigger percent strength above 18, instead of just a point */
  
	if (AMAX(A_STR) > 18) {
		temp = (18 + d((AMAX(A_STR) - 18), 10) +
				(2 * (AMAX(A_STR) - 18)));
		temp2 = urole.attrmax[A_STR];
		if (temp > temp2) temp = temp2;
		ABASE(A_STR) = temp;
		AMAX(A_STR) = temp;
	}
#endif
	
	/* make sure you can carry all you have - especially for Tourists */
	while (inv_weight() > 0) {
		if (adjattrib(A_STR, 1, TRUE)) continue;
		if (adjattrib(A_CON, 1, TRUE)) continue;
		/* only get here when didn't boost strength or constitution */
		break;
	}

	return;
}


/* skills aren't initialized, so we use the role-specific skill lists */
static boolean
restricted_spell_discipline(otyp)
int otyp;
{
    struct def_skill *skills;
    int this_skill = spell_skilltype(otyp);

    switch (Role_switch) {
     case PM_ARCHEOLOGIST:	skills = Skill_A; break;
     case PM_BARBARIAN:		skills = Skill_B; break;
     case PM_CAVEMAN:		skills = Skill_C; break;
     case PM_HEALER:		skills = Skill_H; break;
     case PM_KNIGHT:		skills = Skill_K; break;
     case PM_MONK:		skills = Skill_Mon; break;
     case PM_PRIEST:		skills = Skill_P; break;
     case PM_RANGER:		skills = Skill_Ran; break;
     case PM_ROGUE:		skills = Skill_R; break;
     case PM_SAMURAI:		skills = Skill_S; break;
#ifdef TOURIST
     case PM_TOURIST:		skills = Skill_T; break;
#endif
     case PM_VALKYRIE:		skills = Skill_V; break;
     case PM_WIZARD:		skills = Skill_W; break;
     default:			skills = 0; break;	/* lint suppression */
    }

    while (skills->skill != P_NONE) {
	if (skills->skill == this_skill) return FALSE;
	++skills;
    }
    return TRUE;
}

static void
ini_inv(trop)
register struct trobj *trop;
{
	struct obj *obj;
	int otyp, i;

	while (trop->trclass) {
		if (trop->trotyp != UNDEF_TYP) {
			otyp = (int)trop->trotyp;
			if (urace.malenum != PM_HUMAN) {
			    /* substitute specific items for generic ones */
			    for (i = 0; inv_subs[i].race_pm != NON_PM; ++i)
				if (inv_subs[i].race_pm == urace.malenum &&
					otyp == inv_subs[i].item_otyp) {
				    otyp = inv_subs[i].subs_otyp;
				    break;
				}
			}
			obj = mksobj(otyp, TRUE, FALSE);
		} else {	/* UNDEF_TYP */
			static NEARDATA short nocreate = STRANGE_OBJECT;
			static NEARDATA short nocreate2 = STRANGE_OBJECT;
			static NEARDATA short nocreate3 = STRANGE_OBJECT;
			static NEARDATA short nocreate4 = STRANGE_OBJECT;
		/*
		 * For random objects, do not create certain overly powerful
		 * items: wand of wishing, ring of levitation, or the
		 * polymorph/polymorph control combination.  Specific objects,
		 * i.e. the discovery wishing, are still OK.
		 * Also, don't get a couple of really useless items.  (Note:
		 * punishment isn't "useless".  Some players who start out with
		 * one will immediately read it and use the iron ball as a
		 * weapon.)
		 */
			obj = mkobj(trop->trclass, FALSE);
			otyp = obj->otyp;
			while (otyp == WAN_WISHING
				|| otyp == nocreate
				|| otyp == nocreate2
				|| otyp == nocreate3
				|| otyp == nocreate4
#ifdef ELBERETH
				|| otyp == RIN_LEVITATION
#endif
				|| ((Role_if(PM_FLAME_MAGE) || Role_if(PM_ICE_MAGE))
						&&
				    (otyp == RIN_FIRE_RESISTANCE || 
				     otyp == RIN_COLD_RESISTANCE ||
				     otyp == SPE_ENDURE_HEAT ||
				     otyp == SPE_ENDURE_COLD))

				/* KMH -- Hobbits shouldn't get ring of invis. */
				|| (Role_if(PM_HOBBIT) && otyp == RIN_INVISIBILITY)

				/* KMH, balance patch -- now an amulet */
				|| (Role_if(PM_NECROMANCER) &&
						otyp == AMULET_OF_DRAIN_RESISTANCE)
				/* 'useless' or over powerful items */
				|| otyp == POT_HALLUCINATION
				|| otyp == POT_ACID
				|| otyp == SCR_AMNESIA
				|| otyp == SCR_FIRE
				|| otyp == SCR_STINKING_CLOUD
				|| otyp == SCR_BLANK_PAPER
				|| otyp == SPE_BLANK_PAPER
				|| otyp == RIN_AGGRAVATE_MONSTER
				|| otyp == RIN_HUNGER
				|| otyp == WAN_NOTHING
				/* Monks don't use weapons */
				|| (otyp == SCR_ENCHANT_WEAPON &&
				    Role_if(PM_MONK))
				/* wizard patch -- they already have one */
				|| (otyp == SPE_FORCE_BOLT &&
				    Role_if(PM_WIZARD))
				/* powerful spells are either useless to
				   low level players or unbalancing; also
				   spells in restricted skill categories */
				|| (obj->oclass == SPBOOK_CLASS &&
				    (objects[otyp].oc_level > 3 ||
				    restricted_spell_discipline(otyp)))
							) {
				dealloc_obj(obj);
				obj = mkobj(trop->trclass, FALSE);
				otyp = obj->otyp;
			}

			/* Don't start with +0 or negative rings */
			if (objects[otyp].oc_charged && obj->spe <= 0)
				obj->spe = rne(3);

			/* Heavily relies on the fact that 1) we create wands
			 * before rings, 2) that we create rings before
			 * spellbooks, and that 3) not more than 1 object of a
			 * particular symbol is to be prohibited.  (For more
			 * objects, we need more nocreate variables...)
			 */
			switch (otyp) {
			    case WAN_POLYMORPH:
			    case RIN_POLYMORPH:
			    case POT_POLYMORPH:
				nocreate = RIN_POLYMORPH_CONTROL;
				break;
			    case RIN_POLYMORPH_CONTROL:
				nocreate = RIN_POLYMORPH;
				nocreate2 = SPE_POLYMORPH;
				nocreate3 = POT_POLYMORPH;
			}
			/* Don't have 2 of the same ring or spellbook */
			if (obj->oclass == RING_CLASS ||
					obj->oclass == SPBOOK_CLASS)
				nocreate4 = otyp;
		}

		obj->dknown = obj->bknown = obj->rknown = 1;
		if (objects[otyp].oc_uses_known) obj->known = 1;
		obj->cursed = 0;
		if (obj->opoisoned && u.ualign.type != A_CHAOTIC)
		    obj->opoisoned = 0;
		if(obj->oclass == WEAPON_CLASS || obj->oclass == TOOL_CLASS) {
			obj->quan = (long) trop->trquan;
			trop->trquan = 1;
		}
		
		if(trop->trspe != UNDEF_SPE)
			obj->spe = trop->trspe;
		if(trop->trbless != UNDEF_BLESS)
			obj->blessed = trop->trbless;

		/* defined after setting otyp+quan + blessedness */
		obj->owt = weight(obj);
				
		obj = addinv(obj);

		/* Make the type known if necessary */
		if (OBJ_DESCR(objects[otyp]) && obj->known)
			discover_object(otyp, TRUE, FALSE);
		if (otyp == OIL_LAMP)
			discover_object(POT_OIL, TRUE, FALSE);

		if(obj->oclass == ARMOR_CLASS){
			if (is_shield(obj) && !uarms)
				setworn(obj, W_ARMS);
			else if (is_helmet(obj) && !uarmh)
				setworn(obj, W_ARMH);
			else if (is_gloves(obj) && !uarmg)
				setworn(obj, W_ARMG);
#ifdef TOURIST
			else if (is_shirt(obj) && !uarmu)
				setworn(obj, W_ARMU);
#endif
			else if (is_cloak(obj) && !uarmc)
				setworn(obj, W_ARMC);
			else if (is_boots(obj) && !uarmf)
				setworn(obj, W_ARMF);
			else if (is_suit(obj) && !uarm)
				setworn(obj, W_ARM);
		}

		if (obj->oclass == WEAPON_CLASS || is_weptool(obj) ||
			otyp == TIN_OPENER || otyp == FLINT || otyp == ROCK) {
		    if (is_ammo(obj) || is_missile(obj)) {
			if (!uquiver) setuqwep(obj);
		    } else if (!uwep) setuwep(obj);
		    else if (!uswapwep) setuswapwep(obj);
		}
		if (obj->oclass == SPBOOK_CLASS &&
				obj->otyp != SPE_BLANK_PAPER)
		    initialspell(obj);

#if !defined(PYRAMID_BUG) && !defined(MAC)
		if(--trop->trquan) continue;    /* make a similar object */
#else
		if(trop->trquan) {              /* check if zero first */
			--trop->trquan;
			if(trop->trquan)
				continue;       /* make a similar object */
		}
#endif
		
		trop++;
	}
}

/*u_init.c*/