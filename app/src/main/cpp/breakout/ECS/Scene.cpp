#include "Scene.h"
#include "Entity.h"

template<typename... Component>
static void copyComponent(entt::registry &dst, entt::registry &src,
                          const std::unordered_map<UUID, entt::entity> &enttMap) {
    ([&]() {
        auto view = src.view<Component>();
        for (auto srcEntity: view) {
            entt::entity dstEntity = enttMap.at(src.get<IDComponent>(srcEntity).ID);

            auto &srcComponent = src.get<Component>(srcEntity);
            dst.emplace_or_replace<Component>(dstEntity, srcComponent);
        }
    }(), ...);
}

template<typename... Component>
static void copyComponent(ComponentGroup<Component...>, entt::registry &dst, entt::registry &src,
                          const std::unordered_map<UUID, entt::entity> &enttMap) {
    copyComponent<Component...>(dst, src, enttMap);
}

template<typename... Component>
static void copyComponentIfExists(Entity dst, Entity src) {
    ([&]() {
        if (src.hasComponent<Component>())
            dst.addOrReplaceComponent<Component>(src.getComponent<Component>());
    }(), ...);
}

template<typename... Component>
static void copyComponentIfExists(ComponentGroup<Component...>, Entity dst, Entity src) {
    copyComponentIfExists<Component...>(dst, src);
}
std::shared_ptr<Scene> Scene::copy(Scene& other){
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();


    auto& srcSceneRegistry = other.m_Registry;
    auto& dstSceneRegistry = newScene->m_Registry;
    std::unordered_map<UUID, entt::entity> enttMap;

    // Create entities in new scene
    auto idView = srcSceneRegistry.view<IDComponent>();
    for (auto e : idView)
    {
        UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
        const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
        Entity newEntity = newScene->createEntityWithUUID(uuid, name);
        enttMap[uuid] = (entt::entity)newEntity;
    }

    // Copy components (except IDComponent and TagComponent)
    copyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

    return newScene;
}

std::shared_ptr<Scene> Scene::copy(std::shared_ptr<Scene> other) {
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();


    auto& srcSceneRegistry = other->m_Registry;
    auto& dstSceneRegistry = newScene->m_Registry;
    std::unordered_map<UUID, entt::entity> enttMap;

    // Create entities in new scene
    auto idView = srcSceneRegistry.view<IDComponent>();
    for (auto e : idView)
    {
        UUID uuid = srcSceneRegistry.get<IDComponent>(e).ID;
        const auto& name = srcSceneRegistry.get<TagComponent>(e).Tag;
        Entity newEntity = newScene->createEntityWithUUID(uuid, name);
        enttMap[uuid] = (entt::entity)newEntity;
    }

    // Copy components (except IDComponent and TagComponent)
    copyComponent(AllComponents{}, dstSceneRegistry, srcSceneRegistry, enttMap);

    return newScene;
}
Entity Scene::createEntityWithUUID(UUID uuid, const std::string& name ){
    Entity entt = {m_Registry.create(), this};
    entt.addComponent<IDComponent>(uuid);
    entt.addComponent<TransformComponent>();
    auto &tag = entt.addComponent<TagComponent>();
    tag.Tag = name.empty() ? "Entity" : name;

    m_Entities[uuid] = entt;
    return entt;
}

Entity Scene::createEntity(const std::string &name) {
    return createEntityWithUUID(UUID(), name);
}

void Scene::destroyEntity(Entity entity) {
    m_Entities.erase(entity.getUuid());
    m_Registry.destroy(entity);
}

Entity Scene::duplicateEntity(Entity entity) {
    std::string name = entity.getName();
    Entity newEntity = createEntity(name);
    copyComponentIfExists(AllComponents{}, newEntity, entity);
    return newEntity;
}

Entity Scene::findEntityByName(std::string_view name) {
    auto view = m_Registry.view<TagComponent>();
    for (auto entity: view) {
        const TagComponent &tc = view.get<TagComponent>(entity);
        if (tc.Tag == name)
            return Entity{entity, this};
    }
    return {};
}

Entity Scene::getEntityByUuid(UUID uuid) {
    if (m_Entities.find(uuid) != m_Entities.end()) {
        return {m_Entities.at(uuid), this};
    }

    return {};
}
