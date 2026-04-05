#include "Entity.h"

Entity::Entity() : mPosition {0.0f, 0.0f}, mMovement {0.0f, 0.0f},
                   mVelocity {0.0f, 0.0f}, mAcceleration {0.0f, 0.0f},
                   mScale {DEFAULT_SIZE, DEFAULT_SIZE},
                   mColliderDimensions {DEFAULT_SIZE, DEFAULT_SIZE},
                   mTexture {NULL}, mTextureType {SINGLE}, mAngle {0.0f},
                   mSpriteSheetDimensions {}, mDirection {RIGHT},
                   mAnimationAtlas {{}}, mAnimationIndices {}, mFrameSpeed {0},
                   mEntityType {NONE} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
    EntityType entityType) : mPosition {position}, mVelocity {0.0f, 0.0f},
    mAcceleration {0.0f, 0.0f}, mScale {scale}, mMovement {0.0f, 0.0f},
    mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)},
    mTextureType {SINGLE}, mDirection {RIGHT}, mAnimationAtlas {{}},
    mAnimationIndices {}, mFrameSpeed {0}, mSpeed {DEFAULT_SPEED},
    mAngle {0.0f}, mEntityType {entityType} { }

Entity::Entity(Vector2 position, Vector2 scale, const char *textureFilepath,
        TextureType textureType, Vector2 spriteSheetDimensions, std::map<Direction,
        std::vector<int>> animationAtlas, EntityType entityType) :
        mPosition {position}, mVelocity {0.0f, 0.0f},
        mAcceleration {0.0f, 0.0f}, mMovement { 0.0f, 0.0f }, mScale {scale},
        mColliderDimensions {scale}, mTexture {LoadTexture(textureFilepath)},
        mTextureType {ATLAS}, mSpriteSheetDimensions {spriteSheetDimensions},
        mAnimationAtlas {animationAtlas}, mDirection {RIGHT},
        mAnimationIndices {animationAtlas.at(RIGHT)},
        mFrameSpeed {DEFAULT_FRAME_SPEED}, mAngle { 0.0f },
        mSpeed { DEFAULT_SPEED }, mEntityType {entityType} { }

Entity::~Entity() { UnloadTexture(mTexture); };

void Entity::checkCollisionY(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) -
                              (collidableEntity->mColliderDimensions.y / 2.0f));

            if (mVelocity.y > 0)
            {
                mPosition.y -= yOverlap;
                mVelocity.y  = 0;
                mIsCollidingBottom = true;
            } else if (mVelocity.y < 0)
            {
                mPosition.y += yOverlap;
                mVelocity.y  = 0;
                mIsCollidingTop = true;

                if (collidableEntity->mEntityType == BLOCK)
                    collidableEntity->deactivate();
            }
        }
    }
}

void Entity::checkCollisionX(Entity *collidableEntities, int collisionCheckCount)
{
    for (int i = 0; i < collisionCheckCount; i++)
    {
        Entity *collidableEntity = &collidableEntities[i];

        if (isColliding(collidableEntity))
        {
            float yDistance = fabs(mPosition.y - collidableEntity->mPosition.y);
            float yOverlap  = fabs(yDistance - (mColliderDimensions.y / 2.0f) - (collidableEntity->mColliderDimensions.y / 2.0f));

            if (yOverlap < Y_COLLISION_THRESHOLD) continue;

            float xDistance = fabs(mPosition.x - collidableEntity->mPosition.x);
            float xOverlap  = fabs(xDistance - (mColliderDimensions.x / 2.0f) - (collidableEntity->mColliderDimensions.x / 2.0f));

            if (mVelocity.x > 0) {
                mPosition.x     -= xOverlap;
                mVelocity.x      = 0;
                mIsCollidingRight = true;
            } else if (mVelocity.x < 0) {
                mPosition.x    += xOverlap;
                mVelocity.x     = 0;
                mIsCollidingLeft = true;
            }
        }
    }
}

