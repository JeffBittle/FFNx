/****************************************************************************/
//    Copyright (C) 2009 Aali132                                            //
//    Copyright (C) 2018 quantumpencil                                      //
//    Copyright (C) 2018 Maxime Bacoux                                      //
//    Copyright (C) 2020 myst6re                                            //
//    Copyright (C) 2020 Chris Rizzitello                                   //
//    Copyright (C) 2020 John Pritchard                                     //
//    Copyright (C) 2021 Julian Xhokaxhiu                                   //
//                                                                          //
//    This file is part of FFNx                                             //
//                                                                          //
//    FFNx is free software: you can redistribute it and/or modify          //
//    it under the terms of the GNU General Public License as published by  //
//    the Free Software Foundation, either version 3 of the License         //
//                                                                          //
//    FFNx is distributed in the hope that it will be useful,               //
//    but WITHOUT ANY WARRANTY; without even the implied warranty of        //
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         //
//    GNU General Public License for more details.                          //
/****************************************************************************/

#include "../ff7.h"
#include "../log.h"
#include "../achievement.h"

void magic_thread_start(void (*func)())
{
	ff7_externals.destroy_magic_effects();

	/*
	 * Original function creates a separate thread but the code is not thread
	 * safe in any way! Luckily modern PCs are fast enough to load magic
	 * effects synchronously.
	 */
	func();
}

void ff7_load_battle_stage(int param_1, int battle_location_id, int **param_3){
	((void(*)(int, int, int **)) ff7_externals.load_battle_stage)(param_1, battle_location_id, param_3);

	g_FF7SteamAchievements.initCharStatsBeforeBattle(ff7_externals.savemap->chars);
	g_FF7SteamAchievements.unlockBattleSquareAchievement(battle_location_id);
}

void ff7_battle_sub_5C7F94(int param_1, int param_2){
	((void(*)(int, int)) ff7_externals.battle_sub_5C7F94)(param_1, param_2);

	if (trace_all || trace_achievement)
		ffnx_trace("%s - trying to unlock achievement for gil\n", __func__);
	g_FF7SteamAchievements.unlockGilAchievement(ff7_externals.savemap->gil);
}

void ff7_battle_sub_435139(int param_1, char party_member_index, char param_3, WORD param_4){
	((void(*)(int, char, char, WORD)) ff7_externals.battle_sub_435139)(param_1, party_member_index, param_3, param_4);

	char characterID = ff7_externals.savemap->party_members[party_member_index];
	if(ff7_externals.savemap->chars[characterID].current_limit_level == 1)
		g_FF7SteamAchievements.unlockFirstLimitBreakAchievement(characterID);
}