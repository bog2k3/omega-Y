#ifndef COLLISIONCHECKER_H
#define COLLISIONCHECKER_H

// this checks all contacts generated by the physics engine and then triggers user collision events
// configured via PhysBodyMeta
class CollisionChecker {
public:
	CollisionChecker() = delete;

	static void update(float dt);
};

#endif // COLLISIONCHECKER_H