// from maps
void Entity::checkCollisionY(Map* map)
{
    if (map == nullptr) return;

    Vector2 topCentreProbe = { mPosition.x, mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topLeftProbe = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };
    Vector2 topRightProbe = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y - (mColliderDimensions.y / 2.0f) };

    Vector2 bottomCentreProbe = { mPosition.x, mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomLeftProbe = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };
    Vector2 bottomRightProbe = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y + (mColliderDimensions.y / 2.0f) };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ABOVE (jumping upward)
    if ((map->isSolidTileAt(topCentreProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(topLeftProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(topRightProbe, &xOverlap, &yOverlap))
        && mVelocity.y < 0.0f && xOverlap < map->getTileSize() * 0.4f)
    {
        mPosition.y += yOverlap * 1.01f;   // push down
        mVelocity.y = 0.0f;
        mIsCollidingTop = true;
    }

    // COLLISION BELOW (falling downward)
    if ((map->isSolidTileAt(bottomCentreProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(bottomLeftProbe, &xOverlap, &yOverlap) ||
        map->isSolidTileAt(bottomRightProbe, &xOverlap, &yOverlap)) && mVelocity.y > 0.0f)
    {
        mPosition.y -= yOverlap * 1.01f;   // push up
        mVelocity.y = 0.0f;
        mIsCollidingBottom = true;
    }
}

void Entity::checkCollisionX(Map* map)
{
    if (map == nullptr) return;

    Vector2 leftCentreProbe = { mPosition.x - (mColliderDimensions.x / 2.0f), mPosition.y };

    Vector2 rightCentreProbe = { mPosition.x + (mColliderDimensions.x / 2.0f), mPosition.y };

    float xOverlap = 0.0f;
    float yOverlap = 0.0f;

    // COLLISION ON RIGHT (moving right)
    if (map->isSolidTileAt(rightCentreProbe, &xOverlap, &yOverlap)
        && mVelocity.x > 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x -= xOverlap * 1.01f;   // push left
        mVelocity.x = 0.0f;
        mIsCollidingRight = true;
    }

    // COLLISION ON LEFT (moving left)
    if (map->isSolidTileAt(leftCentreProbe, &xOverlap, &yOverlap)
        && mVelocity.x < 0.0f && yOverlap >= 0.5f)
    {
        mPosition.x += xOverlap * 1.01;   // push right
        mVelocity.x = 0.0f;
        mIsCollidingLeft = true;
    }
}

bool Entity::isColliding(Entity *other) const
{
    if (!other->isActive() || other == this) return false;

    float xDistance = fabs(mPosition.x - other->getPosition().x) -
        ((mColliderDimensions.x + other->getColliderDimensions().x) / 2.0f);
    float yDistance = fabs(mPosition.y - other->getPosition().y) -
        ((mColliderDimensions.y + other->getColliderDimensions().y) / 2.0f);

    if (xDistance < 0.0f && yDistance < 0.0f) return true;

    return false;
}

void Entity::animate(float deltaTime)
{

    Direction animDirection = mDirection;

    //check if atlas has direciton before animating
    if (mAnimationAtlas.find(animDirection) == mAnimationAtlas.end())
    {
        if (mVelocity.x < 0.0f)      animDirection = LEFT;
        else if (mVelocity.x > 0.0f) animDirection = RIGHT;
        else                         animDirection = RIGHT; // default fallback
    }

    mAnimationIndices = mAnimationAtlas.at(animDirection);

    mAnimationTime += deltaTime;
    float framesPerSecond = 1.0f / mFrameSpeed;

    if (mAnimationTime >= framesPerSecond)
    {
        mAnimationTime = 0.0f;

        mCurrentFrameIndex++;
        mCurrentFrameIndex %= mAnimationIndices.size();
    }
}

void Entity::AIWander() {
    //fix when it hits wall
    if (mIsCollidingLeft || mIsCollidingRight || mIsCollidingTop || mIsCollidingBottom) {
        int dir = GetRandomValue(0, 3);
        switch (dir)
        {
        case 0: mDirection = LEFT;  moveLeft(); break;
        case 1: mDirection = RIGHT; moveRight(); break;
        case 2: mDirection = UP;    moveUp(); break;
        case 3: mDirection = DOWN;  moveDown(); break;
        }
        return;
    }

    switch (mDirection)
    {
    case LEFT:  moveLeft();  break;
    case RIGHT: moveRight(); break;
    case UP:    moveUp();    break;
    case DOWN:  moveDown();  break;
    }
}

void Entity::AIFollow(Entity *target)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 250.0f)
            mAIState = WALKING;
        break;

    case WALKING:
        if (mIsCollidingLeft || mIsCollidingRight || mIsCollidingTop || mIsCollidingBottom) {
            int direction = GetRandomValue(0, 3);
            switch (direction)
            {
            case 0: mDirection = LEFT;  moveLeft(); break;
            case 1: mDirection = RIGHT; moveRight(); break;
            case 2: mDirection = UP;    moveUp(); break;
            case 3: mDirection = DOWN;  moveDown(); break;
            }
            return;
        }

        // have it follow both up and down since env is 2d
        if (fabs(mPosition.x - target->getPosition().x) > fabs(mPosition.y - target->getPosition().y)){
            if (mPosition.x > target->getPosition().x) moveLeft();
            else moveRight();
        }
        else
        {
            if (mPosition.y > target->getPosition().y) moveUp();
            else moveDown();
        }
        break;

    default:
        break;
    }
}

