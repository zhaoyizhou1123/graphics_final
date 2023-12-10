#pragma once
#include "sparks/geometries/geometry.h"
#include <memory>
#include <vector>
#include "glm/glm.hpp"

namespace sparks {
	struct Light {
		Light(Geometry* geometry, const glm::vec3 & emission, float emission_strength):
			geometry {geometry},
			emission {emission},
			emission_strength {emission_strength} 
		{}
		Light(const Light& light) :
			geometry{ light.geometry },
			emission{ light.emission },
			emission_strength{ light.emission_strength }
		{}
		Light(Light&& light): 
			geometry {light.geometry},
			emission {light.emission},
			emission_strength {light.emission_strength}
		{}

		Geometry* geometry;
		glm::vec3 emission{ 0.0f, 0.0f, 0.0f };
		float emission_strength{ 0.0f };
	};

	class Lights {
	public:
		Lights():
			lights_ {std::vector<Light>()},
			total_area_ {0.0f},
			areas_ {std::vector<float>()}
		{}
		[[nodiscard]] float GetTotalArea() const;
		[[nodiscard]] glm::vec3 Sample(int seed_x, int seed_y, int seed_z) const;
		//void AddLight(const Light& light);
		//void AddLight(Light&& light);
		void AddLight(Geometry* geometry, const glm::vec3 & emission, float emission_strength);
		Light GetLight(int idx) const;
	private:
		std::vector<Light> lights_ ;
		float total_area_ ;
		std::vector<float> areas_; // A list of areas
	};
} // namespace sparks
