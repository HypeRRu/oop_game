#include "../../inc/states/gameplay_event_handler.h"

GameplayEventHandler::GameplayEventHandler(StateGameplay& _state):
	state{_state}
{
	this->setNext(std::make_shared<MovementHandler>(
		this->state.getPlayerPtr())
	);
}

bool GameplayEventHandler::handleEvent(Event e)
{
	switch (e)
	{
		case Event::EventGoBack:
			this->state.pause(true);
			return true;
		case Event::EventGameClosed:
			this->state.getGame().exit();
			return true;
		default:
			if (this->next_handler.get())
				return this->next_handler->handleEvent(e);
	}
	return false;
}
