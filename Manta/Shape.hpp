#pragma once

#include <SFML/Graphics.hpp>;

#include "Transform.hpp";

namespace Manta {

	struct Shape {
		float (*distanceFunction)(sf::Vector3f) {nullptr};

		std::vector<std::shared_ptr<Transform>> pipeline;

		float distanceEstimate(sf::Vector3f point) {
			sf::Vector3f processedPoint = point;

			for (uint8_t i = 0; i < pipeline.size(); i++) {
				processedPoint = pipeline[i]->process(processedPoint);
			}

			return (*distanceFunction)(processedPoint);
		};

		void pushTransform(Transform* p) {
			pipeline.push_back(std::shared_ptr<Transform>(p));
		};

		sf::Color color;
	};


	// Sphere
	float sphereDE(sf::Vector3f point) {
		return sqrt(powf(point.x, 2) +
			powf(point.y, 2) +
			powf(point.z, 2)) - 1;
	}

	Shape* Sphere() {
		Shape* s = new Shape();
		s->distanceFunction = sphereDE;
		return s;
	}

	// Box
	float boxDE(sf::Vector3f point) {
		sf::Vector3f absP(
			(float)abs(point.x) - 1,
			(float)abs(point.y) - 1,
			(float)abs(point.z) - 1
		);

		sf::Vector3f clampedP(
			fmax(absP.x, 0),
			fmax(absP.y, 0),
			fmax(absP.z, 0)
		);

		return
			(sqrt(powf(clampedP.x, 2) +
				powf(clampedP.y, 2) +
				powf(clampedP.z, 2))) + fmin(fmax(absP.x, fmax(absP.y, absP.z)), 0);
	}

	Shape* Box() {
		Shape* s = new Shape();
		s->distanceFunction = boxDE;
		return s;
	}
}