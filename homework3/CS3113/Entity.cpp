#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f},
                   mVelocity {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE}, 
                   mTexture {}, mAngle {0.0f}, 
                   mEntityStatus{ACTIVE},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f}, mScale {scale}, 
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)}, 
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };


bool Entity::isColliding(Entity *other) const 
{
    if (!other->isActive()) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) - 
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) - 
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::update(float deltaTime)
{
    if(mEntityStatus == INACTIVE) return;

    mPosition.x += mVelocity.x * deltaTime;
    mPosition.y += mVelocity.y * deltaTime;
}

void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea = {
        0.0f,
        0.0f,
        static_cast<float>(mTexture.width),
        static_cast<float>(mTexture.height)
    };

    // Destination rectangle – centred on gPosition
    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    // Origin inside the source texture (centre of the texture)
    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    // Render the texture on screen
    DrawTexturePro(
        mTexture, 
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

    // displayCollider();
}