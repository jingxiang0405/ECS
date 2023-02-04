#include "ecs.h"
#include <iostream>
int main(){
    ECS::EntityManager em(10);
    ECS::Entity e = em.newEntity(ECS::Transform2D);
    std::cout << em.getEntityType(e) << "\n";
    std::cout << ECS::Transform2D;
}
