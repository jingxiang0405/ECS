#include "ecs.h"

using namespace ECS;
EntityManager::EntityManager(Entity maxEntitiesCount) {
    this->_EntityTypes_ = new EntityType[maxEntitiesCount]();
    for (Entity e = 0; e < maxEntitiesCount; ++e)
        this->_EntitiesPool_.push(e);
}

EntityManager::~EntityManager() {
    delete []this->_EntityTypes_;
}

Entity EntityManager::newEntity(EntityType type) {
    Entity e = std::move(this->_EntitiesPool_.front());
    this->_EntitiesPool_.pop();
    this->_EntityTypes_[e] = type;
    ++this->_EntitiesCount_;
    return e;
}

void EntityManager::destroyEntity(Entity entity) {
    this->_EntityTypes_[entity] = NA;
    this->_EntitiesPool_.push(entity);
    --this->_EntitiesCount_;
}

EntityType EntityManager::getEntityType(Entity entity) { return this->_EntityTypes_[entity]; }
