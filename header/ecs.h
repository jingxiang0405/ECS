#ifndef ECS_H
#define ECS_H

// debug
#include <array>
#include <assert.h>
#include <bitset>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <queue>
#include <set>
#include <typeinfo>
#include <unordered_map>

namespace ECS {

const uint32_t MAX_ENTITIES_COUNT = 10000;
using ComponentType = uint16_t;
const ComponentType MAX_COMPONENT_TYPE_COUNT = 32;

using Signature = std::bitset<MAX_COMPONENT_TYPE_COUNT>;

// Entities
using Entity = unsigned int;
class EntityManager {
public:
    EntityManager();
    ~EntityManager();
    Entity createEntity();
    void destroyEntity(Entity entity);
    void setSignature(Entity entity, Signature signature);
    Signature getSignature(Entity entity);

private:
    Signature _EntitySigns_[MAX_ENTITIES_COUNT];
    std::queue<Entity> _EntitiesPool_;
    uint32_t _EntitiesCount_ = 0;
};

// Components
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(Entity entity) = 0;
};
/*
*
* Use "Entity" to look up the actual array index.
*
*/
template <typename T> class ComponentArray : public IComponentArray {
public:
    void insertData (Entity entity, T component) {
        assert(_EntityToIndexMap_.find(entity) == _EntityToIndexMap_.end() && "Components added to the same Entity more than once");
        size_t newIndex = _size_;
        _EntityToIndexMap_[entity] = newIndex;
        _IndexToEntityMap_[newIndex] = entity;
        components[newIndex] = component;
        ++_size_;
    }

    void removeData (Entity entity) {
        assert(_EntityToIndexMap_.find(entity) != _EntityToIndexMap_.end() && "Entity does not exist");
        size_t remove_index = _EntityToIndexMap_[entity];
        size_t last_index = _size_ - 1;
        components[remove_index] = components[last_index];

        Entity last_entity = _IndexToEntityMap_[last_entity];
        _EntityToIndexMap_[last_entity] = remove_index;
        _IndexToEntityMap_[remove_index] = last_entity;

        _EntityToIndexMap_.erase(entity);
        _IndexToEntityMap_.erase(last_index);

        --_size_;
    }

    T& getComponent (Entity entity) {
        assert(_EntityToIndexMap_.find(entity) != _EntityToIndexMap_.end() && "retrieving none exist component");
        return components[_EntityToIndexMap_[entity]];
    }

    void entityDestroyed (Entity entity) override {
        if (_EntityToIndexMap_.find(entity) != _EntityToIndexMap_.end()) { removeData(entity); }
    }

private:
    T components[MAX_ENTITIES_COUNT];
    std::unordered_map<Entity, size_t> _EntityToIndexMap_;
    std::unordered_map<size_t, Entity> _IndexToEntityMap_;
    size_t _size_ = 0;
}; // class ComponentArray

class ComponentManager {
public:
    template <typename T> void registerComponent () {
        const char *typeName = typeid(T).name();
        assert(_ComponentTypeMap_.find(typeName) == _ComponentTypeMap_.end() && "Registering component type more than once");
        std::cout << "register : type=" << typeName << ", "
                  << "ComponentType=" << _RegisteredCount_ << "\n";
        _ComponentTypeMap_.insert(std::pair<const char *, ComponentType>(typeName, _RegisteredCount_));
        ++_RegisteredCount_;

        _ComponentMap_.insert(std::pair<const char *, std::shared_ptr<IComponentArray> >(typeName, std::make_shared<ComponentArray<T> >()));
    }

    template <typename T> void addComponent (Entity entity, T component) { getComponentArray<T>()->insertData(entity, component); }

    template <typename T> void removeComponent (Entity entity) { getComponentArray<T>()->removeData(entity); }

    template <typename T> ComponentType getComponentType () {
        const char *typeName = typeid(T).name();
        assert(_ComponentTypeMap_.find(typeName) != _ComponentTypeMap_.end() && "Component did not register");
        return _ComponentTypeMap_[typeName];
    }

    template <typename T> T& getComponent (Entity entity) { return getComponentArray<T>()->getComponent(entity); }

