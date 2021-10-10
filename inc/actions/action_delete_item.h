#ifndef ACTION_DELETE_ITEM_H
#define ACTION_DELETE_ITEM_H

#include "action.h"
#include "../items/base_item.h"

class ActionDeleteItem: public Action
{
public:
	ActionDeleteItem(BaseItem& _item);
	ActionType getActionType();

	BaseItem& getItem();
protected:
	/* context */
	BaseItem& item;
};

#endif