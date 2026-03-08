#include <iostream>
#include <cstdlib>
#include <ctime>
#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL.h>
#include <../glm/glm.hpp>
#include <SDL3/SDL_render.h>
#include "../Global/global.h"

class Box
{
  public:
    // 运动学属性 (Kinematics)
    glm::vec2 position; // x y 坐标
    glm::vec2 velocity; // 左右移动和跳跃
    glm::vec2 size;     // 宽度和高度

    // 动力学属性 (Dynamics)
    float mass;     // 质量：重的物体撞击力大，且更难被推开
    float inv_mass; // 质量的倒数 (1/m)：物理引擎优化常用，静止物体质量无穷大，则 inv_mass 为 0

    // 材质属性 (Material Properties)
    float restitution; // 恢复系数 (弹性)：0 是橡皮泥，1 是完美弹球
    float friction;    // 摩擦系数：决定在地面滑动时的减速快慢

    // 颜色
    SDL_Color color;

  public:
    Box(glm::vec2 pos, glm::vec2 sz, float m, SDL_Color c)
        : position(pos), size(sz), mass(m), color(c)
    {
        velocity = glm::vec2(0.0f, 0.0f);
        inv_mass = (m > 0.0f) ? 1.0f / m : 0.0f; // 质量为0代表不可移动
    }

    void UpdateBox(float dt, float gravity);

    void RenderBox(SDL_Renderer *renderer);
};

Box CreateBox(glm::vec2 pos, glm::vec2 size, float mass, float rest, float fric, SDL_Color color);

void ResolveCollision(Box &a, Box &b);
