#include   "common.h"
using namespace common;

#define    WindowX      800
#define    WindowY      600
#define    ScreenX      WindowX
#define    ScreenY      WindowY
double     g_time       = 0;
Direct2D   d2d;


//test
#define    TEST_MAX     512
struct obj {
  vec pos, dir;
  float angle;
  float dangle;
  void move() {
    pos += dir;
    angle += dangle;
    if(pos.x > ScreenX || pos.x < 0) dir.x = -dir.x;
    if(pos.y > ScreenY || pos.y < 0) dir.y = -dir.y;
  }
};
obj        testpos[TEST_MAX];


//-------//-------//-------//-------//-------//-------//-------//-------//-------//-------
// func
//-------//-------//-------//-------//-------//-------//-------//-------//-------//-------
void func_init(Direct2D *device) {
  device->CreateTextFmt(0, L"Consolas", 24);
  random rnd;
  obj *w = testpos;
  for(auto i = 0 ; i < TEST_MAX; i++) {
    w->pos = vec(abs(rnd.get()) % WindowX, abs(rnd.get()) % WindowY);
    w->dir = vec(rnd.fget(), rnd.fget());
    w->pos.disp();
    w->dir.disp();
    w->dangle = rnd.fget() * 12;
    w++;
  }
}

void draw_obj(Direct2D *device, obj &p) {
  device->DrawRect(p.pos.x, p.pos.y,  32 * 0.13, 32 * 0.03, p.angle,  64, TRUE, 1.0);
  device->DrawRect(p.pos.x, p.pos.y,  24 * 0.13, 24 * 0.03, 0,        32, TRUE, 1.0);
  device->DrawRect(p.pos.x, p.pos.y,  16 * 0.13, 16 * 0.13, 0,       128, TRUE, 1.0);
  device->DrawRect(p.pos.x, p.pos.y,  64 * 0.03, 64 * 0.13, p.angle, 192, TRUE, 1.5);
}

void func_update(double g_time) {
  for(auto i = 0 ; i < TEST_MAX; i++) testpos[i].move();
}

void func_term(Direct2D *device) {
}

void func_render(Direct2D *device) {
  //device->Clear(1, 1, 1, 1);
  device->Clear();
  device->DrawGrid(32, g_time * 30, g_time * 50, 200);
  float angle =  3.14 * 2 * g_time * 24;
  for(auto i = 0 ; i < TEST_MAX; i++) draw_obj(device, testpos[i]);
  device->DrawText(0, L"Direct2D\nTest.", -1, 0, 0, 240);
  g_time += 0.01666666666666666666666666666666666666666666666;
  show_fps();
}

int main(int argc, char *argv[]) {
  if(d2d.Init(InitWindow("D2D TEST", WindowX, WindowY), ScreenX, ScreenY) == S_OK){
    func_init(&d2d);
    while(ProcMsg()) {
      func_update(g_time);
      d2d.Render(func_render);
      Sleep(0);
    }
    func_term(&d2d);
  }
  d2d.Term();
  return 0;
}

