
#include "if.h"



//------------------------------------------------------------------------------
// Game
//------------------------------------------------------------------------------
class Game
{
public:
	enum
	{
		S_Titie = 0,
		S_Play,
		S_Ending,
		S_Result,
		S_Max,
	};
	
	enum
	{
		Width  = 1024,
		Height = 800,
	};

	Util::ShowFps fps;
	Util::Random  bgrand;
	Render        render;
	Input         input;
	System        system;


	int State = S_Titie;
	void Title() {
		fps.Init();
		State = S_Play;
	}

	void Play() {
		float col[4] = { 0, 0, 1, 1 };
		render.Clear(col);

		static float kkk = 0;
		kkk += 0.003;
		static float t = 0;
		t += 0.015555;
		float R = 50;
		render.Proj(60.0f * (3.14 / 180.0), float(system.Width) / float(system.Height), 0.1f, 2560.0f);
		render.View(
			//0.3, 1.8, -0.5,
			sin(t * 0.4) * 2.3, 0.88, -cos(t * 0.3) * 2.1,
			0, 0, 0,
			0, 1, 0);


		bgrand.Set();
		srand(0);
		for (float z = -10; z < 20; z += 0.22)
			for (float x = -10; x < 20; x += 0.22)
				render.Object(
					x, sin(bgrand.Float(kkk) + t * 5.3) * 0.05, z,
					//x, bgrand.Float(kkk) + sin(bgrand.Float(kkk) + t) * 0.1, z,
					0, 0, 0,
					0.1, 0.1, 0.1,
					//1,1,1, //mono
					0.1 + abs(bgrand.Float()), 0.1 + abs(bgrand.Float()), 0.1 + abs(bgrand.Float()), 
				1);
		render.Draw(0);
	}


	void Ending() {
	}


	void Result() {

	}



	void Init()
	{
		system.Init("aaaaaa", Width, Height);
		render.Init(system.GetHandle(), Width, Height);
		input.Init(system.GetHandle());
	}

	void Term()
	{
	}

	void Update()
	{

		switch (State)
		{
		case  S_Titie:
			Title();
			break;
		case  S_Play:
			Play();
			break;
		case  S_Ending:
			Ending();
			break;
		case  S_Result:
			Result();
			break;
		default:
			exit(0);
			break;
		}
		/*
		if (input.Get(Input::Up))    printf("UP");
		if (input.Get(Input::Down))  printf("DOWN");
		if (input.Get(Input::Left))  printf("LEFT");
		if (input.Get(Input::Right)) printf("RIGHT");
		if (input.Get(Input::B0))    printf("B0");
		if (input.Get(Input::B1))    printf("B1");
		if (input.Get(Input::B2))    printf("B2");
		if (input.Get(Input::B3))    printf("B3");
		printf("\n");
		*/
		render.Flip();

		fps.Update();

		system.Sleep();
	}
};

int main() {
	Game game;
	game.Init();
	while(game.system.IsDone() == false) {
		game.Update();
	}
	return 0;
}

