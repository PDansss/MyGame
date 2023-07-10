#include "engine.hpp"
#include "morphing.hpp"
#include "missile.hpp"
#include "my_math.hpp"

#pragma once
class ITank
{
public:
    virtual void render() = 0;

    virtual void update(vector<obstacle>& objects) = 0;

    virtual void restart() = 0;

    virtual bool IsDead() = 0;

    virtual float get_x() = 0;

    virtual float get_y() = 0;

    virtual stack<missile>* get_missile_stack() = 0;

    virtual void set_stack(vector<stack<missile>*>) = 0;

    virtual ~ITank() = default;
};

ITank* create_tank(IEngine*      eng,
                   float         x,
                   float         y,
                   float         angle,
                   vector<float> norm,
                   float         coeff);

ITank* create_enemy_tank(IEngine*      eng,
                         ITank*        tank,
                         float         x,
                         float         y,
                         float         angle,
                         vector<float> norm,
                         float         coeff,
                         int*          score_);

void delete_tank(ITank* tank);
