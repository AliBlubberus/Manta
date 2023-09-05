#pragma once

#include <SFML/Graphics.hpp>;

namespace Manta {


	class Light abstract {
	public:
		sf::Color getColor() {
			return this->color;
		}

		float getIntensity() {
			return this->intensity;
		}

	protected:
		sf::Color color;
		float intensity;

		Light() {
			this->color = sf::Color(255, 255, 255);
			this->intensity = 1;
		}
	};

	class GlobalLight : public Light {
	public:
		sf::Vector3f direction;

		GlobalLight() : Light() {
			this->direction = sf::Vector3f(0.01, -1, 0.01);
		}
	};

}