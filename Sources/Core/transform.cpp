#include "Core/transform.h"

glm::mat4 Transform::Model()
{
	glm::mat4 posMatrix = glm::translate(pos);
	glm::mat4 rotMatrix = QuaternionRotate(rot, rotAngle);
	glm::mat4 scaleMatrix = glm::scale(scale);

	return posMatrix * rotMatrix * scaleMatrix;
}

glm::mat4 Transform:: QuaternionRotate(const glm::vec3 &axis, ft angle)
{
	ft w = glm::cos(angle / 2);
	ft v = glm::sin(angle / 2);
	glm::vec3 qv = axis * v;
	glm::quat quaternion(w, qv);
	return glm::mat4_cast(quaternion);
}