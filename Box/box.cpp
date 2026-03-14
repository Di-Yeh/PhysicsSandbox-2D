#include "box.h"

/// <summary>
/// 更新Box
/// </summary>
/// <param name="dt"></param>
/// <param name="gravity"></param>
/// <returns></returns>
void Box::UpdateBox(float dt, float gravity)
{
    // 质量超重的沙子 -> 不会移动
    if (mass == 0)
    {
        return;
    }

    // 重力只影响 Y 轴
    velocity.y += gravity * 100.0f * dt; // 这里的 100.0 是像素/米 的比例转换

    // 更新位置
    position += velocity * dt;

    // 底部碰撞 (Y 轴反弹)
    if (position.y + size.y >= (float)Window_Height)
    {
        position.y = (float)Window_Height - size.y;
        velocity.y = -velocity.y * restitution;

        // 落地时，水平方向因为摩擦力而减速
        velocity.x *= (1.0f - friction);
    }

    // 左右墙壁碰撞 (X 轴反弹)
    if (position.x <= 0 || position.x + size.x >= (float)Window_Wdith)
    {
        velocity.x = -velocity.x * restitution;
        // 修正位置防止穿墙
        position.x = (position.x <= 0) ? 0 : (float)Window_Wdith - size.x;
    }
}

/// <summary>
/// 绘制Box
/// </summary>
/// <param name="renderer"></param>
/// <returns></returns>
void Box::RenderBox(SDL_Renderer *renderer)
{
    SDL_FRect rect = {position.x, position.y, size.x, size.y};
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 255);
    SDL_RenderFillRect(renderer, &rect);
}

/// <summary>
/// 创造一个方块
/// </summary>
/// <param name="pos"></param>
/// <param name="size"></param>
/// <param name="mass"></param>
/// <param name="rest"></param>
/// <param name="fric"></param>
/// <param name="color"></param>
/// <returns>Box</returns>
Box CreateBox(glm::vec2 pos, glm::vec2 size, float mass, float rest, float fric, SDL_Color color)
{
    // 给方块一个随机的初始水平速度，让它看起来更像“喷”出来的
    float randomVX = (float)(SDL_rand(200) - 100); // -100 到 100 之间
    glm::vec2 initialVel = glm::vec2(randomVX, 0.0f);

    // 直接构造对象并返回
    Box newBox(pos, size, mass, color);

    newBox.velocity = initialVel;
    newBox.restitution = rest;
    newBox.friction = fric;

    return newBox;
}

/// <summary>
/// 方块碰撞逻辑
/// </summary>
/// <param name="a"></param>
/// <param name="b"></param>
/// <returns></returns>
void ResolveCollision(Box &a, Box &b)
{
    // 计算两个中心点的距离
    glm::vec2 centerA = a.position + a.size * 0.5f;
    glm::vec2 centerB = b.position + b.size * 0.5f;
    glm::vec2 delta = centerB - centerA;

    // 计算重叠程度
    float overlapX = (a.size.x * 0.5f + b.size.x * 0.5f) - SDL_fabs(delta.x);
    float overlapY = (a.size.y * 0.5f + b.size.y * 0.5f) - SDL_fabs(delta.y);

    if (overlapX > 0 && overlapY > 0) // 判断是否碰撞
    {
        float totalInvMass = a.inv_mass + b.inv_mass;
        if (totalInvMass <= 0.0f)
        {
            return; // 两个都是固定物体 不处理
        }

        // 计算推开比例：质量小的挪得多
        float ratioA = a.inv_mass / totalInvMass;
        float ratioB = b.inv_mass / totalInvMass;

        // 统一的弹性系数 (可以取两者中最小的，或者平均值)
        float e = SDL_min(a.restitution, b.restitution);

        // 找出重叠最小的轴（MTV - Minimum Translation Vector）
        if (overlapX < overlapY) // 在X轴碰撞
        {
            float sign = (delta.x > 0) ? 1.0f : -1.0f;

            // 位置修正 (MTV)
            a.position.x -= overlapX * ratioA * sign;
            b.position.x += overlapX * ratioB * sign;

            // 速度处理 (简单的弹性反转)
            // 只有当两者速度是朝向对方时才反转，防止已经分开还在吸
            float relativeVelX = b.velocity.x - a.velocity.x;
            if (relativeVelX * sign < 0)
            {
                a.velocity.x *= -e;
                b.velocity.x *= -e;
            }
        }
        else // 在Y轴碰撞
        {
            float sign = (delta.y > 0) ? 1.0f : -1.0f;

            // 位置修正
            a.position.y -= overlapY * ratioA * sign;
            b.position.y += overlapY * ratioB * sign;

            // 速度处理
            float relativeVelY = b.velocity.y - a.velocity.y;
            if (relativeVelY * sign < 0)
            {
                // 如果是踩在头顶，稍微增加一点损耗防止无限抖动
                a.velocity.y *= -e;
                b.velocity.y *= -e;
            }
        }
    }
}
