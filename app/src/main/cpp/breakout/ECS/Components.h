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

#include <string>

#include <Core/UUID.h>
#include <Math/Math.h>


struct IDComponent {
    UUID ID;

    IDComponent() = default;

    IDComponent(const UUID &id) : ID(id) {}

    IDComponent(const IDComponent &) = default;
};

struct TagComponent {
    std::string Tag;


    TagComponent() = default;

    TagComponent(const TagComponent &) = default;

    TagComponent(const std::string &tag)
            : Tag(tag) {}
};

struct TransformComponent {
    V3 Translation = {0.0f, 0.0f, 0.0f};
    Quaternion Rotation = {1.0f, 0.0f, 0.0f, 0.0f};
    V3 Scale = {1.0f, 1.0f, 1.0f};
    bool Enabled = true;

    TransformComponent() = default;

    TransformComponent(const TransformComponent &) = default;

    TransformComponent(V3 translation, Quaternion rotation, V3 scale)
            : Translation(translation), Rotation(rotation), Scale(scale) {}

    TransformComponent(const V3 &translation)
            : Translation(translation) {}

    TransformComponent(const Mat4 &mat) {
        Math::decomposeTransform(mat, Translation, Rotation, Scale);
    }

    Mat4 getTransform() const {
        return Math::createTransform(Translation, Rotation, Scale);
    }

    Mat4 getSpriteTransform() const {
        return Math::createSpriteTransform(Translation, Rotation, Scale);
    }
};

struct SpriteComponent {
    V3 Color = V3{1.0};
    u32 Texture = 0;

    SpriteComponent() = default;

    SpriteComponent(const SpriteComponent &) = default;
    SpriteComponent(u32 texture)
        : Color(1.0), Texture(texture){}
    SpriteComponent(V3 color, u32 texture)
            : Color(color), Texture(texture) {}
};

struct PlayerComponent {
    u32 Lives = 0;
    u32 Speed = 0;

    PlayerComponent() = default;

    PlayerComponent(const PlayerComponent &) = default;

    PlayerComponent(u32 lives, u32 speed)
            : Lives(lives), Speed(speed) {}
};

struct BallComponent {
    V2 Speed = {};
    f32 Radius = 0;

    BallComponent() = default;
    BallComponent(const BallComponent&) = default;
    BallComponent(V2 speed, f32 radius)
        : Speed(speed), Radius(radius){}
};


enum class TileType {
    SOLID,
    BREAKABLE,
    MAX
};

struct TileComponent {
    TileType Type = TileType::SOLID;

    TileComponent() = default;

    TileComponent(const TileComponent &) = default;

    TileComponent(TileType type)
            : Type(type) {}
};

struct TextComponent{
    std::string Text;
    V3 Color;

    TextComponent() = default;
    TextComponent(const TextComponent&) = default;
    TextComponent(std::string_view text, V3 color)
        : Text(text), Color(color){}
};

template<typename... Component>
struct ComponentGroup {
};

using AllComponents
        = ComponentGroup<IDComponent, TagComponent, TransformComponent,
        SpriteComponent, PlayerComponent, TileComponent, BallComponent, TextComponent>;
