#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <windows.h>


#pragma comment(lib, "winmm.lib")

enum {
  CMD_NONE,
  CMD_WALK,
  CMD_POTION,
  CMD_ATTACK,
  CMD_QUIT,
};

auto main() -> int {
  struct unit {
    int32_t hp = 0;
    int32_t hp_max = 10;
    int32_t potion = 0;
    int32_t offense = 1;
    auto init(int a, int a_max, int np) -> void {
      hp = a;
      hp_max = a_max;
      potion = np;
    }
    auto disp(char *name) -> void {
      if(hp > 0) {
        printf("NAME:%s hp=%d, hp_max=%d, potion=%d\n", name, hp, hp_max, potion);
      }
    }

    auto use_potion() -> void {
      if(potion) {
        hp = (std::min)(hp_max, hp + (1 + (rand() % 6)));
        potion -= 1;
      } else {
        printf("Haven't potion fuck\n");
      }
    }

    auto attack(unit & a, int mod) -> void {
      if((rand() % mod) == 0) {
        int count = rand() % 5;
        a.hp -= count * offense;
        printf("DAMAGE:%d\n", count);
      } else {
        printf("MISSED\n");
      }
    }
  };

  unit ship;
  unit enemy;
  ship.init(10, 12, 10);


  int dist = 0;
  {
    auto kuji = timeGetTime() % 1234;
    volatile auto seed = 0;
    for(int i = 0 ;i < kuji; i++) {
      seed += rand();
    }
    printf("seed=%d\n", seed);
  }

  while(dist < 100) {
    char cmdbuf[134] = {};
    if(enemy.hp > 0) {
      printf("ENEMY HP : %d\n", enemy.hp);
    }
    printf("DISTANCE : %d\n", dist);
    ship.disp("SHIP");
    enemy.disp("ENEMY");
    printf("command : [1:walk] [2:potion] [3:attack] [9:quit]\n");
		printf(">");

    fgets(cmdbuf, sizeof(cmdbuf) - 1, stdin);
    int cmd = strtol(cmdbuf, 0, 10);

    switch(cmd) {
      case CMD_NONE:
        printf("Your seeing around this point\n");
        break;
      case CMD_WALK:
        printf("Your walk to forward...\n");
        dist++;

        break;
      case CMD_POTION:
        printf("Use konjou potion...\n");
        ship.use_potion();
        break;
      case CMD_ATTACK:
        if(enemy.hp > 0) {
          printf("YOUR ATTACK!\n");
          ship.attack(enemy, 3);
          if(enemy.hp <= 0) {
            printf("!!!!!!! ENEMY DOWN!\n");
            int hpinc = rand() % 6;
            printf("You were obteined hpmax=%d\n", hpinc);
            ship.hp_max += hpinc;
          }
        }
        break;
      default:
        printf("None\n");
        break;
    }

    if(enemy.hp > 0) {
      printf("WARNING : ENEMY ATTACK!\n");
      enemy.attack(ship, 11);
    } else {
      if( (rand() % 11) == 0)
      {
        enemy.hp = rand() % 30;
        printf("APPROACHING THE ENEMY!!\n");
      }
    }
    if(ship.hp <= 0) {
      printf("Your died...\n");
      printf("RESULT : dist=%d\n", dist);
      printf("Good bye\n");
      printf("G A M E  O V E R\n");
      break;
    }
  }
  if(ship.hp > 0) {
    printf("Congrats!!!!!!!!!!!! you got a tresure\n");
  }
  printf("T H E  E N D\n");

  return 0;
}
