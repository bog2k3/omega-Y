#include "DebugDrawer.h"
#include "math.h"

#include <boglfw/RenderOpenGL/Shape3D.h>
#include <boglfw/utils/log.h>

namespace physics {

DebugDrawer::DebugDrawer()
{
}

DebugDrawer::~DebugDrawer()
{
}

void DebugDrawer::setDefaultColors(const DefaultColors& colors) {
	colors_ = colors;
}

void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color) {
	Shape3D::get()->drawLine(b2g(from), b2g(to), b2g(color));
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color) {
	// TODO: use lifetime for alpha?
	Shape3D::get()->drawLine(b2g(PointOnB), b2g(PointOnB+normalOnB * distance), glm::vec4(b2g(color), 1.f));
}

void DebugDrawer::reportErrorWarning(const char* warningString) {
	LOGLN("BULLET WARNING: " << warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString) {
}

void DebugDrawer::clearLines() {
}

void DebugDrawer::flushLines() {
}


} // namespace physics