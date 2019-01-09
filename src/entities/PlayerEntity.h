#ifndef PLAYERENTITY_H
#define PLAYERENTITY_H

#include "enttypes.h"
#include <boglfw/entities/Entity.h>
#include <boglfw/renderOpenGL/Mesh.h>

class PlayerEntity : public Entity
{
public:
	PlayerEntity(glm::vec3 position, glm::vec3 direction);
	virtual ~PlayerEntity() override;
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }
	virtual unsigned getEntityType() const override { return EntityTypes::PLAYER; }
	
	virtual void update(float dt) override;
	virtual void draw(Viewport* vp) override;
	virtual aabb getAABB(bool requirePrecise=false) const override;
	virtual glm::mat4 getTransform() const override;
	
	enum direction {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};
	void move(direction dir, bool run);
	// speedFactor is clamped to [0.0 .. 1.0] 
	void rotate(direction dir, float speedFactor);
	
private:
	Mesh mesh_;
	glm::vec3 position_;
	glm::vec3 direction_;
	mutable glm::mat4 transform_;
	mutable bool transformDirty_ = true;
	
	glm::vec3 speed_;
};

#endif // PLAYERENTITY_H
