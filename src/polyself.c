/*	SCCS Id: @(#)polyself.c	3.3	2000/07/14	*/
/*      Copyright (C) 1987, 1988, 1989 by Ken Arromdee */
/* NetHack may be freely redistributed.  See license for details. */

/*
 * Polymorph self routine.
 *
 * Note:  the light source handling code assumes that both youmonst.m_id
 * and youmonst.mx will always remain 0 when it handles the case of the
 * player polymorphed into a light-emitting monster.
 */

#include "hack.h"

#ifdef OVLB
STATIC_DCL void FDECL(polyman, (const char *,const char *));
STATIC_DCL void NDECL(break_armor);
STATIC_DCL void FDECL(drop_weapon,(int));
STATIC_DCL void NDECL(skinback);
STATIC_DCL void NDECL(uunstick);
STATIC_DCL int FDECL(armor_to_dragon,(int));
STATIC_DCL void NDECL(newman);
STATIC_DCL void NDECL(merge_with_armor);

/*  Not Used 
static void NDECL(special_poly);
*/

/* Assumes u.umonster is set up already */
/* Use u.umonster since we might be restoring and you may be polymorphed */
void
init_uasmon()
{
	int i;

	upermonst = mons[u.umonster];

	/* Fix up the flags */
	/* Default flags assume human,  so replace with your race's flags */

	upermonst.mflags1 &= ~(mons[PM_HUMAN].mflags1);
	upermonst.mflags1 |= (mons[urace.malenum].mflags1);

	upermonst.mflags2 &= ~(mons[PM_HUMAN].mflags2);
	upermonst.mflags2 |= (mons[urace.malenum].mflags2);

	upermonst.mflags3 &= ~(mons[PM_HUMAN].mflags2);
	upermonst.mflags3 |= (mons[urace.malenum].mflags2);
	
	/* Fix up the attacks */
	for(i = 0; i < NATTK; i++) {
	    upermonst.mattk[i] = mons[urace.malenum].mattk[i];
	}
	
	set_uasmon();
}

/* update the youmonst.data structure pointer */
void
set_uasmon()
{
	set_mon_data(&youmonst, ((u.umonnum == u.umonster) ? 
					&upermonst : &mons[u.umonnum]), 0);
}

/* make a (new) human out of the player */
STATIC_OVL void
polyman(fmt, arg)
const char *fmt, *arg;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_mimicking_gold = (youmonst.m_ap_type == M_AP_OBJECT
				      && youmonst.mappearance == GOLD_PIECE);
	boolean was_blind = !!Blind;

	if (Upolyd) {
		u.acurr = u.macurr;     /* restore old attribs */
		u.amax = u.mamax;
		u.umonnum = u.umonster;
		flags.female = u.mfemale;
	}

	set_uasmon();

	u.mh = u.mhmax = 0;
	u.mtimedone = 0;
	skinback();
	u.uundetected = 0;
	newsym(u.ux,u.uy);

	if (sticky) uunstick();
	find_ac();
	if (was_mimicking_gold) {
		if (multi < 0) unmul("");
	} else {
	    /*
	     * Clear any in-progress imitations -- the case where not a
	     * mimic is handled above.
	     *
	     * Except, this is not complete if the hero ever gets the
	     * chance to imitate anything, then s/he may be mimicing
	     * gold, but not the way its done for eating a mimic.
	     */
	    youmonst.m_ap_type = M_AP_NOTHING;
	}
	newsym(u.ux,u.uy);

	
	You(fmt, arg);
	/* check whether player foolishly genocided self while poly'd */
	if ((mvitals[urole.malenum].mvflags & G_GENOD) ||
			(urole.femalenum != NON_PM &&
			(mvitals[urole.femalenum].mvflags & G_GENOD)) ||
			(mvitals[urace.malenum].mvflags & G_GENOD) ||
			(urace.femalenum != NON_PM &&
			(mvitals[urace.femalenum].mvflags & G_GENOD))) {
	    /* intervening activity might have clobbered genocide info */
	    killer = delayed_killer;
	    if (!killer || !strstri(killer, "genocid")) {
			killer_format = KILLED_BY;
			killer = "self-genocide";
	    }
	    done(GENOCIDED);
	}
	if (was_blind && !Blind) {	/* reverting from eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}

	if(!Levitation && !u.ustuck &&
	    (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy)))
		spoteffects(TRUE);

	see_monsters();
}

void
change_sex()
{
	/* setting u.umonster for caveman/cavewoman or priest/priestess
	   swap unintentionally makes `Upolyd' appear to be true */
	boolean already_polyd = (boolean) Upolyd;

	/* Some monsters are always of one sex and their sex can't be changed */
	/* succubi/incubi are handled below */
	if (u.umonnum != PM_SUCCUBUS && u.umonnum != PM_INCUBUS && !is_male(youmonst.data) && !is_female(youmonst.data) && !is_neuter(youmonst.data))
	    flags.female = !flags.female;
	if (already_polyd)	/* poly'd: also change saved sex */
	    u.mfemale = !u.mfemale;
	max_rank_sz();		/* [this appears to be superfluous] */
	if (flags.female && urole.name.f)
	    Strcpy(pl_character, urole.name.f);
	else
	    Strcpy(pl_character, urole.name.m);
	u.umonster = ((already_polyd ? u.mfemale : flags.female) && urole.femalenum != NON_PM) ?
			urole.femalenum : urole.malenum;

	if (!already_polyd) {
	    u.umonnum = u.umonster;
	} else if (u.umonnum == PM_SUCCUBUS || u.umonnum == PM_INCUBUS) {
	    /* change monster type to match new sex */
	    u.umonnum = (u.umonnum == PM_SUCCUBUS) ? PM_INCUBUS : PM_SUCCUBUS;
	    set_uasmon();
	}
}

