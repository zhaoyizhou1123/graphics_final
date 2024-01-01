#pragma once
#include "sparks/geometries/geometry.h"
#include <memory>
#include <vector>
#include "glm/glm.hpp"
#include <random>
#include "sparks/util/util.h"

namespace sparks {
	struct Light {
		Light():
			geometry {nullptr},
			emission {0.0f},
			emission_strength {0.0f}
		{}
		Light(std::unique_ptr<Geometry>&& geometry, const glm::vec3 & emission, float emission_strength):
			geometry {std::move(geometry)},
			emission {emission},
			emission_strength {emission_strength} 
		{}
		//Light(const Light& light) :
		//	geometry{ std::move(light.geometry) },
		//	emission{ light.emission },
		//	emission_strength{ light.emission_strength }
		//{}
		//Light(Light&& light): 
		//	geometry {light.geometry},
		//	emission {light.emission},
		//	emission_strength {light.emission_strength}
		//{}

		std::unique_ptr<Geometry> geometry;
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
		/* @brief Sample a light point
		* @param light_idx: Ptr to index of light
		* @param pos: Ptr to The sampled light point
		* @return pdf
		*/
		float Sample(int* light_idx, glm::vec3* pos, std::mt19937& rng) const;
		//void AddLight(const Light& light);
		//void AddLight(Light&& light);
		void AddLight(std::unique_ptr<Geometry> && geometry, const glm::vec3 & emission, float emission_strength);
		const Light* GetLight(int idx) const;
	private:
		std::vector<Light> lights_ ;
		float total_area_ ;
		std::vector<float> areas_; // A list of areas
	};
} // namespace sparks
