#ifndef DisplaySpec_H
#define DisplaySpec_H

#include <set>
#include <RageUtil/Misc/RageLog.h>
#include "RageUtil/Misc/RageTypes.h"

struct DisplayMode
{
	// Width (in pixels) of the display in this mode
	unsigned int width;
	// Height (in pixels) of the display in this mode
	unsigned int height;
	/** Refresh rate, in hz, of the display for the given mode
	 * MacOS Quartz Display Services provides rate as double,
	 * winapi DEVMODE uses 32bit unsigned int (still gives rate as hz),
	 * RandR gives 32 bit pixel clock, which is then divided by width*height.
	 */
	double refreshRate;
	/*
	 * Bits-per-pixel is *not* a property of the DisplayMode for our purposes:
	 * bit-depth is going to be a property of the OpenGL/D3D context, not a
	 * display configuration
	 */

	bool operator<(const DisplayMode& other) const
	{
/** @brief A quick way to compare the two DisplayResolutions. */
#define COMPARE(x)                                                             \
	if (x != other.x)                                                          \
		return x < other.x;
		COMPARE(width);
		COMPARE(height);
		COMPARE(refreshRate);
#undef COMPARE
		return false;
	}

	// Lua
	void PushSelf(lua_State* L);
};

/** @brief The dimensions of the program. */
class DisplaySpec
{
  public:
	/*
	 * Construct a specification for the display with the given ID, which
	 * supports the given modes, and is currently using the specified mode with
	 * the specified logical screen bounds
	 */
	DisplaySpec(const std::string& id,
				const std::string& name,
				const std::set<DisplayMode>& modes,
				const DisplayMode& curMode,
				const RectI& curBounds,
				const bool isVirtual = false)
	  : m_sId(id)
	  , m_sName(name)
	  , m_sModes(modes)
	  , m_bCurModeActive(true)
	  , m_CurMode(curMode)
	  , m_rectBounds(curBounds)
	  , m_bIsVirtual(isVirtual)
	{
		if (m_sModes.find(curMode) == m_sModes.end()) {
			// This is an error, make a failing assertion with a descriptive
			// error message
			std::stringstream msgStream;
			msgStream << "DisplaySpec current mode (" << curMode.width << "x"
					  << curMode.height << "@" << curMode.refreshRate
					  << ") not in given list of supported modes: ";
			for (auto& m : modes) {
				msgStream << m.width << "x" << m.height << "@" << m.refreshRate
						  << ", ";
			}
			auto msg = msgStream.str();
			// Drop the trailing ", "
			msg.resize(msg.size() - 2);

			LOG->Warn(msg.c_str());
		}
	}

	/*
	 * Construct a specification for the display with the given ID, which
	 * supports the given modes, and is currently disabled (has no active mode)
	 */
	DisplaySpec(const std::string id,
				const std::string name,
				const std::set<DisplayMode> modes,
				const bool isVirtual = false)
	  : m_sId(id)
	  , m_sName(name)
	  , m_sModes(modes)
	  , m_bCurModeActive(false)
	  , m_CurMode({})
	  , m_bIsVirtual(isVirtual)
	{
	}

	// Create a specification for a display supporting a single (and currently
	// active) mode
	DisplaySpec(std::string id, std::string name, DisplayMode mode)
	  : m_sId(id)
	  , m_sName(name)
	  , m_bIsVirtual(false)
	  , m_bCurModeActive(true)
	  , m_CurMode(mode)
	{
		m_sModes.insert(mode);
		m_rectBounds = RectI(0, 0, mode.width, mode.height);
	}

	DisplaySpec(const DisplaySpec& other) = default;

	std::string name() const { return m_sName; }

	std::string id() const { return m_sId; }

	const std::set<DisplayMode>& supportedModes() const { return m_sModes; }

	/*
	 * Return a pointer to the currently active display mode, or NULL if
	 * display is inactive
	 *
	 * Note that inactive *does not* necessarily mean unusable. E.g., in X11,
	 * an output can be enabled/disabled by an application by
	 * connecting/disconnecting a crtc
	 */
	const DisplayMode* currentMode() const
	{
		return m_bCurModeActive ? &m_CurMode : nullptr;
	}

	const RectI& currentBounds() const { return m_rectBounds; }

	bool isVirtual() const { return m_bIsVirtual; }

	/**
	 * @brief Determine if one DisplaySpec compares less than the other.
	 *
	 * Used to enforce a consistent ordering of displays, e.g. for consistent
	 * option presentation. Also allows DisplaySpec to be placed in a std::set
	 *
	 * @param other the other DisplaySpec to check.
	 * @return true if this DisplaySpec is less than the other, or false
	 * otherwise. */
	bool operator<(const DisplaySpec& other) const
	{
		return m_sId < other.id();
	}

	// Lua
	void PushSelf(lua_State* L);

  private:
	// Unique identifier of the display
	std::string m_sId;
	// "Human-readable" display name
	std::string m_sName;
	// Modes supported by this display
	std::set<DisplayMode> m_sModes;
	// currently configured mode, if available
	bool m_bCurModeActive;
	DisplayMode m_CurMode;
	// The current bounds of this display in global display coordinate space
	RectI m_rectBounds;
	// Flag is "true" when display represents a logical display like an X screen
	// or the Win32 "Virtual screen"
	bool m_bIsVirtual;
};
/** @brief The collection of DisplaySpec available within the program. */
typedef std::set<DisplaySpec> DisplaySpecs;
// Lua
DisplaySpecs*
pushDisplaySpecs(lua_State* L, const DisplaySpecs& specs);

#endif
