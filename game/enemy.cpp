#include "engine.hpp"
#include "tank.hpp"

class enemy : public ITank
{
public:
    enemy(IEngine*      eng,
          ITank*        play,
          float         x,
          float         y,
          float         base_ang,
          float         tur_ang,
          vector<float> norm_matrix,
          float         coeff,
          int*          score_)

    {
        engine             = eng;
        player             = play;
        tank_x             = x;
		start_x            = x;
        tank_y             = y;
		start_y            = y;
        normolize_matrix   = norm_matrix;
        start_base_angle   = base_ang;
		base_angle         = base_ang;
        start_turret_angle = tur_ang;
        turret_angle       = tur_ang;
        scale_coefficient  = coeff;
        score              = score_;
    }

    void render()
    {
        // Draw platform

        engine->render_triangle(
            "tank",
            normolize_matrix,
            matrices.shift_matrix(tank_x, tank_y),
            matrices.rotate_matrix(base_angle),
            enemy_color,
            engine->get_texture("tank_part_1"),
            false);

        engine->render_triangle(
            "tank",
            normolize_matrix,
            matrices.shift_matrix(tank_x, tank_y),
            matrices.rotate_matrix(turret_angle),
            enemy_color,
            engine->get_texture("tank_part_2"),
            true);

        // Draw missile

        missiles.draw_missile(engine, normolize_matrix);
        explosion_animation.draw_animation(engine, normolize_matrix);
    }

