#pragma once
#include "SDK/SDK.hpp"


// typedef api functions
// LHCore functions
typedef int (*TGetRegisteredPluginCount)();
typedef const char* (TGetRegisteredPluginName)(int);
typedef bool (*TGetRegisteredPluginPresent)(const char*);
// Snow functions
typedef void (*TSetSnowIntensity)(int);
typedef int (*TGetSnowIntensity)();

namespace Deps {
	// Returns a raw pointer to the required function
	// returns nullptr if the function is not found, otherwise returns the function pointer
	void* ResolveImportFunction(const char* fn)
	{
		void* rawFn;
		if (PmGetExported(fn, rawFn) == YYTK_OK)
		{
			return rawFn;
		}
		else
		{
			Misc::Print("[Dependencies] Failed to load " + std::string(fn), CLR_RED);
			return nullptr;
		}

	}

	// Checks if a plugin with the given id is present, returns true if it is, otherwise prints an error and returns false
	bool RequireDependency(const char* id)
	{
		void* pGetRegisteredPluginPresentRaw;
		if (PmGetExported("API_GetRegisteredPluginPresent", pGetRegisteredPluginPresentRaw) == YYTK_OK)
		{
			TGetRegisteredPluginPresent pGetRegisteredPluginPresent = reinterpret_cast<TGetRegisteredPluginPresent>(pGetRegisteredPluginPresentRaw);

			// Check if the plugin is present
			if (pGetRegisteredPluginPresent(id))
			{
				return true;
			}
			else
			{
				Misc::Print("[Dependencies] Required plugin not found: " + std::string(id), CLR_RED);
				return false;
			}
		}
	}
}