STATIC_OVL void
newman()
{
	int     basehp, hp, bonushp, energy, energybase, tmp;        
	/* STEPHEN WHITE'S NEW CODE */
	if (!rn2(10)) change_sex();

	switch  (Role_switch) {        
		case PM_ARCHEOLOGIST:
			energybase = 6;
			basehp = 13;  
			bonushp = 1;  
			break;
		case PM_BARBARIAN:
			energybase = 2;
			basehp = 16;  
			bonushp = 3;            
			break;
		case PM_CAVEMAN:
			energybase = 2;
			basehp = 16;  
			bonushp = 3;            
			break;
		case PM_DOPPELGANGER:
			energybase = 7;
			basehp = 12;
			bonushp = 1;  
			break;
		case PM_ELF: case PM_DROW:
			energybase = 7;
			basehp = 15;
			bonushp = 2;
			break;
		case PM_FLAME_MAGE:
			energybase = 10;
			basehp = 12;
			bonushp = 1;
			break;
		case PM_GNOME:
			energybase = 7;
			basehp = 15;
			bonushp = 2;
			break;
		case PM_HEALER:
			energybase = 10;
			basehp = 13;
			bonushp = 1;
			break;
		case PM_ICE_MAGE:
			energybase = 10;
			basehp = 12;
			bonushp = 1;
			break;
#ifdef YEOMAN
		case PM_YEOMAN:
#endif
		case PM_KNIGHT:
			energybase = 3;
			basehp = 16;
			bonushp = 3;
			break;
		case PM_MONK:
			energybase = 7;
			basehp = 14;
			bonushp = 2;
			break;
		case PM_NECROMANCER:
			energybase = 10;
			basehp = 12;  
			bonushp = 1;  
			break;
		case PM_PRIEST:
			energybase = 10;
			basehp = 14;
			bonushp = 2;            
			break;
		case PM_ROGUE:
			energybase = 6;
			basehp = 12;  
			bonushp = 2;            
			break;
		case PM_SAMURAI:
			energybase = 2;
			basehp = 15;  
			bonushp = 3;            
			break;
#ifdef TOURIST        
		case PM_TOURIST:
			energybase = 6;
			basehp = 10;  
			bonushp = 1;
			break;
#endif        
		case PM_UNDEAD_SLAYER:
			energybase = 3;
			basehp = 16;
			bonushp = 3;
			break;
		case PM_VALKYRIE:
			energybase = 2;
			basehp = 16;  
			bonushp = 3;            
			break;
		case PM_WIZARD:
			energybase = 10;
			basehp = 12;  
			bonushp = 1;  
			break;
		case PM_HUMAN_WEREWOLF:
		default:
			energybase = 2;
			basehp = 12;
			bonushp = 1;            
			break;
	}

	if (!Race_if(PM_DOPPELGANGER)) {
		tmp = u.ulevel;
		u.ulevel = u.ulevel + rn1(5, -2);
		if (u.ulevel > 127 || u.ulevel < 1) u.ulevel = 1;
		if (u.ulevel > MAXULEV) u.ulevel = MAXULEV;
	if (u.ulevelmax < u.ulevel) u.ulevelmax = u.ulevel;

		adjabil(tmp, (int)u.ulevel);
		reset_rndmonst(NON_PM); /* new monster generation criteria */

		/* random experience points for the new experience level */
		u.uexp = rndexp();
	}

	/* u.uhpmax * u.ulevel / tmp2: proportionate hit points to new level
	 * -10 and +10: don't apply proportionate HP to 10 of a starting
	 *   character's hit points (since a starting character's hit points
	 *   are not on the same scale with hit points obtained through level
	 *   gain)
	 * 9 - rn2(19): random change of -9 to +9 hit points
	 */
	if (!Race_if(PM_DOPPELGANGER) && rn2(4)) {
		if (u.ulevel > 1)
		  	hp = basehp + d((u.ulevel-1),8) + bonushp * u.ulevel;
		else  hp = basehp + bonushp;

		u.uhpmax = hp + 7 * u.ulevel;
		u.uhp = u.uhpmax;

		if (u.ulevel > 1) 
			energy = d(u.ulevel, energybase) + 2 * rnd(5);
		else  energy = 2 * rnd(6);                
		u.uenmax = energy + 10 * u.ulevel;
		u.uen = energy;
	}
	redist_attr();
	u.uhunger = rn1(500,500);
	newuhs(FALSE);
	if (Sick) make_sick(0L, (char *) 0, FALSE, SICK_ALL);
	Sick = 0;
	Stoned = 0;
	delayed_killer = 0;
	if (Race_if(PM_DOPPELGANGER)) {        
		if (u.uhp <= 10) u.uhp = 10;
		if (u.uhpmax <= 10) u.uhpmax = 10;
		if (u.uen <= u.ulevel) u.uen = u.ulevel;
		if (u.uenmax <= u.ulevel) u.uenmax = u.ulevel;
	}
	if (u.uhp <= 0 || u.uhpmax <= 0) {
		if (Polymorph_control) {
			if (u.uhp <= 0) u.uhp = 1;
			if (u.uhpmax <= 0) u.uhpmax = 1;
		} else {
			Your("new form doesn't seem healthy enough to survive.");
			killer_format = KILLED_BY_AN;
			killer="unsuccessful polymorph";
			done(DIED);
		}
	}
	polyman("feel like a new %s!",
		(flags.female && urace.individual.f) ? urace.individual.f :
		(urace.individual.m) ? urace.individual.m : urace.noun);
	if (Slimed) {
		Your("body transforms, but there is still slime on you.");
		Slimed = 10L;
	}
	flags.botl = 1;
	vision_full_recalc = 1;
	(void) encumber_msg();
	see_monsters();
}

void
polyself()
{
	char buf[BUFSZ];
	int old_light, new_light;
	int mntmp = NON_PM;
	int tries=0;
	boolean draconian = (uarm &&
				uarm->otyp >= GRAY_DRAGON_SCALE_MAIL &&
				uarm->otyp <= YELLOW_DRAGON_SCALES);
	boolean iswere = (u.ulycn >= LOW_PM || is_were(youmonst.data));
	boolean isvamp = (is_vampire(youmonst.data));

	/* [Tom] I made the chance of dying from Con check only possible for
		 really weak people (it was out of 20) */

	if(!Polymorph_control && !draconian && !iswere && !isvamp &&
			!Race_if(PM_DOPPELGANGER)) {
		if (rn2(12) > ACURR(A_CON)) {
			You(shudder_for_moment);
			losehp(rnd(30), "system shock", KILLED_BY_AN);
			exercise(A_CON, FALSE);
			return;
		}
	}
	old_light = Upolyd ? emits_light(youmonst.data) : 0;

	if (Polymorph_control) {
		do {
			getlin("Become what kind of monster? [type the name]",
				buf);
			mntmp = name_to_mon(buf);
			if (mntmp < LOW_PM)
				pline("I've never heard of such monsters.");
			/* Note:  humans are illegal as monsters, but an
			 * illegal monster forces newman(), which is what we
			 * want if they specified a human.... */
			else if (!polyok(&mons[mntmp]) && !your_race(&mons[mntmp]))
				You("cannot polymorph into that.");
			else break;
		} while(++tries < 5);
		if (tries==5) pline(thats_enough_tries);
		/* allow skin merging, even when polymorph is controlled */
		if (draconian &&
		    (mntmp == armor_to_dragon(uarm->otyp) || tries == 5))
		    goto do_merge;
	} else if (Race_if(PM_DOPPELGANGER)) {
		/* Not an experienced Doppelganger yet */
		do {
			/* Slightly different wording */
			getlin("Attempt to become what kind of monster? [type the name]",
				buf);
			mntmp = name_to_mon(buf);
			if (mntmp < LOW_PM)
				pline("I've never heard of such monsters.");
			/* Note:  humans are illegal as monsters, but an
			 * illegal monster forces newman(), which is what we
			 * want if they specified a human.... */
			else if (!polyok(&mons[mntmp]) && !your_race(&mons[mntmp]))
				You("cannot polymorph into that.");
#ifdef EATEN_MEMORY
			else if (!mvitals[mntmp].eaten) {
				You("attempt an unfamiliar polymorph.");
				if ((rn2(5) + u.ulevel) < mons[mntmp].mlevel)
				    mntmp = LOW_PM - 1; /* Didn't work for sure */
				/* Either way, give it a shot */
				break;
			}
#else
			else if (rn2((u.ulevel + 25)) < 20) {
				mntmp = LOW_PM - 1;
				break;
			}
#endif

			else break;
		} while(++tries < 5);
		if (tries==5) pline(thats_enough_tries);
		/* allow skin merging, even when polymorph is controlled */
		if (draconian &&
		    (mntmp == armor_to_dragon(uarm->otyp) || tries == 5))
		    goto do_merge;
	} else if (draconian || iswere || isvamp) {
		/* special changes that don't require polyok() */
		if (draconian) {
		    do_merge:
			mntmp = armor_to_dragon(uarm->otyp);

			if (!(mvitals[mntmp].mvflags & G_GENOD)) {
				/* Code that was here is now in merge_with_armor */
				merge_with_armor();
			}
		} else if (iswere) {
			if (is_were(youmonst.data))
				mntmp = PM_HUMAN; /* Illegal; force newman() */
			else
				mntmp = u.ulycn;
		} else if (isvamp) {
			if (u.umonnum != PM_VAMPIRE_BAT)
				mntmp = PM_VAMPIRE_BAT;
			else
				mntmp = PM_VAMPIRE;
		}
		/* if polymon fails, "you feel" message has been given
		   so don't follow up with another polymon or newman */
		if (mntmp == PM_HUMAN) newman();	/* werecritter */
		else (void) polymon(mntmp);
		goto made_change;    /* maybe not, but this is right anyway */
	}
	if (mntmp < LOW_PM) {
		tries = 0;
		do {
			/* randomly pick an "ordinary" monster */
			mntmp = rn1(SPECIAL_PM - LOW_PM, LOW_PM);
		} while((!polyok(&mons[mntmp]) || is_placeholder(&mons[mntmp]))
				&& tries++ < 200);
	}

	/* The below polyok() fails either if everything is genocided, or if
	 * we deliberately chose something illegal to force newman().
	 */
        /* Also catches player gnomes going to gnomes */
        /* WAC Doppelgangers go through a 1/20 check rather than 1/5 */
        if (!polyok(&mons[mntmp]) || your_race(&mons[mntmp]) ||
        		(Race_if(PM_DOPPELGANGER) ? (
        			((u.ulevel < mons[mntmp].mlevel)
#ifdef EATEN_MEMORY
        			 || !mvitals[mntmp].eaten
#endif
        			 ) && !rn2(20)) : 
				   !rn2(5)))
		newman();
	else if(!polymon(mntmp)) return;

	if (!uarmg) selftouch("No longer petrify-resistant, you");

 made_change:
	new_light = Upolyd ? emits_light(youmonst.data) : 0;
	if (old_light != new_light) {
	    if (old_light)
		del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
	    if (new_light == 1) ++new_light;  /* otherwise it's undetectable */
	    if (new_light)
		new_light_source(u.ux, u.uy, new_light,
				 LS_MONSTER, (genericptr_t)&youmonst);
	}
}

