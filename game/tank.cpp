#include "tank.hpp"
#include "engine.hpp"

vector<float> triangle_1_base = { -0.2f, 0.1f,  0.0f,  0.6f,  0.75f, 0.35f,
                                  0.0f,  1.0f,  0.2f,  0.1f,  0.0f,  0.6f,
                                  0.75f, 0.35f, 0.0f,  0.0f,  0.2f,  -0.1f,
                                  0.0f,  0.6f,  0.75f, 0.35f, 1.0f,  0.0f };

vector<float> triangle_2_base = { -0.2f, 0.1f,  0.0f,  0.6f,  0.75f, 0.35f,
                                  0.0f,  1.0f,  0.2f,  -0.1f, 0.0f,  0.6f,
                                  0.75f, 0.35f, 1.0f,  0.0f,  -0.2f, -0.1f,
                                  0.0f,  0.6f,  0.75f, 0.35f, 1.0f,  1.0f };

vector<float> triangle_1_turret = { -0.2f, 0.1f,  0.0f,  0.6f,  0.75f, 0.35f,
                                    0.0f,  1.0f,  0.2f,  0.1f,  0.0f,  0.6f,
                                    0.75f, 0.35f, 0.0f,  0.0f,  0.2f,  -0.1f,
                                    0.0f,  0.6f,  0.75f, 0.35f, 1.0f,  0.0f };

vector<float> triangle_2_turret = { -0.2f, 0.1f,  0.0f,  0.6f,  0.75f, 0.35f,
                                    0.0f,  1.0f,  0.2f,  -0.1f, 0.0f,  0.6f,
                                    0.75f, 0.35f, 1.0f,  0.0f,  -0.2f, -0.1f,
                                    0.0f,  0.6f,  0.75f, 0.35f, 1.0f,  1.0f };

class tank : public ITank
{
public:
    tank(IEngine*      eng,
         float         x,
         float         y,
         float         base_ang,
         float         tur_ang,
         vector<float> norm_matrix,
         float         coeff)
    {
        normolize_matrix = norm_matrix;

        engine              = eng;
        tank_center_x       = x;
        tank_center_y       = y;
        start_x             = x;
        start_y             = y;
        base_angle          = base_ang;
        start_base_angle    = base_ang;
        turret_angle        = tur_ang;
        start_turret_angle  = tur_ang;
        scaling_coefficient = coeff;
        base_texture        = eng->get_texture(7);
        turret_texture      = eng->get_texture(8);
        missile_texture     = eng->get_texture(9);
    }

    void render()
    {
        // Draw platform
        engine->render_triangle(
            triangle_1_base,
            normolize_matrix,
            matrices.shift_matrix(tank_center_x, tank_center_y),
            matrices.rotate_matrix(base_angle),
            base_texture,
            false);
        engine->render_triangle(
            triangle_2_base,
            normolize_matrix,
            matrices.shift_matrix(tank_center_x, tank_center_y),
            matrices.rotate_matrix(base_angle),
            base_texture,
            false);

        // Draw turret
        engine->render_triangle(
            triangle_1_turret,
            normolize_matrix,
            matrices.shift_matrix(tank_center_x, tank_center_y),
            matrices.rotate_matrix(turret_angle),
            turret_texture,
            true);
        engine->render_triangle(
            triangle_2_turret,
            normolize_matrix,
            matrices.shift_matrix(tank_center_x, tank_center_y),
            matrices.rotate_matrix(turret_angle),
            turret_texture,
            true);

        // Draw missile

        missiles.draw_missile(engine, normolize_matrix);
        explosion_animation.draw_animation(engine, normolize_matrix);
    }

