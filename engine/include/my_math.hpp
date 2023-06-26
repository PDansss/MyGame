#include <cmath>
#include <vector>
#pragma once

class my_math
{
public:
    std::vector<float> scaling_matrix(float s1, float s2)
    {
        std::vector<float> result;
        result.resize(9);
        result[0] = s1;
        result[1] = 0.0f;
        result[2] = 0.0f;
        result[3] = 0.0f;
        result[4] = s2;
        result[5] = 0.0f;
        result[6] = 0.0f;
        result[7] = 0.0f;
        result[8] = 1.0f;
        return result;
    }

    std::vector<float> shift_matrix(float s1, float s2)
    {
        std::vector<float> result;
        result.resize(9);
        result[0] = 1.0f;
        result[1] = 0.0f;
        result[2] = s1;
        result[3] = 0.0f;
        result[4] = 1;
        result[5] = s2;
        result[6] = 0.0f;
        result[7] = 0.0f;
        result[8] = 1.0f;
        return result;
    }

    std::vector<float> rotate_matrix(float angile)
    {
        std::vector<float> result;
        result.resize(9);
        result[0] = cos(angile);
        result[1] = -sin(angile);
        result[2] = 0.0f;
        result[3] = sin(angile);
        result[4] = cos(angile);
        result[5] = 0.0f;
        result[6] = 0.0f;
        result[7] = 0.0f;
        result[8] = 1.0f;
        return result;
    }

    std::vector<float> single_matrix()
    {
        std::vector<float> result;
        result.resize(9);
        result[0] = 1.0f;
        result[1] = 0.0f;
        result[2] = 0.0f;
        result[3] = 0.0f;
        result[4] = 1.0f;
        result[5] = 0.0f;
        result[6] = 0.0f;
        result[7] = 0.0f;
        result[8] = 1.0f;
        return result;
    }

    std::vector<float> matrix_multiplying(std::vector<float> matrix1,
                                          std::vector<float> matrix2)
    {
        std::vector<float> result;
        result.resize(9);
        result[0] = matrix1[0] * matrix2[0] + matrix1[1] * matrix2[3] +
                    matrix1[2] * matrix2[6];
        result[1] = matrix1[0] * matrix2[1] + matrix1[1] * matrix2[4] +
                    matrix1[2] * matrix2[7];
        result[2] = matrix1[0] * matrix2[2] + matrix1[1] * matrix2[5] +
                    matrix1[2] * matrix2[8];
        result[3] = matrix1[3] * matrix2[0] + matrix1[4] * matrix2[3] +
                    matrix1[5] * matrix2[6];
        result[4] = matrix1[3] * matrix2[1] + matrix1[4] * matrix2[4] +
                    matrix1[5] * matrix2[7];
        result[5] = matrix1[3] * matrix2[2] + matrix1[4] * matrix2[5] +
                    matrix1[5] * matrix2[8];
        result[6] = matrix1[6] * matrix2[0] + matrix1[7] * matrix2[3] +
                    matrix1[8] * matrix2[6];
        result[7] = matrix1[6] * matrix2[1] + matrix1[7] * matrix2[4] +
                    matrix1[8] * matrix2[7];
        result[8] = matrix1[6] * matrix2[2] + matrix1[7] * matrix2[5] +
                    matrix1[8] * matrix2[8];
        return result;
    }
};
