#include "StateController.h"

void StateController::signal(StateSignals signal) {
	if (signalHandlers_[signal])
		signalHandlers_[signal]();
}
