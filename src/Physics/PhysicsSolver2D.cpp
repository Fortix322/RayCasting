#include "PhysicsSolver2D.h"

#include <algorithm>
#include <iostream>
#include <chrono>

#include "glm/glm/geometric.hpp"

static glm::vec2 vmin(const glm::vec2& lvec, const glm::vec2& rvec)
{
	return glm::vec2(fmin(lvec.x, rvec.x), fmin(lvec.y, rvec.y));
}

static glm::vec2 vmax(const glm::vec2& lvec, const glm::vec2& rvec)
{
	return glm::vec2(fmax(lvec.x, rvec.x), fmax(lvec.y, rvec.y));
}

static glm::vec2 clamp(const glm::vec2& vec, const glm::vec2& min, const glm::vec2& max)
{
	return vmin(vmax(vec, min), max);
}

void PhysicsSolver2D::Solve()
{
	float frequency = 60.0f;
	float deltaTime = 1.0f / frequency;

	auto t = std::chrono::high_resolution_clock::now();

	CalculatePositions(deltaTime);

	std::cout << "Calculate positions (SINGLE): " << std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - t).count() << std::endl;

	SatisfyConstraints();
}

void PhysicsSolver2D::CalculatePositions(float deltaTime)
{
	glm::vec2 transform;
	for (size_t i = 0; i < m_Particles.size(); i++)
	{
		if (!m_Particles[i].IsMovable()) continue;

		transform = (2.0f * m_Particles[i].GetPosition() - m_OldPos[i] + m_Forces[i] * deltaTime * deltaTime) - m_Particles[i].GetPosition();
		m_OldPos[i] = m_Particles[i].GetPosition();
		m_Particles[i].Move(transform);
	}
}

void PhysicsSolver2D::SatisfyConstraints()
{
	static const size_t s_Iter = 10;
	
	for (size_t i = 0; i < s_Iter; i++)
	{
		for(Particle &p : m_Particles)
		{
			if (!p.IsMovable()) continue;
			glm::vec2 transform = clamp(p.GetPosition(), { -1.0f, -1.0f }, { 1.0f, 1.0f }) - p.GetPosition();
			p.Move(transform);
		}

		for (StickConstraint& s : m_Sticks)
		{
			if (s.IsBroken()) continue;
			s.Satisfy();
		}
	}

}

const std::vector<Particle>& PhysicsSolver2D::GetParticles() const
{
	return m_Particles;
}

const std::vector<StickConstraint>& PhysicsSolver2D::GetSticks() const
{
	return m_Sticks;
}

void PhysicsSolver2D::AddParticles(Particle* data, uint64_t count)
{
	for (uint64_t i = 0; i < count; i++)
	{
		m_Particles.push_back(data[i]);
		m_OldPos.push_back(data[i].GetPosition());
		m_Forces.push_back({ 0.0f, -10.0f});
	}
}

void PhysicsSolver2D::CreateStick(uint64_t lInd, uint64_t rInd, float elogationRatio)
{
	if (lInd > m_Particles.size() || rInd > m_Particles.size()) return;

	m_Sticks.push_back(StickConstraint(&m_Particles[lInd], &m_Particles[rInd], 
		glm::length(m_Particles[rInd].GetPosition() - m_Particles[lInd].GetPosition()), elogationRatio));
}