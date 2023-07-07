#include "engine.hpp"
#include <vector>

#pragma once

namespace morphing{
    const int number_of_vertecies = 61;

	inline void set_figure(IEngine* eng) {
        vector<float> circle_vertexes;
        vector<float> astroid_vertexes;

        const float   pi = 3.1415926535;

        float         radius = 0.1;

        circle_vertexes.reserve(number_of_vertecies * 6);
        astroid_vertexes.reserve(number_of_vertecies * 6);

        circle_vertexes.push_back(0.f);
        circle_vertexes.push_back(0.f);
        circle_vertexes.push_back(1.f);

        circle_vertexes.push_back(0.5f);
        circle_vertexes.push_back(0.5f);
        circle_vertexes.push_back(0.5f);

        for (int i = 1; i < number_of_vertecies; i += 2)
        {
            float theta = (i - 1) * (2 * pi / number_of_vertecies);

            float x = radius * cos(theta);
            float y = radius * sin(theta);

            circle_vertexes.push_back(x);
            circle_vertexes.push_back(y);
            circle_vertexes.push_back(1.f);

            circle_vertexes.push_back(0.5f);
            circle_vertexes.push_back(0.1f);
            circle_vertexes.push_back(0.8f);

            theta = (i + 1) * (2 * pi / number_of_vertecies);
            x = radius * cos(theta);
            y = radius * sin(theta);
            circle_vertexes.push_back(x);
            circle_vertexes.push_back(y);
            circle_vertexes.push_back(1.f);

            circle_vertexes.push_back(1.f);
            circle_vertexes.push_back(1.f);
            circle_vertexes.push_back(1.f);
        }

        radius += 0.15;

        astroid_vertexes.push_back(0.f);
        astroid_vertexes.push_back(0.f);
        astroid_vertexes.push_back(1.f);
        astroid_vertexes.push_back(0.5f);
        astroid_vertexes.push_back(0.5f);
        astroid_vertexes.push_back(0.5f);
        for (int i = 1; i < number_of_vertecies; i += 2)
        {
            float theta = (i - 1) * (2 * pi / number_of_vertecies);
            float x = radius * pow(cos(theta), 3);
            float y = radius * pow(sin(theta), 3);
            astroid_vertexes.push_back(x);
            astroid_vertexes.push_back(y);
            astroid_vertexes.push_back(1.f);

            astroid_vertexes.push_back(1.f);
            astroid_vertexes.push_back(0.f);
            astroid_vertexes.push_back(1.f);

            theta = (i + 1) * (2 * pi / number_of_vertecies);
            x = radius * pow(cos(theta), 3);
            y = radius * pow(sin(theta), 3);
            astroid_vertexes.push_back(x);
            astroid_vertexes.push_back(y);
            astroid_vertexes.push_back(1.f);

            astroid_vertexes.push_back(0.f);
            astroid_vertexes.push_back(1.f);
            astroid_vertexes.push_back(1.f);
        }
        eng->set_buffer(eng->get_vertex_buffer("figure1"), circle_vertexes);
        eng->set_buffer(eng->get_vertex_buffer("figure2"), astroid_vertexes);
	}
    inline void draw(IEngine* eng, vector<float> normolize_matrix, float x, float y) {
        my_math math;
        eng->render_triangle("figure1", "figure2", number_of_vertecies, normolize_matrix, math.shift_matrix(x, y));
	}
};
