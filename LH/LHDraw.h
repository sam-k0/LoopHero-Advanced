#pragma once
#include <string>
#include "../PluginSetup.h"

// Helpers for drawing from inside draw events, colours are BGR
namespace LHDraw
{
	// Not DrawText, that's a Windows.h macro
	inline void DrawTextWrapper(double x, double y, const std::string& text, double color)
	{
		Binds::CallBuiltinA("draw_set_colour", { color });
		Binds::CallBuiltinA("draw_text", { x, y, text });
	}

	// Saves the draw state the game relies on and restores it when going out of scope
	class DrawStateWrapper
	{
		struct Entry {
			const char* getter;
			const char* setter;
			YYRValue value;
			bool saved;
		};

		Entry entries[5] = {
			{ "draw_get_font", "draw_set_font" },
			{ "draw_get_colour", "draw_set_colour" },
			{ "draw_get_alpha", "draw_set_alpha" },
			{ "draw_get_halign", "draw_set_halign" },
			{ "draw_get_valign", "draw_set_valign" },
		};

	public:
		DrawStateWrapper()
		{
			for (Entry& e : entries)
				e.saved = CallBuiltin(e.value, e.getter, nullptr, nullptr, {});
		}

		~DrawStateWrapper()
		{
			for (Entry& e : entries)
			{
				if (e.saved)
					Binds::CallBuiltinA(e.setter, { e.value });
			}
		}

		DrawStateWrapper(const DrawStateWrapper&) = delete;
		DrawStateWrapper& operator=(const DrawStateWrapper&) = delete;
	};
}
