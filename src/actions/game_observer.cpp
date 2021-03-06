#include "../../inc/actions/game_observer.h"

GameObserver::GameObserver(StateGameplay& game_object):
	game_object{game_object}
{
	this->field = game_object.getFieldPtr();
	this->renderer = game_object.getRendererPtr();
	this->player = game_object.getPlayerPtr();
}

GameObserver::~GameObserver()
{}

bool GameObserver::moveLogicForEnemy(ActionMove& action)
{
	if (this->field->getCell(
		action.getCoords().first,
		action.getCoords().second
	).getEnemy().lock().get() != nullptr)
		return false;
	if (this->player->getX() == action.getCoords().first &&
		this->player->getY() == action.getCoords().second)
	{
		ActionAttack act(
			action.getEntity(),
			*this->player.get()
		);
		this->handleAction(act);
	}
	/* changing array */
	this->field->getCell(
		action.getCoords().first,
		action.getCoords().second
	).setEnemy(
		this->field->getCell(
			action.getEntity().getX(),
			action.getEntity().getY()
		).getEnemy().lock()
	);
	this->field->getCell(
		action.getEntity().getX(),
		action.getEntity().getY()
	).setEnemy(nullptr);

	return true;
}

bool GameObserver::moveLogicForPlayer(ActionMove& action)
{
	Cell& target_cell = this->field->getCell(
		action.getCoords().first,
		action.getCoords().second
	);

	this->game_object.increaseStepsCount();

	if (target_cell.getEnemy().lock().get())
	{
		ActionAttack act(
			action.getEntity(),
			*target_cell.getEnemy().lock().get()
		);
		this->handleAction(act);
		return false;
	}
	if (target_cell.getItem().lock().get())
	{
		ActionPickItem act(
			action.getEntity(),
			*target_cell.getItem().lock().get()
		);
		this->handleAction(act);
	}
	// move on the field
	return true;
}

bool GameObserver::handleAction(ActionMove& action)
{
	Cell& target_cell = this->field->getCell(
		action.getCoords().first,
		action.getCoords().second
	);

	if (target_cell.getHasWall())
		return false;
	if (action.getIsEnemy())
	{
		if (!this->moveLogicForEnemy(action))
			return false;
	}
	else
	{
		if (!this->moveLogicForPlayer(action))
			return true;
	}

	Logger::instance().handleAction(action);

	action.getEntity().moveTo(
		action.getCoords().first,
		action.getCoords().second
	);

	size_t draw_layer = 2;
	ActionAddDrawable actDraw(action.getEntity(), draw_layer);
	this->handleAction(actDraw);

	if (!action.getIsEnemy() && this->field->getEnd() == action.getCoords())
	{
		ActionPlayerReachEnd act(
			action.getEntity(), 
			action.getCoords()
		);
		return this->handleAction(act);
	}
	return true;
}

bool GameObserver::handleAction(ActionAttack& action)
{
	size_t damage = action.getEntity1().getDamage();
	size_t shield = action.getEntity2().getShield();
	int dhealth = damage - shield;
	action.setDamageCaused(dhealth > 0 ? dhealth : 0);
	
	Logger::instance().handleAction(action);

	if (dhealth > 0)
		action.getEntity2().changeHealth(-dhealth);

	return dhealth > 0;
}

bool GameObserver::handleAction(ActionDeleteItem& action)
{
	this->renderer->removeObject(action.getItem());
	Logger::instance().handleAction(action);
	this->field->getCell(
		action.getItem().getX(),
		action.getItem().getY()
	).setItem(nullptr);
	return true;
}

bool GameObserver::handleAction(ActionDeleteEnemy& action)
{
	this->renderer->removeObject(action.getEnemy());
	this->game_object.increaseEnemiesKilled();

	Logger::instance().handleAction(action);
	
	this->field->getCell(
		action.getEnemy().getX(),
		action.getEnemy().getY()
	).setEnemy(nullptr);
	return true;
}

bool GameObserver::handleAction(ActionPickItem& action)
{
	if (!action.getEntity().canPickItem()) 
		return false;	
	Logger::instance().handleAction(action);

	action.getItem().onPickUp(
		action.getEntity()
	);
	return true;
}

bool GameObserver::handleAction(ActionAddDrawable& action)
{
	this->renderer.get()->addObject(
		action.getDrawable(),
		60, 60,
		action.getDrawLayer()
	);
	return true;
}

bool GameObserver::handleAction(ActionPlayerReachEnd& action)
{
	Logger::instance().handleAction(action);
	if (this->game_object.isCompleted())
	{
		Logger::instance().write("Level completed!\n");
		this->game_object.getGame().popState();
	}
	else
	{
		Logger::instance().write("Level not been completed yet!\n");
	}	
	return true;
}

bool GameObserver::handleAction(ActionPlayerDied& action)
{
	Logger::instance().handleAction(action);
	this->game_object.getGame().exit(); // or exit

	return true;
}