/* (try to) make a mntmp monster out of the player */
int
polymon(mntmp)  /* returns 1 if polymorph successful */
int     mntmp;
{
	boolean sticky = sticks(youmonst.data) && u.ustuck && !u.uswallow,
		was_blind = !!Blind, dochange = FALSE;
	int mlvl;

	if (mvitals[mntmp].mvflags & G_GENOD) { /* allow G_EXTINCT */
		You_feel("rather %s-ish.",mons[mntmp].mname);
		exercise(A_WIS, TRUE);
		return(0);
	}

	/* STEPHEN WHITE'S NEW CODE */        
  
	/* If your an Undead Slayer, you can't become undead! */
  
	if (is_undead(&mons[mntmp]) && Role_if(PM_UNDEAD_SLAYER)) {
		if (Polymorph_control) { 
			You("hear a voice boom out: \"How dare you take such a form!\"");
			u.ualign.record -= 5;
#ifdef NOARTIFACTWISH
			u.usacrifice = 0;
#endif
			exercise(A_WIS, FALSE);
		 } else {
			You("start to change into %s, but a voice booms out:", an(mons[mntmp].mname));
			pline("\"No, I will not allow such a change!\"");
		 }
		 return(0);
	}

	/* KMH, conduct */
	u.uconduct.polyselfs++;

	if (!Upolyd) {
		/* Human to monster; save human stats */
		u.macurr = u.acurr;
		u.mamax = u.amax;
		u.mfemale = flags.female;
	} else {
		/* Monster to monster; restore human stats, to be
		 * immediately changed to provide stats for the new monster
		 */
		u.acurr = u.macurr;
		u.amax = u.mamax;
		flags.female = u.mfemale;
	}

	if (youmonst.m_ap_type == M_AP_OBJECT &&
		youmonst.mappearance == GOLD_PIECE) {
	    /* stop mimicking gold immediately */
	    if (multi < 0) unmul("");
	}
	if (is_male(&mons[mntmp])) {
		if(flags.female) dochange = TRUE;
	} else if (is_female(&mons[mntmp])) {
		if(!flags.female) dochange = TRUE;
	} else if (!is_neuter(&mons[mntmp]) && mntmp != u.ulycn) {
		if(!rn2(10)) dochange = TRUE;
	}
	if (dochange) {
		flags.female = !flags.female;
		You("%s %s%s!",
		    (u.umonnum != mntmp) ? "turn into a" : "feel like a new",
		    (is_male(&mons[mntmp]) || is_female(&mons[mntmp])) ? "" :
			flags.female ? "female " : "male ",
		    mons[mntmp].mname);
	} else {
		if (u.umonnum != mntmp)
			You("turn into %s!", an(mons[mntmp].mname));
		else
			You_feel("like a new %s!", mons[mntmp].mname);
	}
	if (Stoned && poly_when_stoned(&mons[mntmp])) {
		/* poly_when_stoned already checked stone golem genocide */
		You("turn to stone!");
		mntmp = PM_STONE_GOLEM;
		Stoned = 0;
		delayed_killer = 0;
	}

	u.mtimedone = rn1(500, 500);
	u.umonnum = mntmp;
	set_uasmon();

	/* New stats for monster, to last only as long as polymorphed.
	 * Currently only strength gets changed.
	 */
	if(strongmonst(&mons[mntmp])) ABASE(A_STR) = AMAX(A_STR) = STR18(100);

	if (Stone_resistance && Stoned) { /* parnes@eniac.seas.upenn.edu */
		Stoned = 0;
		delayed_killer = 0;
		You("no longer seem to be petrifying.");
	}
	if (Sick_resistance && Sick) {
		make_sick(0L, (char *) 0, FALSE, SICK_ALL);
		You("no longer feel sick.");
	}
	if (Slimed) {
	    if (mntmp == PM_FIRE_VORTEX || mntmp == PM_FIRE_ELEMENTAL) {
		pline_The("slime burns away!");
		Slimed = 0;
	    } else if (mntmp == PM_GREEN_SLIME) {
		/* do it silently */
		Slimed = 0;
	    }
	}
	if (nohands(youmonst.data)) Glib = 0;

	/*
	mlvl = adj_lev(&mons[mntmp]);
	 * We can't do the above, since there's no such thing as an
	 * "experience level of you as a monster" for a polymorphed character.
	 */
	mlvl = ((mntmp == u.ulycn) ? u.ulevel : (int)mons[mntmp].mlevel);
	if (youmonst.data->mlet == S_DRAGON && mntmp >= PM_GRAY_DRAGON) {
		u.mhmax = In_endgame(&u.uz) ? (8*mlvl) : (4*mlvl + d(mlvl,4));
	} else if (is_golem(youmonst.data)) {
		u.mhmax = golemhp(mntmp);
	} else {
		if (!mlvl) u.mhmax = rnd(4);
		else u.mhmax = d(mlvl, 8);
		if (is_home_elemental(&mons[mntmp])) u.mhmax *= 3;
	}
	u.mh = u.mhmax;

	if (u.ulevel < mlvl) {
	/* Low level characters can't become high level monsters for long */
#ifdef DUMB
		/* DRS/NS 2.2.6 messes up -- Peter Kendell */
		int mtd = u.mtimedone, ulv = u.ulevel;

		u.mtimedone = mtd * ulv / mlvl;
#else
		u.mtimedone = u.mtimedone * u.ulevel / mlvl;
#endif
	}

#ifdef EATEN_MEMORY
	/* WAC Doppelgangers can stay much longer in a form they know well */
	if (Race_if(PM_DOPPELGANGER) && mvitals[mntmp].eaten) {
		u.mtimedone *= 2;
		u.mtimedone += mvitals[mntmp].eaten;
	}
#endif

	if (uskin && mntmp != armor_to_dragon(uskin->otyp))
		skinback();
	break_armor();
	drop_weapon(1);
	if (hides_under(youmonst.data))
		u.uundetected = OBJ_AT(u.ux, u.uy);
	else if (youmonst.data->mlet == S_EEL)
		u.uundetected = is_pool(u.ux, u.uy);
	else
		u.uundetected = 0;

	if (was_blind && !Blind) {	/* previous form was eyeless */
	    Blinded = 1L;
	    make_blinded(0L, TRUE);	/* remove blindness */
	}
	newsym(u.ux,u.uy);              /* Change symbol */

	if (!sticky && !u.uswallow && u.ustuck && sticks(youmonst.data)) u.ustuck = 0;
	else if (sticky && !sticks(youmonst.data)) uunstick();
#ifdef STEED
	if (u.usteed) {
	    if (touch_petrifies(u.usteed->data) &&
	    		!Stone_resistance && rnl(3)) {
	    	char buf[BUFSZ];

	    	pline("No longer petrifying-resistant, you touch %s.",
	    			mon_nam(u.usteed));
	    	Sprintf(buf, "riding %s", an(u.usteed->data->mname));
	    	instapetrify(buf);
 	    }
	    if (!can_ride(u.usteed)) dismount_steed(DISMOUNT_POLY);
	}
#endif

	if (flags.verbose) {
	    static const char use_thec[] = "Use the command #%s to %s.";
	    static const char monsterc[] = "monster";
	    if (can_breathe(youmonst.data))
		pline(use_thec,monsterc,"use your breath weapon");
	    if (attacktype(youmonst.data, AT_SPIT))
		pline(use_thec,monsterc,"spit venom");
	    if (attacktype(youmonst.data, AT_GAZE))
			pline(use_thec,monsterc,"gaze");
	    if (youmonst.data->mlet == S_NYMPH)
		pline(use_thec,monsterc,"remove an iron ball");
	    if (youmonst.data->mlet == S_UMBER)
		pline(use_thec,monsterc,"confuse monsters");
	    if (is_hider(youmonst.data))
		pline(use_thec,monsterc,"hide");
	    if (is_were(youmonst.data))
		pline(use_thec,monsterc,"summon help");
	    if (webmaker(youmonst.data))
		pline(use_thec,monsterc,"spin a web");
	    if (u.umonnum == PM_GREMLIN)
		pline(use_thec,monsterc,"multiply in a fountain");
	    if (is_unicorn(youmonst.data))
		pline(use_thec,monsterc,"use your horn");
	    if (is_mind_flayer(youmonst.data))
		pline(use_thec,monsterc,"emit a mental blast");
	    if (youmonst.data->msound == MS_SHRIEK) /* worthless, actually */
		pline(use_thec,monsterc,"shriek");
	    if (lays_eggs(youmonst.data) && flags.female)
		pline(use_thec,"sit","lay an egg");
	}
	/* you now know what an egg of your type looks like */
	if (lays_eggs(youmonst.data)) {
	    /* make queen bees recognize killer bee eggs */
	    learn_egg_type(egg_type_from_parent(u.umonnum, TRUE));
	}
	find_ac();
	if((!Levitation && !u.ustuck && !Flying &&
	    (is_pool(u.ux,u.uy) || is_lava(u.ux,u.uy))) ||
	   (Underwater && !Swimming))
	    spoteffects(TRUE);
	if (Passes_walls && u.utrap && u.utraptype == TT_INFLOOR) {
	    u.utrap = 0;
	    pline_The("rock seems to no longer trap you.");
	} else if (likes_lava(youmonst.data) && u.utrap && u.utraptype == TT_LAVA) {
	    u.utrap = 0;
	    pline_The("lava now feels soothing.");
	}
	if (amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data)) {
	    if (Punished) {
		You("slip out of the iron chain.");
		unpunish();
	    }
	}
	if (u.utrap && (u.utraptype == TT_WEB || u.utraptype == TT_BEARTRAP) &&
		(amorphous(youmonst.data) || is_whirly(youmonst.data) || unsolid(youmonst.data) ||
		  (youmonst.data->msize <= MZ_SMALL && u.utraptype == TT_BEARTRAP))) {
	    You("are no longer stuck in the %s.",
		    u.utraptype == TT_WEB ? "web" : "bear trap");
	    /* probably should burn webs too if PM_FIRE_ELEMENTAL */
	    u.utrap = 0;
	}
	if (webmaker(youmonst.data) && u.utrap && u.utraptype == TT_WEB) {
	    You("orient yourself on the web.");
	    u.utrap = 0;
	}
	flags.botl = 1;
	vision_full_recalc = 1;
	see_monsters();
	exercise(A_CON, FALSE);
	exercise(A_WIS, TRUE);
	(void) encumber_msg();
	return(1);
}

