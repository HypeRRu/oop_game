#ifndef RULE_CHECKER_H
#define RULE_CHECKER_H

#include "../interfaces/ichecker.h"

template <class ...rules> 
class RuleChecker: public IChecker
{
public:
	bool checkRule(StateGameplay& state) const
	{
		if (!sizeof...(rules))
			return true;
		return (... && 
			rules{}.exec(state) // creating object of type and calling method
		);
	}
};

#endif