    void update(vector<obstacle>& objects)
    {
        unsigned int time = engine->get_time();
        if (!isDead)
        {
            aim(player);

            if (time - previos_intrepolation_base_time > (unsigned int)(rand() % 5000 + 4500))
            {
                vector<float> hitbox;
                int           i = 0;
                do
                {
                    int random         = rand() % 3;
                    desired_base_angle = angiles[random];

                    float dX = cos(desired_base_angle) * tank_speed;
                    float dY = sin(desired_base_angle) * tank_speed;

                    hitbox = get_hitbox(dX * normolize_matrix[0],
                                        dY * normolize_matrix[4],
                                        desired_base_angle);
                    i++;
                } while (!check_collision_with_walls(hitbox, objects) && i < 4);

                previos_intrepolation_base_time = time;
                base_angle += desired_base_angle;
            }

            dx = cos(base_angle) * tank_speed;
            dy = sin(base_angle) * tank_speed;

            vector<float> hitbox = get_hitbox(
                dx * normolize_matrix[0], dy * normolize_matrix[4], base_angle);

            bool tank_wall_collision =
                check_collision_with_walls(hitbox, objects);

            if (tank_wall_collision)
            {
                shoot();
                if (dx > dy)
                {
                    if (dx > 0)
                    {
                        base_angle   = base_angle + 3.14f;
                        turret_angle = turret_angle + 3.14f;
                    }
                    else
                    {
                        base_angle   = base_angle - 3.14f;
                        turret_angle = turret_angle - 3.14f;
                    }
                }
                else
                {
                    if (dy < 0)
                    {
                        base_angle   = base_angle - 3.14f;
                        turret_angle = turret_angle - 3.14f;
                    }
                    else
                    {
                        base_angle   = base_angle + 3.14f;
                        turret_angle = turret_angle + 3.14f;
                    }
                }
                desired_turret_angle = turret_angle;

                dx = cos(base_angle) * tank_speed;
                dy = sin(base_angle) * tank_speed;
                move(dx, dy);
                previos_time = time;
            }

            bool tank_missile_collision = missiles.missile_collision_with_enemy(
                player->get_missile_stack(),
                tank_x,
                tank_y,
                scale_coefficient,
                normolize_matrix);

            if (tank_missile_collision)
            {
                isDead = true;

                *score += 1;

                set_color(enemy_color, 0.45f, 0.45f, 0.45f);

                explosion_animation.add_animation(tank_x * normolize_matrix[0],
                                                  tank_y * normolize_matrix[4],
                                                  1.8f);
                previos_time = time;
            }

            if (desired_turret_angle != start_turret_angle)
            {
                if (time - previos_intrepolation_time > 60)
                {
                    if (t < 1)
                        t += 0.1f;

                    turret_angle =
                        (start_turret_angle +
                         t * (desired_turret_angle - start_turret_angle));

                    previos_intrepolation_time = time;

                    if (t > 1)
                    {
                        start_turret_angle   = turret_angle;
                        desired_turret_angle = turret_angle;
                        t                    = 0;
                        shoot();
                    }
                }
            }
            move(dx, dy);
        }
        else
        {
            if (time - previos_time > 20000)
            {
                previos_time = time;
                isDead       = false;
                set_color(enemy_color, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                float obstacle_color = 0.45f + ((time - previos_time) / 20000.f) * (0.55f);
                set_color(enemy_color, obstacle_color, obstacle_color, obstacle_color);
            }
        }
        missiles.update(engine,
                        objects,
                        normolize_matrix,
                        scale_coefficient,
                        explosion_animation);
    }

    void shoot()
    {
        float exposion_x = normolize_matrix[0] * cos(turret_angle) * 0.25f +
                           tank_x * normolize_matrix[0];
        float exposion_y = normolize_matrix[4] * sin(turret_angle) * 0.25f +
                           tank_y * normolize_matrix[4];
        explosion_animation.add_animation(exposion_x, exposion_y, 1);
        missiles.add_missile_to_stack(tank_x, tank_y, turret_angle);
    }

    void aim(ITank* player)
    {

        float enemy_x = tank_x * normolize_matrix[0];
        float enemy_y = tank_y * normolize_matrix[0];

        float player_x = player->get_x() * normolize_matrix[0];
        float player_y = player->get_y() * normolize_matrix[0];

        float dx = enemy_x - player_x;
        float dy = enemy_y - player_y;

        float distance = sqrt(dx * dx + dy * dy);

        if (distance < field_of_view * normolize_matrix[0] * scale_coefficient)
        {
            float angle = atan2(player_y - enemy_y, player_x - enemy_x);
            desired_turret_angle = angle;
        }
        else
        {
            desired_turret_angle = base_angle;
        }
    }

    void move(float dx, float dy)
    {
        tank_x += dx;
        tank_y += dy;
    }
    void restart()
    {
        tank_x     = start_x;
        tank_y     = start_y;
        base_angle = start_base_angle;
		turret_angle = start_turret_angle;
        set_color(enemy_color, 1.0f, 1.0f, 1.0f);
        isDead = false;
    }

    bool IsDead() { return isDead; }

    float get_x() { return tank_x; }

    float get_y() { return tank_y; }

    stack<missile>* get_missile_stack() { return missiles.get_stack(); }

    void set_stack(vector<stack<missile>*> pos) {}

private:
    IEngine* engine = nullptr;
    ITank*   player = nullptr;

    int* score;

    vector<float> normolize_matrix;
    float         scale_coefficient;

    my_math matrices;

    animation explosion_animation;
    Missile   missiles;

    float tank_x;
    float tank_y;
    float start_x;
    float start_y;
    float base_angle = 0;
    float turret_angle;
    bool  isDead = false;

    float tank_speed = 0.005f;

    float        field_of_view = 0.8f;
    float        dx = 0.f, dy = 0.f, param = 1.f;
    unsigned int previos_time = 0;
    unsigned int temp;

    unsigned int previos_intrepolation_time = 0;
    float        desired_turret_angle       = 0.f;
    float        start_turret_angle;
    float        t = 0.f;

    unsigned int  previos_intrepolation_base_time = 0;
    float         desired_base_angle              = 0.f;
    float         start_base_angle                = 0.f;
    float         t2                              = 0.f;
    vector<float> angiles{ 0.f, 1.57f, 3.14f, 4.71f };

    float temp_ang;

    vector<float> enemy_color = { 1.0f,1.0f,1.0f,1.0f };

    void rotate(float& x, float& y, float cx, float cy, float angle)
    {
        float _x = x - cx;
        float _y = y - cy;

        x = _x * cos(angle) - _y * sin(angle);
        y = _x * sin(angle) + _y * cos(angle);
        x += cx;
        y += cy;
    }
    void set_color(vector<float>& vec, float r, float g, float b)
    {
        vec[0]  = r;
        vec[1]  = g;
        vec[2]  = b;
    }

    vector<float> get_hitbox(float d_x, float d_y, float d_angle)
    {
        float left_down_x = (tank_x + d_x - 0.2f) * normolize_matrix[0];
        float left_down_y = (tank_y + d_y - 0.1f) * normolize_matrix[4];

        rotate(left_down_x,
               left_down_y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float left_up_x = (tank_x + d_x - 0.2f) * normolize_matrix[0];
        float left_up_y = (tank_y + d_y + 0.1f) * normolize_matrix[4];

        rotate(left_up_x,
               left_up_y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float right_down_x = (tank_x + d_x + 0.14f) * normolize_matrix[0];
        float right_down_y = (tank_y + d_y - 0.1f) * normolize_matrix[4];

        rotate(right_down_x,
               right_down_y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float right_up_x = (tank_x + d_x + 0.14f) * normolize_matrix[0];
        float right_up_y = (tank_y + d_y + 0.1f) * normolize_matrix[4];
        rotate(right_up_x,
               right_up_y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float x = (tank_x + d_x + 0.14f) * normolize_matrix[0];
        float y = (tank_y + d_y) * normolize_matrix[4];
        rotate(x,
               y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float _x = (tank_x + d_x - 0.2f) * normolize_matrix[0];
        float _y = (tank_y + d_y) * normolize_matrix[4];
        rotate(_x,
               _y,
               (tank_x + d_x) * normolize_matrix[0],
               (tank_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        vector<float> hitbox = { left_down_x, left_down_y,  left_up_x,
                                 left_up_y,   right_down_x, right_down_y,
                                 right_up_x,  right_up_y,   x,
                                 y,           _x,           _y };
        return hitbox;
    }

    bool check_collision_with_walls(vector<float>    hitbox,
                                    vector<obstacle> objects)
    {

        for (int i = 0; i < objects.size();
             i++) // Обработка коллизий танка со стенами
        {
            if (objects[i].exist)
            {
                float obj_x =
                    objects[i].x * normolize_matrix[0] * scale_coefficient;
                float obj_y =
                    objects[i].y * normolize_matrix[4] * scale_coefficient;

                float obj_left_down_x = (obj_x - 0.1f * normolize_matrix[0]);
                float obj_left_down_y = (obj_y - 0.1f * normolize_matrix[4]);

                float obj_right_down_x = (obj_x + 0.1f * normolize_matrix[0]);

                float obj_right_up_y = (obj_y + 0.1f * normolize_matrix[4]);

                for (int j = 0; j < hitbox.size(); j += 2)
                {
                    if (hitbox[j] >= obj_left_down_x &&
                        hitbox[j] <= obj_right_down_x &&
                        hitbox[j + 1] >= obj_left_down_y &&
                        hitbox[j + 1] <= obj_right_up_y)
                    {
                        return true;
                    }
                }
            }
        }
        return false;
    }
};

ITank* create_enemy_tank(IEngine*      eng,
                         ITank*        tank,
                         float         x,
                         float         y,
                         float         ang_base,
                         float         turret_ang,
                         vector<float> norm,
                         float         coeff,
                         int*          score_)
{
    return new enemy(
        eng, tank, x, y, ang_base, turret_ang, norm, coeff, score_);
}