STATIC_OVL void
break_armor()
{
    register struct obj *otmp;
    boolean controlled_change = (Race_if(PM_DOPPELGANGER) || 
    		(Race_if(PM_HUMAN_WEREWOLF) && u.umonnum == PM_WEREWOLF));

    if (breakarm(youmonst.data)) {
	if ((otmp = uarm) != 0) {
	    if(otmp->oartifact) {
		if (donning(otmp)) cancel_don();
		Your("armor falls off!");
		(void) Armor_gone();
		dropx(otmp); /*WAC Drop instead of destroy*/
	    } else if (controlled_change && !otmp->cursed) {
		if (donning(otmp)) cancel_don();
		You("quickly remove your armor as you start to change.");
		(void) Armor_gone();
		dropx(otmp); /*WAC Drop instead of destroy*/
	    } else {
		if (donning(otmp)) cancel_don();
		You("break out of your armor!");
		exercise(A_STR, FALSE);
		(void) Armor_gone();
		useup(otmp);
	    }
	}
	if ((otmp = uarmc) != 0) {
	    if(otmp->oartifact) {
		Your("cloak falls off!");
		(void) Cloak_off();
		dropx(otmp);
	    } else if (controlled_change && !otmp->cursed) {
		You("remove your cloak before you transform.");
		(void) Cloak_off();
		dropx(otmp);
	    } else {
		Your("cloak tears apart!");
		(void) Cloak_off();
		useup(otmp);
	    }
	}
#ifdef TOURIST
	if ((otmp = uarmu) != 0) {
		if (controlled_change && !otmp->cursed) {
			You("take off your shirt just before it starts to rip.");
			setworn((struct obj *)0, otmp->owornmask & W_ARMU);
			dropx(otmp);
		} else {                
		Your("shirt rips to shreds!");
		useup(uarmu);
	}
	}
#endif
    } else if (sliparm(youmonst.data)) {
	if ((otmp = uarm) != 0) {
		if (donning(otmp)) cancel_don();
		Your("armor falls around you!");
		(void) Armor_gone();
		dropx(otmp);
	}
	if ((otmp = uarmc) != 0) {
		if (is_whirly(youmonst.data))
			Your("cloak falls, unsupported!");
		else You("shrink out of your cloak!");
		(void) Cloak_off();
		dropx(otmp);
	}
#ifdef TOURIST
	if ((otmp = uarmu) != 0) {
		if (is_whirly(youmonst.data))
			You("seep right through your shirt!");
		else You("become much too small for your shirt!");
		setworn((struct obj *)0, otmp->owornmask & W_ARMU);
		dropx(otmp);
	}
#endif
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data)) {
	if ((otmp = uarmg) != 0) {
	    if (donning(otmp)) cancel_don();
	    /* Drop weapon along with gloves */
	    You("drop your gloves%s!", uwep ? " and weapon" : "");
	    drop_weapon(0);
	    (void) Gloves_off();
	    dropx(otmp);
	}
	if ((otmp = uarms) != 0) {
	    You("can no longer hold your shield!");
	    (void) Shield_off();
	    dropx(otmp);
	}
	if ((otmp = uarmh) != 0) {
	    if (donning(otmp)) cancel_don();
	    Your("helmet falls to the %s!", surface(u.ux, u.uy));
	    (void) Helmet_off();
	    dropx(otmp);
	}
    }
    if (nohands(youmonst.data) || verysmall(youmonst.data) ||
		slithy(youmonst.data) || youmonst.data->mlet == S_CENTAUR) {
	if ((otmp = uarmf) != 0) {
	    if (donning(otmp)) cancel_don();
	    if (is_whirly(youmonst.data))
		Your("boots fall away!");
	    else Your("boots %s off your feet!",
			verysmall(youmonst.data) ? "slide" : "are pushed");
	    (void) Boots_off();
	    dropx(otmp);
	}
    }
}

