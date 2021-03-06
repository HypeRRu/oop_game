#ifndef BASE_OBSERVED_H
#define BASE_OBSERVED_H

#include "../interfaces/observed.h"

class GameObserver;

class BaseObserved: public Observed
{
public:
	void setObserver(std::shared_ptr<GameObserver> observer);
	GameObserver& getObserver() const;

	bool operator ==(const BaseObserved& other) const;
	bool operator !=(const BaseObserved& other) const;
protected:
	std::weak_ptr<GameObserver> observer;
};

#endif
