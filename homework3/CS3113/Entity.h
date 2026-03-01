#ifndef ENTITY_H
#define ENTITY_H

#include "cs3113.h"

enum EntityStatus { ACTIVE, INACTIVE              };
enum EntityType   { PLAYER, BALL, NONE };

class Entity
{
private:
    Vector2 mPosition;
    Vector2 mVelocity;

    Vector2 mScale;
    Vector2 mColliderDimensions;
    
    Texture2D mTexture;

    int mSpeed;
    float mAngle;

    EntityStatus mEntityStatus = ACTIVE;
    EntityType   mEntityType;

public:
    static constexpr int   DEFAULT_SIZE          = 250;
    static constexpr int   DEFAULT_SPEED         = 200;

    Entity();
    Entity(Vector2 position, Vector2 scale, const char *textureFilepath, 
        EntityType entityType);
    ~Entity();

    void update(float deltaTime);
    void render();



    bool isColliding(Entity* other) const;

    Vector2     getPosition()              const { return mPosition;              }
    Vector2     getVelocity()              const { return mVelocity;              }         
    Vector2     getScale()                 const { return mScale;                 }
    Vector2     getColliderDimensions()    const { return mColliderDimensions; }
    bool    isActive()              const { return mEntityStatus == ACTIVE; }               
 

    void setPosition(Vector2 newPosition)
        { mPosition = newPosition;                 }
    void setVelocity(Vector2 newVelocity) { mVelocity = newVelocity; }
    void setScale(Vector2 newScale)
        { mScale = newScale;                       }
    void setTexture(const char *textureFilepath)
        { mTexture = LoadTexture(textureFilepath); }
    void setColliderDimensions(Vector2 newDimensions) 
        { mColliderDimensions = newDimensions;     }
    void setAngle(float newAngle) 
        { mAngle = newAngle;                       }
};

#endif // ENTITY_H