/**
 * Smoothly moves this entity toward `target` using linear interpolation.
 *
 * FSM:
 *   IDLE      → switch to FOLLOWING once the player is within 350 units
 *   FOLLOWING → lerp mPosition toward target each frame
 *
 * lerp formula applied per component:
 *   mPosition.x = mPosition.x + (target.x - mPosition.x) * mLerpFactor * deltaTime
 *   mPosition.y = mPosition.y + (target.y - mPosition.y) * mLerpFactor * deltaTime
 */
void Entity::AILerp(Entity *target, float deltaTime)
{
    switch (mAIState)
    {
    case IDLE:
        if (Vector2Distance(mPosition, target->getPosition()) < 350.0f)
            mAIState = FOLLOWING;
        break;

    case FOLLOWING:
    {
        Vector2 targetPos = target->getPosition();
        float t = mLerpFactor * deltaTime;

        mPosition.x = mPosition.x + (targetPos.x - mPosition.x) * t;
        mPosition.y = mPosition.y + (targetPos.y - mPosition.y) * t;

        if (mPosition.x > targetPos.x) setDirection(LEFT);
        else                           setDirection(RIGHT);
        break;
    }

    default:
        break;
    }
}

void Entity::AIActivate(Entity *target, float deltaTime)
{
    switch (mAIType)
    {
    case WANDERER:
        AIWander();
        break;

    case FOLLOWER:
        AIFollow(target);
        break;

    case LERPER:
        AILerp(target, deltaTime);
        break;

    default:
        break;
    }
}

void Entity::update(float deltaTime, Entity *player, Entity *collidableEntities,
    int collisionCheckCount, Entity* blocks, int blockCount)
{
    if (mEntityStatus == INACTIVE) return;

    if (mEntityType == NPC) AIActivate(player, deltaTime);

    resetColliderFlags();

    if (mAIType != LERPER)
    {
        mVelocity.x = mMovement.x * mSpeed;
        if (mEntityType == NPC)
            mVelocity.y = mMovement.y * mSpeed;

        mVelocity.x += mAcceleration.x * deltaTime;
        mVelocity.y += mAcceleration.y * deltaTime;

        if (mIsJumping)
        {
            mIsJumping = false;
            mVelocity.y -= mJumpingPower;
        }

        mPosition.y += mVelocity.y * deltaTime;
        checkCollisionY(collidableEntities, collisionCheckCount);
        checkCollisionY(blocks, blockCount);

        mPosition.x += mVelocity.x * deltaTime;
        checkCollisionX(collidableEntities, collisionCheckCount);
        checkCollisionX(blocks, blockCount);
    }

    if (mTextureType == ATLAS)
    {
        bool shouldAnimate = (mAIType == LERPER) || (GetLength(mMovement) != 0);
        if (shouldAnimate) animate(deltaTime);
    }
}

