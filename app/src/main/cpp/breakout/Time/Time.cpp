#include <chrono>
#include "Time.h"


namespace Time
{
	namespace
	{
		float g_DeltaTime;

		std::chrono::time_point<std::chrono::steady_clock> g_LastTime;

		float g_TimeSinceStart = 0.0f;
	}

	float getTimeSinceStart()
	{
		return g_TimeSinceStart;
	}

	float getDeltaTime()
	{
		return g_DeltaTime;
	}

	float getFps()
	{
		return 1000.0f / g_DeltaTime;
	}

	void startTimeUpdate()
	{
		g_LastTime = std::chrono::high_resolution_clock::now();
	}

	void endTimeUpdate()
	{
		const auto endTime = std::chrono::high_resolution_clock::now();
		const std::chrono::duration<float> duration = (endTime - g_LastTime);
		g_DeltaTime = duration.count() * 1000.0f;

		g_TimeSinceStart += duration.count();
	}
}

