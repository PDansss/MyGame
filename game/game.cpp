#include "engine.hpp"
#include "tank.hpp"

int main()
{
    IEngine* myeng = create_engine();

    float width  = 1920;
    float height = 1080;
    float scale  = 0.4;

    myeng->init(width, height);

    my_math mat;

    vector<float> norm =
        mat.matrix_multiplying(mat.scaling_matrix(height / width, 1),
                               mat.scaling_matrix(scale, scale));

    float scaling_coff = 1 / scale;

    vector<stack<missile>*> positions;

    int score = 0;

    ITank* player = create_tank(myeng, 1.7, 1.8, 0, 0, norm, scaling_coff);

    ITank* enemy1 = create_enemy_tank(
        myeng, player, -0.5, -0.3, 0, 0, norm, scaling_coff, &score);

    ITank* enemy2 = create_enemy_tank(
        myeng, player, 0.5, -0.8, 0, 0, norm, scaling_coff, &score);

    ITank* enemy3 = create_enemy_tank(
        myeng, player, -0.5, 0.3, 0, 0, norm, scaling_coff, &score);

    ITank* enemy4 = create_enemy_tank(
        myeng, player, 0.5, -0.4, 0, 0, norm, scaling_coff, &score);

    positions.push_back(enemy1->get_missile_stack());
    positions.push_back(enemy2->get_missile_stack());
    positions.push_back(enemy3->get_missile_stack());
    positions.push_back(enemy4->get_missile_stack());

    player->set_stack(positions);

    obstacles obs(myeng, norm, scaling_coff);

    obs.set_obstacle("res/level.txt");

    bool show_main_menu = true, pause = false, exit = false;

    while (true)
    {
        myeng->next_frame_gui();
        if (!myeng->read_events() || exit)
            break;

        if (show_main_menu)
        {
            myeng->draw_main_menu(show_main_menu, exit);
        }
        else
        {

            if (!player->IsDead() && !pause)
            {
                enemy1->update(*obs.get_obstacle());
                enemy2->update(*obs.get_obstacle());
                enemy3->update(*obs.get_obstacle());
                enemy4->update(*obs.get_obstacle());
                player->update(*obs.get_obstacle());
                myeng->draw_wiget_elements(pause, score);
            }
            else
            {
                if (player->IsDead())
                {
                    player->restart();
                    score = 0;
                }
            }

            myeng->draw_pause_menu(pause, show_main_menu);
            obs.render();
            enemy1->render();
            enemy2->render();
            enemy3->render();
            enemy4->render();
            player->render();
        }
        myeng->display();
    }

    delete_tank(player);
    delete_tank(enemy1);
    delete_tank(enemy2);
    delete_tank(enemy3);
    delete_tank(enemy4);

    myeng->clean();
    delete_engine(myeng);
    return 0;
}
