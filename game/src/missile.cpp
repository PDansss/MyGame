#include "missile.hpp"

void Missile::add_missile_to_stack(float x, float y, float angle)
{
	missiles.push({ x, y, 0.f, 0.f, angle });
}

void Missile::draw_missile(IEngine* eng, vector<float> normolize_matrix)
{
    stack<missile> temp_missiles;
    while (!missiles.empty())
    {
        float dy = missiles.top().dy;
        float dx = missiles.top().dx;
        float centerY = missiles.top().center_y;
        float centerX = missiles.top().center_x;
        float Angile = missiles.top().angle;
        missiles.pop();

        vector<float> shoot_norm = matrices.matrix_multiplying(
            normolize_matrix, matrices.shift_matrix(dx, dy));

        vector<float> shoot_shift = matrices.shift_matrix(
            centerX + 0.24f * cos(Angile), centerY + 0.24f * sin(Angile));
        vector<float> shoot_rotate = matrices.rotate_matrix(Angile);

        eng->render_triangle("missile",
            shoot_norm,
            shoot_shift,
            shoot_rotate,
            color,
            eng->get_texture("missile"),
            false);

        temp_missiles.push({ centerX, centerY, dx, dy, Angile });
    }


    missiles = temp_missiles;
}

void Missile::update(IEngine* eng, vector<obstacle>& objects, vector<float> normolize_matrix, float scaling_coefficient, animation& anim)
{
    bool           missile_collision;
    stack<missile> temp_missiles;
    while (!missiles.empty()) // ќбработка коллизий снар€дов со стенами и
                              // движение снар€да
    {
        missile_collision = false;

        float dy = missiles.top().dy;
        float dx = missiles.top().dx;
        float centerY = missiles.top().center_y;
        float centerX = missiles.top().center_x;
        float Angile = missiles.top().angle;
        missiles.pop();

        dx += (0.24f * cos(Angile)) / missile_speed;
        dy += (0.24f * sin(Angile)) / missile_speed;

        float X = normolize_matrix[0] * (centerX + 0.24f * cos(Angile)) +
            normolize_matrix[0] * dx;

        float Y = normolize_matrix[4] * (centerY + 0.24f * sin(Angile)) +
            normolize_matrix[4] * dy;

        for (int i = 0; i < objects.size(); i++)
        {
            if (objects[i].exist)
            {
                float obj_x = objects[i].x * normolize_matrix[0];
                float obj_y = objects[i].y * normolize_matrix[4];

                float obj_left_down_x = (obj_x - 0.1f * normolize_matrix[0]);
                float obj_left_down_y = (obj_y - 0.1f * normolize_matrix[4]);

                float obj_right_down_x = (obj_x + 0.1f * normolize_matrix[0]);

                float obj_right_up_y = (obj_y + 0.1f * normolize_matrix[4]);

                if (X >= obj_left_down_x &&
                    X <= obj_right_down_x &&
                    Y >= obj_left_down_y &&
                    Y <= obj_right_up_y)
                {
                    missile_collision = true;
                    if (objects[i].change)
                        objects[i].exist = false;
                    if (objects[i].breakable)
                        objects[i].change = true;

                    anim.add_animation(X, Y, 1.5f);
                    break;
                }
            }
        }

        if (!missile_collision)
        {
            temp_missiles.push({ centerX, centerY, dx, dy, Angile });
        }
    }
    missiles = temp_missiles;
}

bool Missile::missile_collision_with_enemy(stack<missile>* stack, float x, float y, float scale_coeff, vector<float> normolize_matrix)
{
    bool collision = false;

    std::stack<missile> temp_missiles;
    std::stack<missile> temp_missiles2 = *stack;

    if (!temp_missiles2.empty())
    {

        while (!temp_missiles2.empty())
        {

            float dy = temp_missiles2.top().dy;
            float dx = temp_missiles2.top().dx;
            float missile_center_y = temp_missiles2.top().center_y;
            float missile_center_x = temp_missiles2.top().center_x;
            float Angile = temp_missiles2.top().angle;
            temp_missiles2.pop();

            float obj_x = normolize_matrix[0] *
                (missile_center_x + 0.25f * cos(Angile)) +
                normolize_matrix[0] * dx;

            float obj_y = normolize_matrix[4] *
                (missile_center_y + 0.25f * sin(Angile)) +
                normolize_matrix[4] * dy;

            float player_x = x * normolize_matrix[0];
            float player_y = y * normolize_matrix[4];
            float dX = obj_x - player_x;
            float dY = obj_y - player_y;
            float distance = sqrt(dX * dX + dY * dY);

            if (distance <= 0.1f * normolize_matrix[0] * scale_coeff)
            {
                collision = true;
            }

            else
            {
                temp_missiles.push(
                    { missile_center_x, missile_center_y, dx, dy, Angile });
            }
        }
        *stack = temp_missiles;
    }
    return collision;
}

bool Missile::missile_collision_with_player(vector<stack<missile>*> vector_missile_stack, float x, float y, float scale_coeff, vector<float> normolize_matrix)
{
    bool collision = false;
    for (int i = 0; i < vector_missile_stack.size(); i++)
        //ќбработка коллизий танка со снар€дами
    {
        std::stack<missile> temp_missiles;
        std::stack<missile> temp_missiles2;

        temp_missiles2 = *vector_missile_stack[i];
        if (!temp_missiles2.empty())
        {

            while (!temp_missiles2.empty())
            {

                float dy = temp_missiles2.top().dy;
                float dx = temp_missiles2.top().dx;
                float missile_center_y = temp_missiles2.top().center_y;
                float missile_center_x = temp_missiles2.top().center_x;
                float Angile = temp_missiles2.top().angle;
                temp_missiles2.pop();

                float obj_x = normolize_matrix[0] *
                    (missile_center_x + 0.25f * cos(Angile)) +
                    normolize_matrix[0] * dx;

                float obj_y = normolize_matrix[4] *
                    (missile_center_y + 0.25f * sin(Angile)) +
                    normolize_matrix[4] * dy;

                float player_x = x * normolize_matrix[0];
                float player_y = y * normolize_matrix[4];
                float dX = obj_x - player_x;
                float dY = obj_y - player_y;
                float distance = sqrt(dX * dX + dY * dY);

                if (distance <= 0.1f * normolize_matrix[0] * scale_coeff)
                {
                    collision = true;
                }

                else
                {
                    temp_missiles.push({ missile_center_x,
                                         missile_center_y,
                                         dx,
                                         dy,
                                         Angile });
                }
            }
            *vector_missile_stack[i] = temp_missiles;
        }
    }
    return collision;
}

stack<missile>* Missile::get_stack()
{
    return &missiles;
}
