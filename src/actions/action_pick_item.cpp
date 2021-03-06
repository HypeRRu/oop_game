#include "../../inc/actions/action_pick_item.h"

ActionPickItem::ActionPickItem(BaseEntity& entity, BaseItem& _item): 
	entity{entity}, item{_item}
{}
	
BaseItem& ActionPickItem::getItem() const
{
	return this->item;
}

BaseEntity& ActionPickItem::getEntity() const
{
	return this->entity;
}
