#include "engine.hpp"
#include "tank.hpp"

class enemy : public ITank
{
public:
    enemy(IEngine*      eng,
          ITank*        play,
          float         x,
          float         y,
          float         angile,
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

        start_base_angle   = angile;
		base_angle         = angile;
        start_turret_angle = angile;
        turret_angle       = angile;

        scale_coefficient  = coeff;
        score              = score_;
    }

    void render()
    {
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

        engine->render_line(attr, matrices.single_matrix(), matrices.single_matrix(), matrices.single_matrix(), ind);

        if(draw_figure)
            morphing::draw(engine, normolize_matrix, tank_x, tank_y);


        missiles.draw_missile(engine, normolize_matrix);
        explosion_animation.draw_animation(engine, normolize_matrix);
    }

    void update(vector<obstacle>& objects)
    {
        unsigned int time = engine->get_time();
        if (!isDead)
        {
            get_player_angle(player,desired_turret_angle);
           
            vector<float> hitbox;

            int i = 0;

            if (time - previos_intrepolation_base_time > (unsigned int)(rand() % 5000 + 2500) && !have_obstacle)
            {
                int random         = rand() % 7;
                base_angle = angiles[random];

                float dX = cos(base_angle) * tank_speed;
                float dY = sin(base_angle) * tank_speed;

                hitbox = get_hitbox(dX, dY, base_angle);
                i++;
                turret_angle = base_angle;
                previos_intrepolation_base_time = time;
                param = 1;
            }

            have_obstacle = false;

            dx = cos(base_angle) * tank_speed * param;
            dy = sin(base_angle) * tank_speed * param;

            hitbox = get_hitbox(
                dx * normolize_matrix[0], dy * normolize_matrix[4], base_angle);

            bool tank_wall_collision =
                check_collision_with_walls(hitbox, objects);

            if (tank_wall_collision)
            {
                shoot();
                param = -1;
                turret_angle = base_angle + 3.14;
                desired_turret_angle = turret_angle;
                dx = cos(base_angle) * tank_speed * param;
                dy = sin(base_angle) * tank_speed * param;
                move(dx, dy);
                previos_time = time;
            }
            else {
                move(dx, dy);
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
                draw_figure = true;

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
                        start_turret_angle = turret_angle;
                        desired_turret_angle = turret_angle;
                        t = 0;
                        shoot();
                    }
                }
            }
        }
        else
        {
            if (time - previos_time > 20000)
            {
                previos_time = time;
                isDead       = false;
                draw_figure = false;
                set_color(enemy_color, 1.0f, 1.0f, 1.0f);
            }
            else
            {
                float obstacle_color = 0.45f + ((time - previos_time) / 20000.f) * (0.55f);
                set_color(enemy_color, obstacle_color, obstacle_color, obstacle_color);

                if (draw_figure) {
                    float player_x = player->get_x() * normolize_matrix[0];
                    float player_y = player->get_y() * normolize_matrix[4];
                    float dX = tank_x * normolize_matrix[0] - player_x;
                    float dY = tank_y * normolize_matrix[4] - player_y;
                    float distance = sqrt(dX * dX + dY * dY);

                    if (distance <= 0.15f * (1 / scale_coefficient))
                    {
                        *score += 10;
                        draw_figure = false;
                    }
                }
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

    void get_player_angle(ITank* player, float& angle)
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
            angle = atan2(player_y - enemy_y, player_x - enemy_x);
        }
        else {
            angle = base_angle;
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

    vector<float> attr;
    vector<unsigned int> ind = { 0,1,3,2,0 };

    float tank_x;
    float tank_y;
    float start_x;
    float start_y;
    float base_angle = 0;
    float turret_angle;
    bool  isDead = false;
    bool draw_figure = false;

    bool have_obstacle = false;

    float base_angle_speed = 0.03f;

    float tank_speed = 0.005f;

    float        field_of_view = 0.8f;
    float        dx = 0.f, dy = 0.f, param = 1.f;
    unsigned int previos_time = 0;
    unsigned int temp;

    unsigned int previos_intrepolation_time = 0;
    float        desired_turret_angle       = 0.f;
    float        start_turret_angle = 0.f;
    float        t = 0.f;

    unsigned int  previos_intrepolation_base_time = 0;
    float         start_base_angle                = 0.f;

    vector<float> angiles = { 0.f,0.79f, 1.57f, 2.36f, 3.14f, 3.93f, 4.71f,5.5f };
  
    float temp_ang;

    vector<float> enemy_color = { 1.0f,1.0f,1.0f,1.0f };

    void rotate(float& x, float& y, float cx, float cy, float angle)
    {
        float _x = x - cx;
        float _y = y - cy;

        x = (_x * cos(angle) - _y * sin(angle)) * normolize_matrix[0] * scale_coefficient;
        y = (_x * sin(angle) + _y * cos(angle)) * normolize_matrix[4] * scale_coefficient;
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
        float left_down_x = ((tank_x + d_x)*normolize_matrix[0]) - 0.2f * (1 / scale_coefficient);
        float left_down_y = ((tank_y + d_y)*normolize_matrix[4]) - 0.1f * (1 / scale_coefficient);

        rotate(left_down_x,
            left_down_y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        float left_up_x = ((tank_x + d_x)*normolize_matrix[0]) - 0.2f * (1 / scale_coefficient);
        float left_up_y = ((tank_y + d_y)*normolize_matrix[4]) + 0.1f * (1 / scale_coefficient);

        rotate(left_up_x,
            left_up_y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        float right_down_x = ((tank_x + d_x) * normolize_matrix[0]) + 0.1f * (1 / scale_coefficient);
        float right_down_y = ((tank_y + d_y) * normolize_matrix[4]) - 0.1f * (1 / scale_coefficient);

        rotate(right_down_x,
            right_down_y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        float right_up_x = ((tank_x + d_x) * normolize_matrix[0]) + 0.1f * (1 / scale_coefficient);
        float right_up_y = ((tank_y + d_y) * normolize_matrix[4]) + 0.1f * (1 / scale_coefficient);

        rotate(right_up_x,
            right_up_y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        float x = ((tank_x + d_x) * normolize_matrix[0]) + 0.1f * (1 / scale_coefficient);
        float y = ((tank_y + d_y) * normolize_matrix[4]);

        rotate(x,
            y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        float _x = ((tank_x + d_x) * normolize_matrix[0]) - 0.2f * (1 / scale_coefficient);
        float _y = ((tank_y + d_y) * normolize_matrix[4]);

        rotate(_x,
            _y,
            (tank_x + d_x) * normolize_matrix[0],
            (tank_y + d_y) * normolize_matrix[4],
            d_angle);

        vector<float> hitbox = { left_down_x, left_down_y,  left_up_x,
                                 left_up_y,   right_down_x, right_down_y,
                                 right_up_x,  right_up_y,   x,
                                 y,           _x,           _y };

        attr = { left_down_x , left_down_y, 1,1,0,1,
                 left_up_x   , left_up_y,   1,1,0,1,
                 right_down_x, right_down_y,1,1,0,1,
                 right_up_x  , right_up_y,  1,1,0,1 };

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
                    objects[i].x * normolize_matrix[0];
                float obj_y =
                    objects[i].y * normolize_matrix[4];

                float obj_left_down_x = (obj_x - 0.1f * normolize_matrix[0]);
                float obj_left_down_y = (obj_y - 0.1f * normolize_matrix[4]);

                float obj_right_down_x = (obj_x + 0.1f * normolize_matrix[0]);

                float obj_right_up_y = (obj_y + 0.1f * normolize_matrix[4]);

                float dx = tank_x * normolize_matrix[0] - obj_x;
                float dy = tank_y * normolize_matrix[4] - obj_y;

                float distance = sqrt(dx * dx + dy * dy);

                if (distance < 0.4 * (1 / scale_coefficient))
                {
                    have_obstacle = true;
                }

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
                         float         angle,
                         vector<float> norm,
                         float         coeff,
                         int*          score_)
{
    return new enemy(
        eng, tank, x, y, angle, norm, coeff, score_);
}
