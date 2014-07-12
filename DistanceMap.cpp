/* DistanceMap.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License for more details.
*/

#include "DistanceMap.h"

#include "Outfit.h"
#include "PlayerInfo.h"
#include "Ship.h"
#include "System.h"

#include <vector>

using namespace std;



// If a player is given, the map will only use hyperspace paths known to the
// player; that is, one end of the path has been visited. Also, if the
// player's flagship has a jump drive, the jumps will be make use of it.
DistanceMap::DistanceMap(const System *center, const Set<System> &systems)
{
	distance[center] = 0;
	Init(systems);
}



DistanceMap::DistanceMap(const PlayerInfo &player, const Set<System> &systems)
{
	if(!player.GetShip() || !player.GetShip()->GetSystem())
		return;
	
	distance[player.GetShip()->GetSystem()] = 0;
	
	if(player.GetShip()->Attributes().Get("jump drive"))
		InitJump(systems, player);
	else if(player.GetShip()->Attributes().Get("hyperdrive"))
		InitHyper(systems, player);
	
	// If the player has a ship but no hyperdrive capability, all systems are
	// marked as unreachable except for this one.
}



// Find out if the given system is reachable.
bool DistanceMap::HasRoute(const System *system) const
{
	auto it = distance.find(system);
	return (it != distance.end());
}



// Find out how many jumps away the given system is.
int DistanceMap::Distance(const System *system) const
{
	auto it = distance.find(system);
	if(it == distance.end())
		return -1;
	
	return it->second;
}



// If I am in the given system, going to the player's system, what system
// should I jump to next?
const System *DistanceMap::Route(const System *system) const
{
	auto it = distance.find(system);
	if(it == distance.end())
		return nullptr;
	
	for(const System *link : (hasJump ? system->Neighbors() : system->Links()))
	{
		auto lit = distance.find(link);
		if(lit != distance.end() && lit->second < it->second)
			return lit->first;
	}
	return system;
}



void DistanceMap::Init(const Set<System> &systems)
{
	vector<const System *> edge = {distance.begin()->first};
	for(int steps = 1; !edge.empty(); ++steps)
	{
		vector<const System *> newEdge;
		for(const System *system : edge)
			for(const System *link : system->Links())
			{
				auto it = distance.find(link);
				if(it != distance.end())
					continue;
				
				distance[link] = steps;
				newEdge.push_back(link);
			}
		newEdge.swap(edge);
	}
}



void DistanceMap::InitHyper(const Set<System> &systems, const PlayerInfo &player)
{
	vector<const System *> edge = {distance.begin()->first};
	for(int steps = 1; !edge.empty(); ++steps)
	{
		vector<const System *> newEdge;
		for(const System *system : edge)
			for(const System *link : system->Links())
			{
				if(!player.HasSeen(link))
					continue;
				if(!player.HasVisited(link) && !player.HasVisited(system))
					continue;
				
				auto it = distance.find(link);
				if(it != distance.end())
					continue;
				
				distance[link] = steps;
				newEdge.push_back(link);
			}
		newEdge.swap(edge);
	}
}



void DistanceMap::InitJump(const Set<System> &systems, const PlayerInfo &player)
{
	hasJump = true;
	
	vector<const System *> edge = {distance.begin()->first};
	for(int steps = 1; !edge.empty(); ++steps)
	{
		vector<const System *> newEdge;
		for(const System *system : edge)
			for(const System *link : system->Neighbors())
			{
				if(!player.HasSeen(link))
					continue;
				
				auto it = distance.find(link);
				if(it != distance.end())
					continue;
				
				distance[link] = steps;
				newEdge.push_back(link);
			}
		newEdge.swap(edge);
	}
}