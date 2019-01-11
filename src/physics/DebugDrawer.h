#ifndef DEBUGDRAWER_H
#define DEBUGDRAWER_H

#include <bullet3/LinearMath/btIDebugDraw.h>

namespace physics
{

class DebugDrawer : public btIDebugDraw
{
public:
	DebugDrawer();
	virtual ~DebugDrawer() override;
	
	virtual void setDefaultColors(const DefaultColors& colors) override;
	virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color) override;
	virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) override;
	virtual void reportErrorWarning(const char* warningString) override;
	virtual void draw3dText(const btVector3& location, const char* textString) override;
	virtual void setDebugMode(int debugMode) override { debugMode_ = debugMode; }
	virtual int getDebugMode() const override { return debugMode_; }
	
	virtual void clearLines() override;
	virtual void flushLines() override;
	
private:
	int debugMode_ = btIDebugDraw::DBG_DrawWireframe 
					| btIDebugDraw::DBG_DrawContactPoints
					| btIDebugDraw::DBG_DrawConstraints
					| btIDebugDraw::DBG_DrawNormals;
	DefaultColors colors_;
};

}

#endif // DEBUGDRAWER_H
