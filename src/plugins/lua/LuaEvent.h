/*
 * MacroQuest2: The extension platform for EverQuest
 * Copyright (C) 2002-2020 MacroQuest Authors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2, as published by
 * the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#pragma once

#include "LuaCommon.h"

#include <queue>

class Blech;

namespace mq::lua::thread {
struct LuaThread;
struct ThreadState;
}

namespace mq::lua::events {

void RegisterLua(sol::table& lua);

struct LuaEventProcessor;
struct LuaEvent
{
	const std::string name;
	const std::string expression;
	const sol::function function;
	LuaEventProcessor* processor;
	uint32_t id;

	LuaEvent(std::string_view name, std::string_view expression, const sol::function& function, LuaEventProcessor* processor);
	~LuaEvent();
};

struct LuaBind
{
	const std::string name;
	const sol::function function;
	uint8_t* callback;
	LuaEventProcessor* processor;

	LuaBind(const std::string& name, const sol::function& function, LuaEventProcessor* processor);
	~LuaBind();
};

template <typename T>
struct LuaEventInstance
{
	T* definition;
	std::vector<std::string> args;
	sol::thread thread;
};

struct LuaEventProcessor
{
	const thread::LuaThread* thread;

	std::unique_ptr<Blech> blech;
	std::vector<std::unique_ptr<LuaEvent>> eventDefinitions;
	std::vector<LuaEventInstance<LuaEvent>> eventsPending;
	std::vector<std::pair<sol::coroutine, std::vector<std::string>>> eventsRunning;

	std::vector<std::unique_ptr<LuaBind>> bindDefinitions;
	std::vector<LuaEventInstance<LuaBind>> bindsPending;
	std::vector<std::pair<sol::coroutine, std::vector<std::string>>> bindsRunning;

	LuaEventProcessor(const thread::LuaThread* thread);
	~LuaEventProcessor();

	void AddEvent(std::string_view name, std::string_view expression, const sol::function& function);
	void RemoveEvent(std::string_view name);

	void AddBind(std::string_view name, const sol::function& function);
	void RemoveBind(std::string_view name);

	void Process(std::string_view line) const;

	// this is guaranteed to always run at the exact same time, so we can run binds and events in it
	void RunEvents(const thread::LuaThread& thread);

	// we need two separate functions here because we need to be able to run these at separate points, independently
	void PrepareEvents();
	void PrepareBinds();
};

} // namespace mq::lua::events