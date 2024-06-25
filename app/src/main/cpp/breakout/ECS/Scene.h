/*
MIT License

Copyright (c) 2023 Victor Falcon Zaro

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#pragma once

#include <Core/UUID.h>
#include <Common.h>
#include <Entt/entt.hpp>
#include "Components.h"


class Entity;
//! Scene class
/*
* Holds all the entities and manages them
*/
class Scene
{
public:
	//! Default constructor
	Scene() = default;

	//! Default destructor
	~Scene() = default;

    //! Copy
    static std::shared_ptr<Scene> copy(Scene& other);
    static std::shared_ptr<Scene> copy(std::shared_ptr<Scene> other);

	DEFAULT_MOVE_AND_COPY(Scene)

	//! Entity creation function
	//! @param name Name of the created entity
	//! @return The created entity
	Entity createEntity(const std::string &name);

    //! Entity creation function
    //! @param uuid UUID of the created entity
    //! @param name Name of the created entity
    //! @return The created entity
    Entity createEntityWithUUID(UUID uuid, const std::string& name = std::string());

	//! Entity destruction function
	//! @param entity The entity being destroyed
	void destroyEntity(Entity entity);

	//! Duplicate entity function
	//! @param entity The entity to duplicate
	//! @return The duplicated entity
	Entity duplicateEntity(Entity entity);

	//! Finds an entity with the specified name
	//! @param name Identifier of the entity
	//! @return Entity found on the search
	Entity findEntityByName(std::string_view name);

    //! Finds an entity with an specific UUID
    //! @param uuid The uuid to search
    //! @return The found entity
	Entity getEntityByUuid(UUID uuid);

	//! Templated function for all the entities with the components given
	//! @return A view of all the entities with the given components
	template<typename... Components>
	auto getAllEntitiesWith() const
	{
		return m_Registry.view<Components...>();
	}

private:
	entt::registry m_Registry; /**< Scene entity registry */
	std::unordered_map<UUID, entt::entity> m_Entities{}; /**< Registered entities map */

	friend class Entity;
};

