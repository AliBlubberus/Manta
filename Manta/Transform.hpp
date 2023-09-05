#pragma once

#include <SFML/Graphics.hpp>;
#include "Rotation.hpp";

namespace Manta {

	class Transform {
	public:
		virtual sf::Vector3f process(sf::Vector3f point) = 0;
	};


	class Translate : public Transform {
	public:
		sf::Vector3f deltaPosition;

		sf::Vector3f process(sf::Vector3f point) override {
			return point + deltaPosition;
		};

		Translate(sf::Vector3f deltaPosition) {
			this->deltaPosition = deltaPosition;
		};
	};

	class Rotate : public Transform {
	public:
		sf::Vector3f eulerAngles;

		sf::Vector3f process(const sf::Vector3f point) override {
			sf::Vector3f p = rotateX(&point, this->eulerAngles.x);
			p = rotateZ(&p, this->eulerAngles.z);
			p = rotateY(&p, this->eulerAngles.y);
			return p;
		};
	};

	class Scale : public Transform {
	public:
		sf::Vector3f factor = sf::Vector3f(1, 1, 1);

		sf::Vector3f process(const sf::Vector3f point) override {
			return sf::Vector3f(
				point.x / this->factor.x,
				point.y / this->factor.y,
				point.z / this->factor.z
			);
		}
	};

};