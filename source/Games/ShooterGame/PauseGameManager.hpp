
class PauseGameManager
{
public:

	static inline void SetGamePaused(bool val)
	{
		gamePaused = val;
	}

	static inline bool GetGamePaused()
	{
		return gamePaused;
	}

	static inline void SetGamePausedGameplay(bool val)
	{
		gamePausedGameplay = val;
	}

	static inline bool GetGamePausedGameplay()
	{
		return gamePausedGameplay;
	}


private:

	static inline bool isGameAnyPaused()
	{
		return gamePaused || gamePausedGameplay;
	}

	static inline bool gamePaused = false;
	static inline bool gamePausedGameplay = false;

	friend class GameStart;

};