    void update(vector<obstacle>& objects)
    {

        float d_x = 0.f, d_y = 0.f, d_angle = 0.f;

        if (!isDead)
        {
            if (engine->check_button("W"))
            {
                d_x = cos(base_angle) * tank_speed;
                d_y = sin(base_angle) * tank_speed;
            }
            if (engine->check_button("S"))
            {
                d_x = -cos(base_angle) * tank_speed;
                d_y = -sin(base_angle) * tank_speed;
            }
            if (engine->check_button("A"))
            {
                d_angle = base_angle_speed;
            }
            if (engine->check_button("D"))
            {
                d_angle = -base_angle_speed;
            }

            turret_angle +=
                turret_angle_speed * engine->check_button("MOUSE_WHEEL");

            if (engine->check_button("MOUSE_CLICK")) // SPACE
            {
                float exposion_x =
                    normolize_matrix[0] * cos(turret_angle) * 0.25f +
                    tank_center_x * normolize_matrix[0];
                float exposion_y =
                    normolize_matrix[4] * sin(turret_angle) * 0.25f +
                    tank_center_y * normolize_matrix[4];
                engine->play_audio(2);

                explosion_animation.add_animation(exposion_x, exposion_y, 1);
                missiles.add_missile_to_stack(
                    tank_center_x, tank_center_y, turret_angle);
            }

            vector<float> hitbox = get_hitbox(d_x, d_y, d_angle);

            bool tank_wall_collision =
                check_collision_with_walls(hitbox, objects);

            bool tank_missile_collision =
                missiles.missile_collision_with_player(positions,
                                                       tank_center_x,
                                                       tank_center_y,
                                                       scaling_coefficient,
                                                       normolize_matrix);

            if (tank_missile_collision)
            {
                isDead = true;
                explosion_animation.add_animation(
                    tank_center_x * normolize_matrix[0],
                    tank_center_y * normolize_matrix[4],
                    2);
            }

            if (!tank_wall_collision)
            {
                tank_center_x += d_x;
                tank_center_y += d_y;
                base_angle += d_angle;
            }
            else
            {
                tank_center_x -= d_x;
                tank_center_y -= d_y;
                base_angle -= d_angle;
            }
            missiles.update(engine,
                            objects,
                            normolize_matrix,
                            scaling_coefficient,
                            explosion_animation);
        }

        // check_missile_collision_with_walls(missiles, objects);
    }
    vector<float> get_hitbox(float d_x, float d_y, float d_angle)
    {
        float left_down_x = (tank_center_x + d_x - 0.2f) * normolize_matrix[0];
        float left_down_y = (tank_center_y + d_y - 0.1f) * normolize_matrix[4];

        rotate(left_down_x,
               left_down_y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float left_up_x = (tank_center_x + d_x - 0.2f) * normolize_matrix[0];
        float left_up_y = (tank_center_y + d_y + 0.1f) * normolize_matrix[4];

        rotate(left_up_x,
               left_up_y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float right_down_x = (tank_center_x + d_x + 0.14f) * normolize_matrix[0];
        float right_down_y = (tank_center_y + d_y - 0.1f) * normolize_matrix[4];

        rotate(right_down_x,
               right_down_y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float right_up_x = (tank_center_x + d_x + 0.14f) * normolize_matrix[0];
        float right_up_y = (tank_center_y + d_y + 0.1f) * normolize_matrix[4];
        rotate(right_up_x,
               right_up_y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float x = (tank_center_x + d_x + 0.14f) * normolize_matrix[0];
        float y = (tank_center_y + d_y) * normolize_matrix[4];
        rotate(x,
               y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
               base_angle + d_angle);

        float _x = (tank_center_x + d_x - 0.2f) * normolize_matrix[0];
        float _y = (tank_center_y + d_y) * normolize_matrix[4];
        rotate(_x,
               _y,
               (tank_center_x + d_x) * normolize_matrix[0],
               (tank_center_y + d_y) * normolize_matrix[4],
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
                    objects[i].x * normolize_matrix[0] * scaling_coefficient;
                float obj_y =
                    objects[i].y * normolize_matrix[4] * scaling_coefficient;

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

    void check_missile_collision_with_walls(stack<missile>&   miss,
                                            vector<obstacle>& objects)
    {
        missiles.update(engine,
                        objects,
                        normolize_matrix,
                        scaling_coefficient,
                        explosion_animation);
    }

    void shoot()
    {
        float exposion_x = normolize_matrix[0] * cos(turret_angle) * 0.25f +
                           tank_center_x * normolize_matrix[0];
        float exposion_y = normolize_matrix[4] * sin(turret_angle) * 0.25f +
                           tank_center_y * normolize_matrix[4];
        explosion_animation.add_animation(exposion_x, exposion_y, 1);
        missiles.add_missile_to_stack(
            tank_center_x, tank_center_y, turret_angle);
    }

    // stack<missile>* get_missile_stack() { return &missiles; }

    void restart()
    {
        tank_center_x = start_x;
        tank_center_y = start_y;
        base_angle    = start_base_angle;
        turret_angle  = start_turret_angle;
        isDead        = false;
    }

    bool IsDead() { return isDead; }

    float get_x() { return tank_center_x; }

    float get_y() { return tank_center_y; }

    stack<missile>* get_missile_stack() { return missiles.get_stack(); }

    void set_stack(vector<stack<missile>*> pos) { positions = pos; }

private:
    IEngine*      engine = nullptr;
    my_math       matrices;
    vector<float> normolize_matrix;
    float         scaling_coefficient;
    unsigned int  base_texture, turret_texture, missile_texture;

    animation explosion_animation;
    Missile   missiles;

    vector<stack<missile>*> positions;

    // tank initial params
    float tank_speed         = 0.005f;
    float base_angle_speed   = 0.03f;
    float turret_angle_speed = 0.1f;

    // tank initial params
    float start_x, start_y, start_base_angle, start_turret_angle;
    float tank_center_x;
    float tank_center_y;
    float base_angle;
    float turret_angle;
    bool  isDead = false;

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
        vec[3]  = r;
        vec[4]  = g;
        vec[5]  = b;
        vec[11] = r;
        vec[12] = g;
        vec[13] = b;
        vec[19] = r;
        vec[20] = g;
        vec[21] = b;
    }
};

ITank* create_tank(IEngine*      eng,
                   float         x,
                   float         y,
                   float         ang_base,
                   float         turret_ang,
                   vector<float> norm,
                   float         coeff)
{
    return new tank(eng, x, y, ang_base, turret_ang, norm, coeff);
}
void delete_tank(ITank* tank)
{
    delete tank;
}
