#pragma once

struct App;
typedef unsigned int           u32;

u32 LoadModel(App* app, const char* filename);

u32 CreatePlane(App* app);
u32 CreateCube(App* app);

