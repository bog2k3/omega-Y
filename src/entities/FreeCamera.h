#ifndef FREECAMERA_H
#define FREECAMERA_H

#include "IUserControllable.h"
#include "enttypes.h"
#include <boglfw/entities/Entity.h>

class FreeCamera : public Entity, public IUserControllable
{
public:
	FreeCamera(glm::vec3 position, glm::vec3 direction);
	virtual ~FreeCamera() override;
	
	virtual FunctionalityFlags getFunctionalityFlags() const override { return FunctionalityFlags::DRAWABLE | FunctionalityFlags::UPDATABLE; }
	virtual unsigned getEntityType() const override { return EntityTypes::FREE_CAMERA; }
	
	virtual void update(float dt) override;
	virtual aabb getAABB(bool requirePrecise=false) const override;
	virtual glm::mat4 getTransform() const override;
	
	virtual void move(direction dir) override;
	virtual void toggleRun(bool on) override { running_ = on; }
	virtual void rotate(direction dir, float angle) override;
	
	virtual void setActionState(int actionId, bool on) override {}

private:
	glm::vec3 position_;
	glm::vec3 direction_;
	mutable glm::mat4 transform_;
	mutable bool transformDirty_ = true;
	
	glm::vec3 speed_ {0.f};
	bool running_ = false;
	glm::vec3 frameMoveValues_ {0.f};
	glm::vec2 frameRotateValues_ {0.f};
};

#endif // FREECAMERA_H
