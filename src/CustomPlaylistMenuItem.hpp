#pragma once

#include "MenuItem.hpp"

struct CustomPlaylistMenuItem : MenuItem
{
	enum struct State
	{
		Idle,
		OpeningMusicPlayer,
		InMusicPlayer,
	};

public:
	using MenuItem::MenuItem;

	bool onEnter();
	bool onUpdate();

private:
	State m_state = State::Idle;
};
