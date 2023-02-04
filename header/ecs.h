#ifndef ECS_H
#define ECS_H
#include <queue>
//
namespace ECS {

// Entity
using Entity = unsigned int;

using EntityType = enum { NA = 0, Transform2D, Transform3D, Graphic };
class EntityManager {
public:
    EntityManager(Entity maxEntitiesCount);
    ~EntityManager();
    Entity newEntity(EntityType type);
    void destroyEntity(Entity entity);
    EntityType getEntityType(Entity entity);

private:
    EntityType *_EntityTypes_;
    std::queue<Entity> _EntitiesPool_;
    uint32_t _EntitiesCount_ = 0;
};

// Component
// struct Graphic {
//     SDL_Texture *texture;
//     SDL_Rect rect;
// }

} // namespace ECS
#endif
