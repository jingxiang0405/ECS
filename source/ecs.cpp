#include "../header/ecs.h"

using namespace ECS;
EntityManager::EntityManager() {
    for (Entity e = 0; e < MAX_ENTITIES_COUNT; ++e)
        this->_EntitiesPool_.push(e);
}

EntityManager::~EntityManager() {}

Entity EntityManager::createEntity() {
    Entity e = std::move(this->_EntitiesPool_.front());
    this->_EntitiesPool_.pop();
    ++this->_EntitiesCount_;
    return e;
}

void EntityManager::destroyEntity(Entity entity) {
    this->_EntitySigns_[entity].reset();
    this->_EntitiesPool_.push(entity);
    --this->_EntitiesCount_;
}

void EntityManager::setSignature (Entity entity, Signature signature) { this->_EntitySigns_[entity] = signature; }
Signature EntityManager::getSignature(Entity entity) { return this->_EntitySigns_[entity]; }

Coordinator::Coordinator() {
    this->_EntityManager_ = new EntityManager();
    this->_ComponentManager_ = new ComponentManager();
    this->_SystemManager_ = new SystemManager();
}

Coordinator::~Coordinator() {
    delete this->_EntityManager_;
    delete this->_ComponentManager_;
    delete this->_SystemManager_;
}
Entity Coordinator::createEntity() { return this->_EntityManager_->createEntity(); }

void Coordinator::destroyEntity(Entity entity) {
    this->_EntityManager_->destroyEntity(entity);
    this->_ComponentManager_->entityDestroyed(entity);
    this->_SystemManager_->entityDestroyed(entity);
}