// merge version from map and AI
void Entity::update(float deltaTime, Entity* player, Map* map,
    Entity* collidableEntities, int collisionCheckCount,
    Entity* blocks, int blockCount)
{
    if (mEntityStatus == INACTIVE) return;

    if (mEntityType == NPC) AIActivate(player, deltaTime);

    resetColliderFlags();

    if (mAIType != LERPER)
    {
        mVelocity.x = mMovement.x * mSpeed;
        if (mEntityType == NPC)
            mVelocity.y = mMovement.y * mSpeed;

        mVelocity.x += mAcceleration.x * deltaTime;
        mVelocity.y += mAcceleration.y * deltaTime;

        if (mIsJumping)
        {
            mIsJumping = false;
            mVelocity.y -= mJumpingPower;
        }

        mPosition.x += mVelocity.x * deltaTime;
        checkCollisionX(collidableEntities, collisionCheckCount);
        checkCollisionX(blocks, blockCount);
        checkCollisionX(map);

        mPosition.y += mVelocity.y * deltaTime;
        checkCollisionY(collidableEntities, collisionCheckCount);
        checkCollisionY(blocks, blockCount);
        checkCollisionY(map);
    }

    if (mTextureType == ATLAS)
    {
    /*
        bool shouldAnimate = (mAIType == LERPER) ||
            (GetLength(mMovement) != 0 && mIsCollidingBottom);
    */
        bool isMoving = GetLength(mVelocity) > 0.1f || GetLength(mMovement) > 0.1f;

        bool shouldAnimate = (mAIType == LERPER) || isMoving;
    // since not just lerpers now, wanderer needs to animate
        if (mEntityType == PLAYER) {
            shouldAnimate = (GetLength(mMovement) != 0 && mIsCollidingBottom);
        }
        if (shouldAnimate) animate(deltaTime);
    }
}
void Entity::render()
{
    if(mEntityStatus == INACTIVE) return;

    Rectangle textureArea;

    switch (mTextureType)
    {
        case SINGLE:
            textureArea = {
                0.0f, 0.0f,
                static_cast<float>(mTexture.width),
                static_cast<float>(mTexture.height)
            };
            break;
        case ATLAS:
            textureArea = getUVRectangle(
                &mTexture,
                mAnimationIndices[mCurrentFrameIndex],
                mSpriteSheetDimensions.x,
                mSpriteSheetDimensions.y
            );

        default: break;
    }

    Rectangle destinationArea = {
        mPosition.x,
        mPosition.y,
        static_cast<float>(mScale.x),
        static_cast<float>(mScale.y)
    };

    Vector2 originOffset = {
        static_cast<float>(mScale.x) / 2.0f,
        static_cast<float>(mScale.y) / 2.0f
    };

    DrawTexturePro(
        mTexture,
        textureArea, destinationArea, originOffset,
        mAngle, WHITE
    );

    // displayCollider();
}

void Entity::displayCollider()
{
    Rectangle colliderBox = {
        mPosition.x - mColliderDimensions.x / 2.0f,
        mPosition.y - mColliderDimensions.y / 2.0f,
        mColliderDimensions.x,
        mColliderDimensions.y
    };

    DrawRectangleLines(
        colliderBox.x,
        colliderBox.y,
        colliderBox.width,
        colliderBox.height,
        GREEN
    );
}
