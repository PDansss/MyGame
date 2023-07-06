#include "engine.hpp"
#include "tank.hpp"

vector<float> tank_vertecies = {
   -0.2f,  0.1f, 0.0f, 0.0f, 1.0f,
    0.2f,  0.1f, 0.0f, 0.0f, 0.0f,
    0.2f, -0.1f, 0.0f, 1.0f, 0.0f,
   -0.2f, -0.1f, 0.0f, 1.0f, 1.0f
};

vector<float> missile_vertecies = { 
   -0.041f, -0.022f, 1.0f, 1.0f, 1.0f,
   -0.041f,  0.022f, 1.0f, 0.0f, 1.0f,
    0.041f,  0.022f, 1.0f, 0.0f, 0.0f,
    0.041f, -0.022f, 1.0f, 1.0f, 0.0f };

vector<float> animation_vertecies = { 
   -0.1f,  0.1f, 0.0f, 0.0f, 0.0f,
    0.1f,  0.1f, 0.0f, 1.0f, 0.0f,
    0.1f, -0.1f, 0.0f, 1.0f, 1.0f,
   -0.1f, -0.1f, 0.0f, 0.0f, 1.0f };

vector<float> obj_vertecies = { 
   -0.1f,  0.1f, 0.0f, 0.0f, 0.0f,
       0.1f,  0.1f, 0.0f, 1.0f, 0.0f,
       0.1f, -0.1f, 0.0f, 1.0f, 1.0f,
   -0.1f, -0.1f, 0.0f, 0.0f, 1.0f };

vector<float>background_vertecies = { 
   -1.0f,  1.0f, 0.0f, 0.0f, 0.0f,
    1.0f,  1.0f, 0.0f, 1.0f, 0.0f,
    1.0f, -1.0f, 0.0f, 1.0f, 1.0f,
   -1.0f, -1.0f, 0.0f, 0.0f, 1.0f };

vector<unsigned int> indexes = { 0,1,2,0,2,3 };

int main()
{
    IEngine* myeng = create_engine();

	int   width = 1900, height = 680;

    float scale  = 0.4f;
	float scaling_coff = 1 / scale;

	vector<float> norm;
    myeng->init(width, height,norm);

    myeng->set_buffer(myeng->get_vertex_buffer("tank"), tank_vertecies);
    myeng->set_buffer(myeng->get_index_buffer("tank"), indexes);
    myeng->set_buffer(myeng->get_vertex_buffer("missile"), missile_vertecies);
    myeng->set_buffer(myeng->get_index_buffer("missile"), indexes);
    myeng->set_buffer(myeng->get_vertex_buffer("animation"), animation_vertecies);
    myeng->set_buffer(myeng->get_index_buffer("animation"), indexes);
    myeng->set_buffer(myeng->get_vertex_buffer("object"), obj_vertecies);
    myeng->set_buffer(myeng->get_index_buffer("object"), indexes);
    myeng->set_buffer(myeng->get_vertex_buffer("background"), background_vertecies);
    myeng->set_buffer(myeng->get_index_buffer("background"), indexes);

    vector<stack<missile>*> positions;

    int score = 0;

    ITank* player = create_tank(myeng, 1.7f, 1.8f, 0.f, 0.f, norm, scaling_coff);

    ITank* enemy1 = create_enemy_tank(
        myeng, player, -0.5f, -0.3f, 0.f, 0.f, norm, scaling_coff, &score);

    ITank* enemy2 = create_enemy_tank(
        myeng, player, 0.5f, -0.8f, 0.f, 0.f, norm, scaling_coff, &score);

    ITank* enemy3 = create_enemy_tank(
        myeng, player, -0.5f, 0.3f, 0.f, 0.f, norm, scaling_coff, &score);

    ITank* enemy4 = create_enemy_tank(
        myeng, player, 0.5f, -0.4f, 0.f, 0.f, norm, scaling_coff, &score);

    positions.push_back(enemy1->get_missile_stack());
    positions.push_back(enemy2->get_missile_stack());
    positions.push_back(enemy3->get_missile_stack());
    positions.push_back(enemy4->get_missile_stack());

    player->set_stack(positions);

    obstacles obs(myeng, norm, scaling_coff);
    obs.set_obstacle("res/level.txt");

    bool show_main_menu = true, pause = false, exit = false;
	unsigned int time = 0, temp = 0;
	
    while (true)
    {
		time = myeng->get_time();
		if(time - temp > 16){
			temp = time;
			if (!myeng->read_events() || exit)
				break;

			if (show_main_menu)
			{
				myeng->draw_main_menu(show_main_menu, exit);
				enemy1->restart();
				enemy2->restart();
				enemy3->restart();
				enemy4->restart();
				player->restart();
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
