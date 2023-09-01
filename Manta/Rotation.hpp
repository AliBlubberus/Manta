#pragma once

#define _USE_MATH_DEFINES

#include <math.h>;
#include <stdio.h>;
#include <SFML/Graphics.hpp>;

namespace Manta {

	inline float degToRad(float angle) {
		return (angle / 180.0) * ((double)M_PI);
	};

	sf::Vector3f rotateX(const sf::Vector3f* point, float angle) {
		return sf::Vector3f(
			point->x,
			point->y * cos(angle) - point->z * sin(angle),
			point->z * cos(angle) + point->y * sin(angle)
		);
	};

	sf::Vector3f rotateY(const sf::Vector3f* point, float angle) {
		return sf::Vector3f(
			point->x * cos(angle) - point->z * sin(angle),
			point->y,
			point->z * cos(angle) + point->x * sin(angle)
		);
	};

	sf::Vector3f rotateZ(const sf::Vector3f* point, float angle) {
		return sf::Vector3f(
			point->x * cos(angle) - point->y * sin(angle),
			point->y * cos(angle) + point->x * sin(angle),
			point->z
		);
	};
}