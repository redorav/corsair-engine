#pragma once

class CrPlatformInput
{
public:

	static const unsigned MaxControllers = 4;

	enum class ConnectionStatus
	{
		Unknown      = 0,
		Disconnected = 1,
		Connected    = 2,
		Count
	};

	static void Initialize();

	static void Deinitialize();

	void RequestRefreshControllers();

	void Update();

private:

	ConnectionStatus m_previousControllerStatus[MaxControllers] = {};

	ConnectionStatus m_controllerStatus[MaxControllers] = {};

	bool m_requestControllersRefresh = false;
};

extern CrPlatformInput* PlatformInput;