    void entityDestroyed (Entity entity) {

        for (auto const& pair : _ComponentMap_) {

            auto const& component = pair.second;
            component->entityDestroyed(entity);
        }
    }

private:
    std::unordered_map<const char *, ComponentType> _ComponentTypeMap_;
    std::unordered_map<const char *, std::shared_ptr<IComponentArray> > _ComponentMap_;
    ComponentType _RegisteredCount_ = 0;

    template <typename T> std::shared_ptr<ComponentArray<T>> getComponentArray () {
        const char *typeName = typeid(T).name();
        assert(_ComponentTypeMap_.find(typeName) != _ComponentTypeMap_.end() && "Component did not register");
        return std::static_pointer_cast<ComponentArray<T>>(_ComponentMap_[typeName]);
    }
};
// System
class System {
public:
    std::set<Entity> _Entities_;
};

class SystemManager {
public:
    template <typename T> std::shared_ptr<T> registerSystem () {
        const char *typeName = typeid(T).name();
        assert(_Systems_.find(typeName) == _Systems_.end() && "Registering system more than once.");
        auto system = std::make_shared<T>();
        _Systems_.insert({typeName, system});
        return system;
    }

    template <typename T> void setSignature (Signature signature) {
        const char *typeName = typeid(T).name();

        assert(_Systems_.find(typeName) != _Systems_.end() && "System used before registered.");

        // Set the signature for this system
        _SystemSigns_.insert({typeName, signature});
    }

    void entityDestroyed (Entity entity) {
        // Erase a destroyed entity from all system lists
        // mEntities is a set so no check needed
        for (auto const& pair : _Systems_) {
            auto const& system = pair.second;

            system->_Entities_.erase(entity);
        }
    }

    void entitySignatureChanged (Entity entity, Signature entitySignature) {
        // Notify each system that an entity's signature changed
        for (auto const& pair : _Systems_) {
            auto const& type = pair.first;
            auto const& system = pair.second;
            auto const& systemSignature = _SystemSigns_[type];

            // Entity signature matches system signature - insert into set
            if ((entitySignature & systemSignature) == systemSignature) {
                system->_Entities_.insert(entity);
            }
            // Entity signature does not match system signature - erase from set
            else {
                system->_Entities_.erase(entity);
            }
        }
    }

private:
    std::unordered_map<const char *, Signature> _SystemSigns_; // type, signatures
    std::unordered_map<const char *, std::shared_ptr<System> > _Systems_;
};

class Coordinator {
public:
    Coordinator();
    ~Coordinator();
    Entity createEntity();
    void destroyEntity(Entity entity);
    template <typename T> void registerComponent () { _ComponentManager_->registerComponent<T>(); }
    
    template <typename T> void addComponent(Entity entity, T component) {
        _ComponentManager_->addComponent(entity, component);

        auto signature = _EntityManager_->getSignature(entity);
        signature.set(_ComponentManager_->getComponentType<T>(), true);
        _EntityManager_->setSignature(entity, signature);
        _SystemManager_->entitySignatureChanged(entity, signature);
    }
    template <typename T> void removeComponent(Entity entity) {
        _ComponentManager_->removeComponent<T>(entity);

		auto signature = _EntityManager_->getSignature(entity);
		signature.set(_ComponentManager_->getComponentType<T>(), false);
		_EntityManager_->setSignature(entity, signature);

		_SystemManager_->entitySignatureChanged(entity, signature);
    }

    template<typename T>
    T& getComponent(Entity entity){
        return _ComponentManager_->getComponent<T>(entity);
    }

    template<typename T>
    ComponentType getComponentType(){
        return _ComponentManager_->getComponentType<T>();
    }

    template<typename T>
    std::shared_ptr<T> registerSystem(){
        return _SystemManager_->registerSystem<T>();
    }

    template<typename T>
	void setSystemSignature(Signature signature)
	{
		_SystemManager_->setSignature<T>(signature);
	}

    

private:
    ComponentManager *_ComponentManager_;
    EntityManager *_EntityManager_;
    SystemManager *_SystemManager_;
};
} // namespace ECS

#endif
