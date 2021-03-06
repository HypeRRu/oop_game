#include "../../inc/states/state_gameplay.h"
#include "../../inc/saves/gameplay_saver.h"

StateGameplay::StateGameplay(
	Game& game,
	std::shared_ptr<Renderer> _renderer
): game_object{std::ref(game)}, renderer{_renderer}, is_paused{false}
{
	GameConfig& config = GameConfig::instance();

	this->field = std::make_shared<Field>(
		config.getFieldSize().first,
		config.getFieldSize().second
	);
	this->generateLevel();
	this->configureObserver();
	this->setEventHandler(
		std::make_shared<GameplayEventHandler>(*this)
	);
	this->is_running = false;
	this->time_to_update = 0;
	this->steps_to_update = 0;

	this->enemies_killed = 0;
	this->steps_count = 0;

	this->initRules();
}

StateGameplay::StateGameplay(
	Game& game,
	std::shared_ptr<Renderer> _renderer,
	std::shared_ptr<Field> _field,
	std::shared_ptr<Player> _player,
	int _enemies_killed,
	int _steps_count
): game_object{std::ref(game)}, renderer{_renderer}, is_paused{false}
{
	this->field = _field;
	this->player = _player;
	this->configureObserver();
	this->setEventHandler(
		std::make_shared<GameplayEventHandler>(*this)
	);
	this->is_running = false;
	this->time_to_update = 0;
	this->steps_to_update = 0;

	this->enemies_killed = _enemies_killed;
	this->steps_count = _steps_count;

	this->initRules();
}

StateGameplay::~StateGameplay()
{
	this->renderer->flushObjects();
}

void StateGameplay::initDraw() const
{
	Field::iterator iterator = this->field->begin();
	while (iterator != this->field->end())
	{
		auto enemy = (*iterator).getEnemy().lock();
		auto item = (*iterator).getItem().lock();
		if (enemy.get())
			enemy->spawn();
		if (item.get())
			item->onAdd();
		++iterator;
	}

	this->field->onCellsAdded();
	this->player->spawn();
}

void StateGameplay::initRules()
{
	this->checker = std::make_shared<RuleChecker<
		EnemiesToKillRule<4>, 
		MaxStepsRule<100>,
		MoneyRule<1>
	>>();
}

bool StateGameplay::generateLevel()
{
	/* field generation */
	FieldGenerator field_generator(
		this->field->getWidth(),
		this->field->getHeight()
	);
	auto generated_cells = field_generator.generateCells();
	this->field->setStart(field_generator.getStart());
	this->field->setEnd(field_generator.getEnd());

	for (size_t y = 0; y < this->field->getHeight(); y++)
	{
		for (size_t x = 0; x < this->field->getWidth(); x++)
		{
			this->field->setCell(
				x, 
				y, 
				generated_cells.at(y * this->field->getWidth() + x).get()
			);
		}
	}

	/* player generation */
	this->player = std::make_shared<Player>(
		this->field->getStart().first,
		this->field->getStart().second
	);

	/* items generation */
	ItemsGenerator<> item_generator(*this->field.get());
	auto items = item_generator.generateItems();
	for (auto _item: items)
	{
		this->field->getCell(
			_item.first.first,
			_item.first.second
		).setItem(_item.second);
	}

	/* enemies generation */
	EnemiesGenerator<> enemies_generator(*this->field.get());
	auto enemies = enemies_generator.generateEnemies();
	for (auto _enemy: enemies)
	{
		this->field->getCell(
			_enemy.first.first,
			_enemy.first.second
		).setEnemy(_enemy.second);
	}

	return true;
}

std::string StateGameplay::getSave() const
{
	std::shared_ptr<GameplaySaver> saver = this->createSaver();
	return saver->save();
}

bool StateGameplay::update(int time_passed)
{
	if (this->isPaused())
		return false;
	this->time_to_update += time_passed;
	if (
		std::abs(this->steps_count - this->steps_to_update) < 
		GameConfig::instance().getEnemiesWalk()
	)
		return true;	
	std::set<BaseEnemy*> update_list;
	if (this->time_to_update >= 1500)
	{
		this->steps_to_update = this->steps_count;
		this->time_to_update = 0;
		Field::iterator iterator = this->field->begin();
		while (iterator != this->field->end())
		{
			auto enemy = (*iterator).getEnemy().lock();
			if (
				enemy.get() &&
				update_list.find(enemy.get()) == update_list.end()
			)
			{
				enemy->update();
				update_list.insert(enemy.get());
			}
			++iterator;
		}
	}
	update_list.clear();
	return true;
}

void StateGameplay::showing() const
{
	this->renderer->removeWindowRendering();
	this->initDraw();
}

void StateGameplay::configureObserver()
{
	this->observer = std::make_shared<GameObserver>(*this);
	this->renderer->setObserver(this->observer);
	this->field->setObserver(this->observer);
	this->player->setObserver(this->observer);

	Field::iterator iterator = this->field->begin();
	while (iterator != this->field->end())
	{
		auto enemy = (*iterator).getEnemy().lock();
		auto item = (*iterator).getItem().lock();
		if (enemy.get())
			enemy->setObserver(this->observer);
		if (item.get())
			item->setObserver(this->observer);
		++iterator;
	}
}

bool StateGameplay::pause(bool pause_state)
{
	if (pause_state)
	{
		auto state_pause = std::make_unique<StatePause>(
			this->getGame(), 
			this->renderer,
			*this
		);
		this->getGame().newState(std::move(state_pause));
	}
	
	this->is_paused = false;
	return this->isPaused();
}

bool StateGameplay::isCompleted()
{
	return this->checker->checkRule(*this);
}

bool StateGameplay::isPaused() const
{
	return this->is_paused;
}

Game& StateGameplay::getGame() const
{
	return this->game_object.get();
}

std::shared_ptr<Renderer> StateGameplay::getRendererPtr() const
{
	return this->renderer;
}

std::shared_ptr<Field> StateGameplay::getFieldPtr() const
{
	return this->field;
}

std::shared_ptr<Player> StateGameplay::getPlayerPtr() const
{
	return this->player;
}

int StateGameplay::getEnemiesKilled() const
{
	return this->enemies_killed;
}

void StateGameplay::increaseEnemiesKilled()
{
	++this->enemies_killed;
}

int StateGameplay::getStepsCount() const
{
	return this->steps_count;
}

void StateGameplay::increaseStepsCount()
{
	++this->steps_count;
}

std::shared_ptr<GameplaySaver> StateGameplay::createSaver() const
{
	return std::make_shared<GameplaySaver>(
		this->player,
		this->field,
		this->enemies_killed,
		this->steps_count
	);
}