STATIC_OVL void
drop_weapon(alone)
int alone;
{
    struct obj *otmp;
    if ((otmp = uwep) != 0) {
	/* !alone check below is currently superfluous but in the
	 * future it might not be so if there are monsters which cannot
	 * wear gloves but can wield weapons
	 */
	if (!alone || cantwield(youmonst.data)) {
	    struct obj *wep = uwep;

	    if (alone) You("find you must drop your weapon%s!",
			   	u.twoweap ? "s" : "");
	    uwepgone();
	    if (!wep->cursed || wep->otyp != LOADSTONE)
		dropx(otmp);
		untwoweapon();
	}
    }
}

void
rehumanize()
{
	boolean forced = (u.mh < 1);
	
	/* KMH, balance patch -- you can't revert back while unchanging */
	if (Unchanging && forced) {
		killer_format = NO_KILLER_PREFIX;
		killer = "killed while stuck in creature form";
		done(DIED);
	}
  
	if (emits_light(youmonst.data))
	    del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
	polyman("return to %s form!", urace.adj);

	if (u.uhp < 1) {
	    char kbuf[256];
	    
	    Sprintf(kbuf, "reverting to unhealthy %s form", urace.adj);
	    killer_format = KILLED_BY;
	    killer = kbuf;
	    done(DIED);
	}
	
	if (forced || (!Race_if(PM_DOPPELGANGER) && (rn2(20) > ACURR(A_CON)))) { 
	/* Exhaustion for "forced" rehumaization & must pass con chack for 
    	 * non-doppelgangers 
    	 * Don't penalize doppelgangers/polymorph running out */
    	 
   	/* WAC Polymorph Exhaustion 1/2 HP to prevent abuse */
	    u.uhp = (u.uhp/2) + 1;
	}

	if (!uarmg) selftouch("No longer petrify-resistant, you");
	nomul(0);

	flags.botl = 1;
	vision_full_recalc = 1;
	(void) encumber_msg();
}


/* WAC -- MUHAHAHAAHA - Gaze attacks! 
 * Note - you can only gaze at one monster at a time, to keep this 
 * from getting out of hand ;B  Also costs 20 energy.
 */
int
dogaze()
{
	coord cc;
	struct monst *mtmp;

	if (Blind) {
		You("can't see a thing!");
		return(0);
	}
	if (u.uen < 20) {
		You("lack the energy to use your special gaze!");
		return(0);
	}
	pline("Where do you wish to look?");
	cc.x = u.ux;
	cc.y = u.uy;
	getpos(&cc, TRUE, "the spot to look");

	if (cc.x == -10) return (0); /* user pressed esc */

	mtmp = m_at(cc.x, cc.y);

	if (!mtmp || !canseemon(mtmp)) {
		You("don't see a monster there!");
		return (0);
	}


	if ((flags.safe_dog && !Confusion && !Hallucination
		  && mtmp->mtame) || (flags.confirm && mtmp->mpeaceful 
		  && !Confusion && !Hallucination)) {
		  	char qbuf[QBUFSZ];
		  	
			Sprintf(qbuf, "Really gaze at %s?", mon_nam(mtmp));
			if (yn(qbuf) != 'y') return (0);
			if (mtmp->mpeaceful) setmangry(mtmp);
	}

	u.uen -= 20;

	You("gaze at %s...", mon_nam(mtmp));

	if ((mtmp->data==&mons[PM_MEDUSA]) && !mtmp->mcan) {
		pline("Gazing at the awake Medusa is not a very good idea.");
		/* as if gazing at a sleeping anything is fruitful... */
		You("turn to stone...");
		killer_format = KILLED_BY;
		killer = "deliberately gazing at Medusa's hideous countenance";
		done(STONING);
	} else if (!mtmp->mcansee || mtmp->msleeping) {
	    pline("But nothing happens.");
	    return (1);
	} else if (Invis && !perceives(mtmp->data)) {
	    pline("%s seems not to notice your gaze.", Monnam(mtmp));
	    return (1);
	} else {
	    register struct attack *mattk;
	    register int i;

	    for(i = 0; i < NATTK; i++) {
		mattk = &(youmonst.data->mattk[i]);
		if(mattk->aatyp == AT_GAZE) break;
	    }
	    damageum(mtmp, mattk);
	}
	return(1);
}

int
dobreathe()
{
	int energy = 0;


	if (Strangled) {
	    You_cant("breathe.  Sorry.");
	    return(0);
	}

	/* WAC -- no more belching.  Use up energy instead */
	if (Race_if(PM_DOPPELGANGER)
			|| (Role_if(PM_FLAME_MAGE) && u.umonnum == PM_RED_DRAGON)
			|| (Role_if(PM_ICE_MAGE) && u.umonnum == PM_WHITE_DRAGON)) {
		if (u.uen < 10) {
			You("don't have enough energy to breathe!");
			return(0);
		} 
		energy = 10;
	} else {
		if (u.uen < 15) {
			You("don't have enough energy to breathe!");
			return(0);
		} 
		energy = 15;
	}
	if (!getdir((char *)0)) return(0);
	else {
	    register struct attack *mattk;
	    register int i;

		u.uen -= energy;
		flags.botl = 1;

	    for(i = 0; i < NATTK; i++) {
		mattk = &(youmonst.data->mattk[i]);
		if(mattk->aatyp == AT_BREA) break;
	    }
	    buzz((int) (20 + mattk->adtyp-1), (int)mattk->damn,
		u.ux, u.uy, u.dx, u.dy);
	}
	return(1);
}

int
dospit()
{
	struct obj *otmp;

	if (!getdir((char *)0)) return(0);
	otmp = mksobj(u.umonnum==PM_COBRA ? BLINDING_VENOM : ACID_VENOM,
			TRUE, FALSE);
	otmp->spe = 1; /* to indicate it's yours */
	throwit(otmp, 0L, 0);
	return(1);
}

int
doremove()
{
	if (!Punished) {
		You("are not chained to anything!");
		return(0);
	}
	unpunish();
	return(1);
}

int
dospinweb()
{
	register struct trap *ttmp = t_at(u.ux,u.uy);

	if (Levitation || Is_airlevel(&u.uz)
	    || Underwater || Is_waterlevel(&u.uz)) {
		You("must be on the ground to spin a web.");
		return(0);
	}
	if (u.uswallow) {
		You("release web fluid inside %s.", mon_nam(u.ustuck));
		if (is_animal(u.ustuck->data)) {
			expels(u.ustuck, u.ustuck->data, TRUE);
			return(0);
		}
		if (is_whirly(u.ustuck->data)) {
			int i;

			for (i = 0; i < NATTK; i++)
				if (u.ustuck->data->mattk[i].aatyp == AT_ENGL)
					break;
			if (i == NATTK)
			       impossible("Swallower has no engulfing attack?");
			else {
				char sweep[30];

				sweep[0] = '\0';
				switch(u.ustuck->data->mattk[i].adtyp) {
					case AD_FIRE:
						Strcpy(sweep, "ignites and ");
						break;
					case AD_ELEC:
						Strcpy(sweep, "fries and ");
						break;
					case AD_COLD:
						Strcpy(sweep,
						      "freezes, shatters and ");
						break;
				}
				pline_The("web %sis swept away!", sweep);
			}
			return(0);
		}                    /* default: a nasty jelly-like creature */
		pline_The("web dissolves into %s.", mon_nam(u.ustuck));
		return(0);
	}
	if (u.utrap) {
		You("cannot spin webs while stuck in a trap.");
		return(0);
	}
	exercise(A_DEX, TRUE);
	if (ttmp) switch (ttmp->ttyp) {
		case PIT:
		case SPIKED_PIT: You("spin a web, covering up the pit.");
			deltrap(ttmp);
			bury_objs(u.ux, u.uy);
			if (Invisible) newsym(u.ux, u.uy);
			return(1);
		case SQKY_BOARD: pline_The("squeaky board is muffled.");
			deltrap(ttmp);
			if (Invisible) newsym(u.ux, u.uy);
			return(1);
		case TELEP_TRAP:
		case LEVEL_TELEP:
		case MAGIC_PORTAL:
			Your("webbing vanishes!");
			return(0);
		case WEB: You("make the web thicker.");
			return(1);
		case HOLE:
		case TRAPDOOR:
			You("web over the %s.",
			    (ttmp->ttyp == TRAPDOOR) ? "trap door" : "hole");
			deltrap(ttmp);
			if (Invisible) newsym(u.ux, u.uy);
			return 1;
		case ARROW_TRAP:
		case DART_TRAP:
		case BEAR_TRAP:
		case LANDMINE:
		case SLP_GAS_TRAP:
		case RUST_TRAP:
		case MAGIC_TRAP:
		case ANTI_MAGIC:
		case POLY_TRAP:
			You("have triggered a trap!");
			dotrap(ttmp);
			return(1);
		default:
			impossible("Webbing over trap type %d?", ttmp->ttyp);
			return(0);
	}
	ttmp = maketrap(u.ux, u.uy, WEB);
	if (ttmp) {
		ttmp->tseen = 1;
		ttmp->madeby_u = 1;
	}
	if (Invisible) newsym(u.ux, u.uy);
	return(1);
}

