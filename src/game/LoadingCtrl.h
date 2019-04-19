#ifndef LOADINGCTRL_H
#define LOADINGCTRL_H

#include "StateController.h"

#include <vector>
#include <memory>

class LoadingScreen;

class LoadingCtrl : public StateController
{
public:

	enum Situation {
		INITIAL,
		SESSION_START,
		SESSION_END,
	};

	LoadingCtrl(GameState &state, Situation);
	~LoadingCtrl() override;

	void update(float dt) override;

private:
	GameState::StateNames nextState_;
	std::vector<int> tasks_;
	unsigned taskPointer_ = 0;
	unsigned taskProgress_ = 0;

	std::shared_ptr<LoadingScreen> loadingScreen_;
};

#endif // LOADINGCTRL_H
