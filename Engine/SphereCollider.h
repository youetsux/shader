#pragma once
class SphereCollider
{
	float radius_;
public:
	SphereCollider(float _radius);
	float GetRadius() { return(radius_); }
};