int
dosummon()
{
	/* STEPHEN WHITE'S NEW CODE */        
	if (u.uen < 10) {
	    You("lack the energy to send forth a call for help!");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

	You("call upon your brethren for help!");
	exercise(A_WIS, TRUE);
	if (!were_summon(youmonst.data,TRUE))
		pline("But none arrive.");
	return(1);
}


#if 0
/* WAC supplanted by dogaze (). */
int
doconfuse()
{
	register struct monst *mtmp;
	int looked = 0;
	char qbuf[QBUFSZ];

	if (Blind) {
		You_cant("see anything to gaze at.");
		return 0;
	}
	if (u.uen < 15) {
	    You("lack the energy to use your special gaze!");
	    return(0);
	}
	u.uen -= 15;
	flags.botl = 1;

	for (mtmp = fmon; mtmp; mtmp = mtmp->nmon) {
	    if (DEADMONSTER(mtmp)) continue;
	    if (canseemon(mtmp)) {
		looked++;
		if (Invis && !perceives(mtmp->data))
		    pline("%s seems not to notice your gaze.", Monnam(mtmp));
		else if (mtmp->minvis && !See_invisible)
		    You_cant("see where to gaze at %s.", Monnam(mtmp));
		else if (mtmp->m_ap_type == M_AP_FURNITURE
			|| mtmp->m_ap_type == M_AP_OBJECT) {
		    looked--;
		    continue;
		} else if (flags.safe_dog && !Confusion && !Hallucination
		  && mtmp->mtame) {
		    You("avoid gazing at %s.", y_monnam(mtmp));
		} else {
		    if (flags.confirm && mtmp->mpeaceful && !Confusion
							&& !Hallucination) {
			Sprintf(qbuf, "Really confuse %s?", mon_nam(mtmp));
			if (yn(qbuf) != 'y') continue;
			setmangry(mtmp);
		    }
		    if (!mtmp->mcanmove || mtmp->mstun || mtmp->msleeping ||
				    !mtmp->mcansee || !haseyes(mtmp->data)) {
			looked--;
			continue;
		    }
		    if (!mon_reflects(mtmp,"Your gaze is reflected by %s %s.")){
			if (!mtmp->mconf)
			    Your("gaze confuses %s!", mon_nam(mtmp));
			else
			    pline("%s is getting more and more confused.",
							    Monnam(mtmp));
			mtmp->mconf = 1;
		    }
		    if ((mtmp->data==&mons[PM_FLOATING_EYE]) && !mtmp->mcan) {
			if (!Free_action) {
			You("are frozen by %s gaze!",
					 s_suffix(mon_nam(mtmp)));
			nomul((u.ulevel > 6 || rn2(4)) ?
				-d((int)mtmp->m_lev+1,
					(int)mtmp->data->mattk[0].damd)
				: -200);
			return 1;
			} else
			    You("stiffen momentarily under %s gaze.",
				    s_suffix(mon_nam(mtmp)));
		    }
		    if ((mtmp->data==&mons[PM_MEDUSA]) && !mtmp->mcan) {
			pline(
			 "Gazing at the awake %s is not a very good idea.",
			    l_monnam(mtmp));
			/* as if gazing at a sleeping anything is fruitful... */
			You("turn to stone...");
                        killer_format = KILLED_BY;
                        killer =
                         "deliberately gazing at Medusa's hideous countenance";
			done(STONING);
		    }
		}
	    }
	}
	if (!looked) You("gaze at no place in particular.");
	return 1;
}
#endif


int
dohide()
{
	boolean ismimic = youmonst.data->mlet == S_MIMIC;

	if (u.uundetected || (ismimic && youmonst.m_ap_type != M_AP_NOTHING)) {
		You("are already hiding.");
		return(0);
	}
	if (ismimic) {
		/* should bring up a dialog "what would you like to imitate?" */
		youmonst.m_ap_type = M_AP_OBJECT;
		youmonst.mappearance = STRANGE_OBJECT;
	} else
		u.uundetected = 1;
	newsym(u.ux,u.uy);
	return(1);
}

int
domindblast()
{
	struct monst *mtmp, *nmon;

	if (u.uen < 10) {
	    You("concentrate but lack the energy to maintain doing so.");
	    return(0);
	}
	u.uen -= 10;
	flags.botl = 1;

	pline("A wave of psychic energy pours out.");
	for(mtmp=fmon; mtmp; mtmp = nmon) {
		int u_sen;

		nmon = mtmp->nmon;
		if (DEADMONSTER(mtmp))
			continue;
		if (distu(mtmp->mx, mtmp->my) > BOLT_LIM * BOLT_LIM)
			continue;
		if(mtmp->mpeaceful)
			continue;
		u_sen = telepathic(mtmp->data) && !mtmp->mcansee;
		if (u_sen || (telepathic(mtmp->data) && rn2(3)) || !rn2(2)) {
			You("lock in on %s %s.", s_suffix(mon_nam(mtmp)),
				u_sen ? "telepathy" :
				telepathic(mtmp->data) ? "latent telepathy" :
				"mind");
			mtmp->mhp -= rn1(4,4);
			if (mtmp->mhp <= 0)
				killed(mtmp);
		}
	}
	return 1;
}

STATIC_OVL void
uunstick()
{
	pline("%s is no longer in your clutches.", Monnam(u.ustuck));
	u.ustuck = 0;
}

STATIC_OVL void
skinback()
{
	if (uskin) {
		Your("skin returns to its original form.");
		uarm = uskin;
		uskin = (struct obj *)0;
		/* undo save/restore hack */
		uarm->owornmask &= ~I_SPECIAL;
	}
}

#endif /* OVLB */
#ifdef OVL1

const char *
mbodypart(mon, part)
struct monst *mon;
int part;
{
	static NEARDATA const char
	*humanoid_parts[] = { "arm", "eye", "face", "finger",
		"fingertip", "foot", "hand", "handed", "head", "leg",
		"light headed", "neck", "spine", "toe", "hair", "blood", "lung"},
	*jelly_parts[] = { "pseudopod", "dark spot", "front",
		"pseudopod extension", "pseudopod extremity",
		"pseudopod root", "grasp", "grasped", "cerebral area",
		"lower pseudopod", "viscous", "middle", "surface",
		"pseudopod extremity", "ripples", "juices", "surface" },
	*animal_parts[] = { "forelimb", "eye", "face", "foreclaw", "claw tip",
		"rear claw", "foreclaw", "clawed", "head", "rear limb",
		"light headed", "neck", "spine", "rear claw tip",
		"fur", "blood", "lung" },
	*horse_parts[] = { "foreleg", "eye", "face", "forehoof", "hoof tip",
		"rear hoof", "foreclaw", "hooved", "head", "rear leg",
		"light headed", "neck", "backbone", "rear hoof tip",
		"mane", "blood", "lung" },
	*sphere_parts[] = { "appendage", "optic nerve", "body", "tentacle",
		"tentacle tip", "lower appendage", "tentacle", "tentacled",
		"body", "lower tentacle", "rotational", "equator", "body",
		"lower tentacle tip", "cilia", "life force", "retina" },
	*fungus_parts[] = { "mycelium", "visual area", "front", "hypha",
		"hypha", "root", "strand", "stranded", "cap area",
		"rhizome", "sporulated", "stalk", "root", "rhizome tip",
		"spores", "juices", "gill" },
	*vortex_parts[] = { "region", "eye", "front", "minor current",
		"minor current", "lower current", "swirl", "swirled",
		"central core", "lower current", "addled", "center",
		"currents", "edge", "currents", "life force", "center" },
	*snake_parts[] = { "vestigial limb", "eye", "face", "large scale",
		"large scale tip", "rear region", "scale gap", "scale gapped",
		"head", "rear region", "light headed", "neck", "length",
		"rear scale", "scales", "blood", "lung" },
	*fish_parts[] = { "fin", "eye", "premaxillary", "pelvic axillary",
		"pelvic fin", "anal fin", "pectoral fin", "finned", "head", "peduncle",
		"played out", "gills", "dorsal fin", "caudal fin",
		"scales", "blood", "gill" };
	/* claw attacks are overloaded in mons[]; most humanoids with
	   such attacks should still reference hands rather than claws */
	static const char not_claws[] = {
		S_HUMAN, S_MUMMY, S_ZOMBIE, S_ANGEL,
		S_NYMPH, S_LEPRECHAUN, S_QUANTMECH, S_VAMPIRE,
		S_ORC, S_GIANT,         /* quest nemeses */
		'\0'            /* string terminator; assert( S_xxx != 0 ); */
	};
	struct permonst *mptr = mon->data;

	if (part == HAND || part == HANDED) {   /* some special cases */
	    if (mptr->mlet == S_DOG || mptr->mlet == S_FELINE ||
		    mptr->mlet == S_YETI)
		return part == HAND ? "paw" : "pawed";
	    if (humanoid(mptr) && attacktype(mptr, AT_CLAW) &&
		    !index(not_claws, mptr->mlet) &&
		    mptr != &mons[PM_STONE_GOLEM] &&
		    mptr != &mons[PM_INCUBUS] && mptr != &mons[PM_SUCCUBUS])
		return part == HAND ? "claw" : "clawed";
	}
	if (mptr == &mons[PM_SHARK] && part == HAIR)
	    return "skin";	/* sharks don't have scales */
	if (humanoid(mptr) &&
		(part == ARM || part == FINGER || part == FINGERTIP ||
		    part == HAND || part == HANDED))
	    return humanoid_parts[part];
	if (mptr->mlet == S_CENTAUR || mptr->mlet == S_UNICORN ||
		(mptr == &mons[PM_ROTHE] && part != HAIR))
	    return horse_parts[part];
	if (mptr->mlet == S_EEL && mptr != &mons[PM_JELLYFISH])
	    return fish_parts[part];
	if (slithy(mptr))
	    return snake_parts[part];
	if (mptr->mlet == S_EYE)
	    return sphere_parts[part];
	if (mptr->mlet == S_JELLY || mptr->mlet == S_PUDDING ||
		mptr->mlet == S_BLOB || mptr == &mons[PM_JELLYFISH])
		return jelly_parts[part];
	if (mptr->mlet == S_VORTEX || mptr->mlet == S_ELEMENTAL)
	    return vortex_parts[part];
	if (mptr->mlet == S_FUNGUS)
	    return fungus_parts[part];
	if (humanoid(mptr))
	    return humanoid_parts[part];
	return animal_parts[part];
}

const char *
body_part(part)
int part;
{
	return mbodypart(&youmonst, part);
}

#endif /* OVL1 */
#ifdef OVL0

int
poly_gender()
{
/* Returns gender of polymorphed player; 0/1=same meaning as flags.female,
 * 2=none.
 */
	if (is_neuter(youmonst.data) || !humanoid(youmonst.data)) return 2;
	return flags.female;
}

#endif /* OVL0 */
#ifdef OVLB

void
ugolemeffects(damtype, dam)
int damtype, dam;
{
	int heal = 0;
	/* We won't bother with "slow"/"haste" since players do not
	 * have a monster-specific slow/haste so there is no way to
	 * restore the old velocity once they are back to human.
	 */
	if (u.umonnum != PM_FLESH_GOLEM && u.umonnum != PM_IRON_GOLEM)
		return;
	switch (damtype) {
		case AD_ELEC: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam / 6; /* Approx 1 per die */
			break;
		case AD_FIRE: if (u.umonnum == PM_IRON_GOLEM)
				heal = dam;
			break;
	}
	if (heal && (u.mh < u.mhmax)) {
		u.mh += heal;
		if (u.mh > u.mhmax) u.mh = u.mhmax;
		flags.botl = 1;
		pline("Strangely, you feel better than before.");
		exercise(A_STR, TRUE);
	}
}

STATIC_OVL int
armor_to_dragon(atyp)
int atyp;
{
	switch(atyp) {
	    case GRAY_DRAGON_SCALE_MAIL:
	    case GRAY_DRAGON_SCALES:
		return PM_GRAY_DRAGON;
	    case SILVER_DRAGON_SCALE_MAIL:
	    case SILVER_DRAGON_SCALES:
		return PM_SILVER_DRAGON;
	    case SHIMMERING_DRAGON_SCALE_MAIL:
	    case SHIMMERING_DRAGON_SCALES:
		return PM_SHIMMERING_DRAGON;
	    case DEEP_DRAGON_SCALE_MAIL:
	    case DEEP_DRAGON_SCALES:
		return PM_DEEP_DRAGON;
	    case RED_DRAGON_SCALE_MAIL:
	    case RED_DRAGON_SCALES:
		return PM_RED_DRAGON;
	    case ORANGE_DRAGON_SCALE_MAIL:
	    case ORANGE_DRAGON_SCALES:
		return PM_ORANGE_DRAGON;
	    case WHITE_DRAGON_SCALE_MAIL:
	    case WHITE_DRAGON_SCALES:
		return PM_WHITE_DRAGON;
	    case BLACK_DRAGON_SCALE_MAIL:
	    case BLACK_DRAGON_SCALES:
		return PM_BLACK_DRAGON;
	    case BLUE_DRAGON_SCALE_MAIL:
	    case BLUE_DRAGON_SCALES:
		return PM_BLUE_DRAGON;
	    case GREEN_DRAGON_SCALE_MAIL:
	    case GREEN_DRAGON_SCALES:
		return PM_GREEN_DRAGON;
	    case YELLOW_DRAGON_SCALE_MAIL:
	    case YELLOW_DRAGON_SCALES:
		return PM_YELLOW_DRAGON;
	    default:
		return -1;
	}
}


int
polyatwill()      /* Polymorph at will for Doppelganger class */
{
	int mon;

#define EN_DOPP 	20 	/* This is the "base cost" for a polymorph
				 * Actual cost is this base cost + 5 * monster level
				 * of the final form you actually assume.
				 * Energy will be taken first, then you will get 
				 * more hungry if you do not have enough energy.
				 */
#define EN_WERE 	10
#define EN_BABY_DRAGON 	10
#define EN_ADULT_DRAGON 15

	boolean scales = ((uarm && uarm->otyp == RED_DRAGON_SCALES
				&& Role_if(PM_FLAME_MAGE)) ||
			  (uarm && uarm->otyp == WHITE_DRAGON_SCALES
				&& Role_if(PM_ICE_MAGE)));
	boolean scale_mail = ((uarm && uarm->otyp == RED_DRAGON_SCALE_MAIL   
				&& Role_if(PM_FLAME_MAGE)) ||
			  (uarm && uarm->otyp == WHITE_DRAGON_SCALE_MAIL 
				&& Role_if(PM_ICE_MAGE)));

	/* Set to TRUE when you can polyatwill but choose not to */
	boolean can_polyatwill = FALSE; 
	
	/* KMH, balance patch -- new intrinsic */
	if (Unchanging) {
	    pline("You cannot change your form.");
	    return (0);
	}

	/* First, if in correct polymorphed form, rehumanize (for free) 
	 * Omit Lycanthropes,  who need to spend energy to change back and forth
	 */
	if (Upolyd && (Race_if(PM_DOPPELGANGER) ||
			(Role_if(PM_FLAME_MAGE) && (u.umonnum == PM_RED_DRAGON || 
				u.umonnum == PM_BABY_RED_DRAGON)) ||
			(Role_if(PM_ICE_MAGE) && (u.umonnum == PM_WHITE_DRAGON || 
				u.umonnum == PM_BABY_WHITE_DRAGON)))) {
	    rehumanize();
	    return(1);	    
	}

	if ((Role_if(PM_ICE_MAGE) || Role_if(PM_FLAME_MAGE)) &&
		((u.uen > EN_BABY_DRAGON && u.ulevel > 6) || scales || scale_mail)) {
	    if (yn("Polymorph to your draconic form?") == 'n') 
	    	can_polyatwill = TRUE;
	    
	    /* Check if you can do the adult form */
	    else if ((u.ulevel > 13 && u.uen > EN_ADULT_DRAGON) || 
	    	(u.ulevel > 6 && (scales && u.uen > EN_BABY_DRAGON) || scale_mail)) {
		    /* If you have scales, energy cost is less */
		    /* If you have scale mail,  there is no cost! */
	    	    if (!scale_mail) {
			    if (scales) u.uen -= EN_BABY_DRAGON; 
			    else u.uen -= EN_ADULT_DRAGON;
	    	    }
		    
		    /* Get the adult dragon form */
		    if (Role_if(PM_FLAME_MAGE)) mon = PM_RED_DRAGON;
		    else mon = PM_WHITE_DRAGON;

		    if (!(mvitals[mon].mvflags & G_GENOD) && scales) {
		    	merge_with_armor();
		    }
		    
		    polymon(mon); /* Goto adult form */
		    return(1);
	    /* Try the baby form */
	    } else if ((u.ulevel > 6 && ((u.uen > EN_BABY_DRAGON) || scales)) || scale_mail) {
		    if(!scales && !scale_mail) u.uen -= EN_BABY_DRAGON;
		
		    if (Role_if(PM_FLAME_MAGE)) mon = PM_BABY_RED_DRAGON;
		    else mon = PM_BABY_WHITE_DRAGON;

		    if (!(mvitals[mon].mvflags & G_GENOD) && scales) {
		    	merge_with_armor();
		    }
		
		    polymon(mon); /* Goto baby form */
		    return(1); 
	    } else {
		You("don't have the energy to polymorph.");
		return (0);		
	    }
	}
	if (Race_if(PM_DOPPELGANGER)) {
	    if (yn("Polymorph at will?") == 'n')	    
	    	can_polyatwill = TRUE;
	    else if (u.uen < EN_DOPP) {
		    pline("You don't have the energy to polymorph!");
		    return(0);
	    } else {
		u.uen -= EN_DOPP;
		if (multi >= 0) {
		    if (occupation) stop_occupation();
		    else nomul(0);
		}
		polyself();
		if (Upolyd) { /* You actually polymorphed */
		    u.uen -= 5 * mons[u.umonnum].mlevel;
		    if (u.uen < 0) {
			morehungry(-u.uen);
			u.uen = 0;
		    }
		}
		return(1);	
	    }
	} else if (Race_if(PM_HUMAN_WEREWOLF) && (!Upolyd || u.umonnum == u.ulycn)) {
	    if (yn("Change form?") == 'n')
	    	can_polyatwill = TRUE;
	    else if (u.ulycn == NON_PM) {
	    	/* Very serious */
	    	You("are no longer a lycanthrope!");
	    } else if (u.ulevel <= 2) {
	    	You("can't invoke the change at will yet.");
		return (0);		
	    } else if (u.uen < EN_WERE) {
		pline("You don't have the energy to change form!");
		return (0);
	    } else {
	    	/* Committed to the change now */
		u.uen -= EN_WERE;
		if (!Upolyd) {
		    if (multi >= 0) {
			if (occupation) stop_occupation();
			else nomul(0);
		    }
		    you_were();
		    return(1);
		} else {
		    rehumanize();
		    return(1);
		}
	    }
	} else if (!can_polyatwill) {
		pline("You can't polymorph at will%s.", 
			((Role_if(PM_FLAME_MAGE) || Role_if(PM_ICE_MAGE) || 
			  Race_if(PM_HUMAN_WEREWOLF) || Race_if(PM_DOPPELGANGER)) ?
				" yet" : ""));
		return 0;
	}
	
	flags.botl = 1;
	return 1;
}

static void
merge_with_armor()
{
	/* This function does hides the armor being worn 
	 * It currently assumes that you are changing into a dragon
	 * Should check that monster being morphed into is not genocided
	 * see do_merge above for correct use
	 */
	You("merge with your scaly armor.");
	uskin = uarm;
	uarm = (struct obj *)0;
	/* save/restore hack */
	uskin->owornmask |= I_SPECIAL;

}

#if 0	/* What the f*** is this for? -- KMH */
static void
special_poly()
{
	char buf[BUFSZ];
	int old_light, new_light;
	int mntmp = NON_PM;
	int tries=0;

	old_light = (u.umonnum >= LOW_PM) ? emits_light(youmonst.data) : 0;
	do {
		getlin("Become what kind of monster? [type the name]", buf);
		mntmp = name_to_mon(buf);
		if (mntmp < LOW_PM)
				pline("I've never heard of such monsters.");
			/* Note:  humans are illegal as monsters, but an
			 * illegal monster forces newman(), which is what we
			 * want if they specified a human.... */

			/* [Tom] gnomes are polyok, so this doesn't apply for
			   player gnomes */
	                     /* WAC but we want to catch player gnomes and not
	                        so add an extra check */
		else if (!polyok(&mons[mntmp]) &&
				(Role_elven ? !is_elf(&mons[mntmp]) :
#ifdef DWARF
				Role_if(PM_DWARF) ? !is_gnome(&mons[mntmp]) :
#endif
				/* WAC
				 * should always fail (for now) gnome check
				 * unless gnomes become not polyok.  Then, it'll
				 * still work ;B
				 */
				Role_if(PM_GNOME) ? !is_gnome(&mons[mntmp]) :
				!is_human(&mons[mntmp])))
			You("cannot polymorph into that.");
#ifdef EATEN_MEMORY
		else if (!mvitals[mntmp].eaten && (rn2((u.ulevel + 25)) < 20)) {
			You("don't have the knowledge to polymorph into that.");
			return;  /* Nice try */
		} else {
			You("attempt an unfamiliar polymorph.");
			break;
		}
#endif
	} while(++tries < 5);
	if (tries==5) {
		pline(thats_enough_tries);
		return;
	} else if (polymon(mntmp)) {
		/* same as made_change above */
		new_light = (u.umonnum >= LOW_PM) ? emits_light(youmonst.data) : 0;
		if (old_light != new_light) {
		    if (old_light)
			del_light_source(LS_MONSTER, (genericptr_t)&youmonst);
		    if (new_light == 1) ++new_light;  /* otherwise it's undetectable */
		    if (new_light)
			new_light_source(u.ux, u.uy, new_light,
					 LS_MONSTER, (genericptr_t)&youmonst);
		}
	}
	return;
}
#endif


#endif /* OVLB */

/*polyself.c*/