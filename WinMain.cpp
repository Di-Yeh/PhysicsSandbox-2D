#include <iostream>
#include <windows.h>
#include <cstdlib>
#include <ctime>
#include <vector>
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include "Box/box.h"
#include "Global/global.h"
#include "SDL3/SDL_keycode.h"
#include "SDL3/SDL_scancode.h"
#include "SDL3/SDL_stdinc.h"

SDL_Window *window = SDL_CreateWindow("2D 物理沙盒 程序窗口", Window_Wdith, Window_Height, 0);
SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);
MIX_Mixer *mixer = NULL;
MIX_Track *bgmTrack = NULL;
MIX_Audio *music = NULL;
MIX_Audio *clickSound = NULL;
TTF_Font *font;
SDL_Texture *texture = NULL;

/// <summary>
/// 主循环
/// </summary>s
/// <param name=""></param>
/// <returns></returns>
void MainLoop()
{
    /* 事件循环 */
    bool isQuit = false;
    SDL_Event event;

    glm::vec2 mousePos = glm::vec2(0.0f, 0.0f);
    glm::vec2 boxSize(10.0f, 10.0f);

    std::vector<Box> vecBox;

    float boxMass = 3.0f;
    float boxRestitution = 0.6f;
    float boxFriction = 0.2f;
    float gGravity = 9.8f;

    SDL_Color boxColor = {255, 255, 0, 255}; // 黄色
    SDL_Color randomBoxColor = {(Uint8)(std::rand() % 256), (Uint8)(std::rand() % 256), (Uint8)(std::rand() % 256), 255};
    float tempR = (float)boxColor.r;
    float tempG = (float)boxColor.g;
    float tempB = (float)boxColor.b;
    char flagColor = 0;

    Uint32 delayTime;
    float targetFPS = 60.0f;

    // 图片
    texture = IMG_LoadTexture(renderer, "assets/picture/SDL3_Logo.png");

    SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    SDL_SetTextureAlphaMod(texture, 100); // 128 大約是 50% 的透明度 (255 * 0.5)

    // 文字
    char strBoxCounter[0x64];
    char strFPS[0x64] = "FPS: 0";
    char strBox[0x1024];

    // 绘制文本
    TTF_TextEngine *textEngine = TTF_CreateRendererTextEngine(renderer);

    TTF_Text *textBoxCounter = TTF_CreateText(textEngine, font, strBoxCounter, 0);
    TTF_SetTextColor(textBoxCounter, 255, 255, 0, 255);
    TTF_SetTextWrapWidth(textBoxCounter, 300);

    TTF_Text *textFPS = TTF_CreateText(textEngine, font, strFPS, 0);
    TTF_SetTextColor(textFPS, 0, 255, 0, 255);
    TTF_SetTextWrapWidth(textFPS, 300);

    TTF_Text *textBox = TTF_CreateText(textEngine, font, strBox, 0);
    TTF_SetTextColor(textBox, 102, 153, 255, 255);
    TTF_SetTextWrapWidth(textBox, 600);

    // 设置文字居中
    TTF_SetFontWrapAlignment(font, TTF_HORIZONTAL_ALIGN_CENTER);

    // 初始化时间
    Uint32 last_time = SDL_GetTicks();
    float fpsTimer = 0.0f;
    int frameCount = 0;
    int currentFPS = 0;

    while (!isQuit)
    {
        // 处理输入 (Input)
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
                // 退出程序
            case SDL_EVENT_QUIT:
                isQuit = true;
                break;

            // 键盘按下事件
            case SDL_EVENT_KEY_DOWN:
                switch (event.key.key)
                {
                case SDLK_C:
                    vecBox.clear(); // 清空所有方块
                    break;

                case SDLK_ESCAPE:
                    isQuit = true; // 按下 ESC 退出程序
                    break;
                default:
                    break;
                }
            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                // 检查是不是左键
                if (event.button.button == SDL_BUTTON_LEFT)
                {
                    SDL_GetMouseState(&mousePos.x, &mousePos.y);

                    // 播放音效
                    MIX_PlayAudio(mixer, clickSound);

                    // 调用工厂函数并存入 vector
                    if (!flagColor)
                    {
                        vecBox.push_back(CreateBox(mousePos, boxSize, boxMass, boxRestitution, boxFriction, boxColor));
                    }
                    else
                    {
                        randomBoxColor = {(Uint8)(std::rand() % 256), (Uint8)(std::rand() % 256), (Uint8)(std::rand() % 256), 255};
                        vecBox.push_back(CreateBox(mousePos, boxSize, boxMass, boxRestitution, boxFriction, randomBoxColor));
                    }
                }
                break;
            default:
                break;
            }
        }

        /* =======================> 清空屏幕 <======================= */
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255); // 背景黑
        SDL_RenderClear(renderer);

        snprintf(strBoxCounter, sizeof(strBoxCounter), "当前方块数量: %llu", vecBox.size());

        snprintf(strBox, sizeof(strBox), "方块大小 w:%f h:%f\n方块质量:%f\n方块恢复系数:%f\n方块摩擦系数:%f\n方块颜色 r:%u g:%u b:%u\n随机颜色启动:%d\n重力:%f\nfps参数:%f", boxSize.x, boxSize.y, boxMass, boxRestitution, boxFriction, boxColor.r, boxColor.g, boxColor.b, flagColor, gGravity, targetFPS);

        /* =======================> 主要内容 <======================= */

        // 獲取當前時間
        Uint32 current_time = SDL_GetTicks();
        frameCount++;
        float dt = (current_time - last_time) / 1000.0f;
        fpsTimer += dt;

        // 每隔一秒计算一次 FPS
        if (fpsTimer >= 1.0f) // 当累计满 1 秒时
        {
            currentFPS = frameCount;
            frameCount = 0;
            fpsTimer -= 1.0f; // 减去 1 秒，剩下的残余时间留到下次，更精准

            // 更新字符串内容（不要每一帧都 sprintf，只在 FPS 变动时更新）
            snprintf(strFPS, sizeof(strFPS), "FPS: %d", currentFPS);
        }

        for (size_t i = 0; i < vecBox.size(); ++i)
        {
            // 方块掉落更新
            vecBox[i].UpdateBox(dt, gGravity);

            for (size_t j = i + 1; j < vecBox.size(); ++j)
            {
                // 碰撞检测
                ResolveCollision(vecBox[i], vecBox[j]);
            }

            // 绘制沙盒物体
            vecBox[i].RenderBox(renderer);
        }

        /* =======================> 监听按键 <======================= */
        flagColor = (GetKeyState(0x47) & 0x0001) != 0;

        // 在主循环内，渲染和物理更新之前
        const bool *keys = SDL_GetKeyboardState(NULL);

        // boxSize
        if (keys[SDL_SCANCODE_Q] && boxSize.x > 1.f)
        {
            boxSize.x -= o_boxSize * dt;
        }
        if (keys[SDL_SCANCODE_W] && boxSize.x < 100.f)
        {
            boxSize.x += o_boxSize * dt;
        }
        if (keys[SDL_SCANCODE_E] && boxSize.y > 1.f)
        {
            boxSize.y -= o_boxSize * dt;
        }
        if (keys[SDL_SCANCODE_R] && boxSize.y < 100.f)
        {
            boxSize.y += o_boxSize * dt;
        }

        // boxMass
        if (keys[SDL_SCANCODE_A] && boxMass > 0.1f)
        {
            boxMass -= o_boxMass * dt;
        }
        if (keys[SDL_SCANCODE_S] && boxMass < 100.f)
        {
            boxMass += o_boxMass * dt;
        }

        // boxRestitution
        if (keys[SDL_SCANCODE_D] && boxRestitution > 0.1f)
        {
            boxRestitution -= o_boxRestitution * dt;
        }
        if (keys[SDL_SCANCODE_F] && boxRestitution < 100.f)
        {
            boxRestitution += o_boxRestitution * dt;
        }

        // boxFriction
        if (keys[SDL_SCANCODE_Z] && boxFriction > 0.1f)
        {
            boxFriction -= o_boxFriction * dt;
        }
        if (keys[SDL_SCANCODE_X] && boxFriction < 100.f)
        {
            boxFriction += o_boxFriction * dt;
        }

        // boxcolor
        // R
        if (keys[SDL_SCANCODE_Y])
            tempR -= o_boxColor * dt;
        if (keys[SDL_SCANCODE_U])
            tempR += o_boxColor * dt;

        // G
        if (keys[SDL_SCANCODE_H])
            tempG -= o_boxColor * dt;
        if (keys[SDL_SCANCODE_J])
            tempG += o_boxColor * dt;

        // B
        if (keys[SDL_SCANCODE_N])
            tempB -= o_boxColor * dt;
        if (keys[SDL_SCANCODE_M])
            tempB += o_boxColor * dt;

        // 统一约束并赋值
        tempR = glm::clamp(tempR, 0.0f, 255.0f);
        tempG = glm::clamp(tempG, 0.0f, 255.0f);
        tempB = glm::clamp(tempB, 0.0f, 255.0f);
        boxColor.r = (Uint8)tempR;
        boxColor.g = (Uint8)tempG;
        boxColor.b = (Uint8)tempB;

        // gGravity
        if (keys[SDL_SCANCODE_V] && gGravity > 0.1f)
        {
            gGravity -= o_gGravity * dt;
        }
        if (keys[SDL_SCANCODE_B] && gGravity < 100.f)
        {
            gGravity += o_gGravity * dt;
        }

        // fps
        if (keys[SDL_SCANCODE_LEFTBRACKET])
        {
            targetFPS -= 10.0f * dt;
        }
        if (keys[SDL_SCANCODE_RIGHTBRACKET])
        {
            targetFPS += 10.0f * dt;
        }
        // 限制 FPS 范围
        targetFPS = glm::clamp(targetFPS, 10.0f, 300.0f);

        // 更新文字
        TTF_SetTextString(textBoxCounter, strBoxCounter, 0);
        TTF_SetTextString(textFPS, strFPS, 0);
        TTF_SetTextString(textBox, strBox, 0);

        // 新的画文本方法：
        TTF_DrawRendererText(textBoxCounter, 5.0f, 30.0f);
        TTF_DrawRendererText(textFPS, 0.0f, 5.0f);
        TTF_DrawRendererText(textBox, -20.0f, 80.0f);

        /* =======================> 绘制图片 <======================= */
        SDL_FRect dstrect = {810, 100, 380, 200};
        SDL_RenderTexture(renderer, texture, NULL, &dstrect);

        /* =======================> 绘制屏幕 <======================= */
        SDL_RenderPresent(renderer);

        /* =======================> 控制帧率 <======================= */
        delayTime = (Uint32)(1000.0f / targetFPS);
        SDL_Delay(delayTime);
        last_time = current_time;
    }

    // 清理字体资源
    TTF_DestroyText(textBoxCounter);
    TTF_DestroyText(textFPS);
    TTF_DestroyRendererTextEngine(textEngine);
    TTF_CloseFont(font);
    TTF_Quit();
}

/// <summary>
/// 主函数
/// </summary>
/// <param name="argc"></param>
/// <param name="argv"></param>
/// <returns></returns>
int main(int argc, char *argv[])
{
    std::srand(std::time(NULL));

    /* 初始化 SDL */
    if (SDL_Init(SDL_INIT_VIDEO) == false)
    {
        printf("SDL 初始化失败: %s\n", SDL_GetError());
        return -1;
    }

    /* 创建窗口 */
    if (!window)
    {
        printf("窗口创建失败: %s\n", SDL_GetError());
        SDL_Quit();
        return -1;
    }

    /* 创建渲染器 */
    if (!renderer)
    {
        printf("渲染器创建失败: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return -1;
    }

    // 在軌道上播放音樂
    SDL_AudioSpec spec;
    SDL_zero(spec);
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = 44100;

    if (!MIX_Init())
    {
        SDL_Log("MIX_Init 失敗: %s", SDL_GetError());
    }

    mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec);
    if (mixer == NULL)
    {
        SDL_Log("MIX_CreateMixerDevice 失敗: %s", SDL_GetError());
    }

    // 读取音乐
    music = MIX_LoadAudio(mixer, "assets/bgm/music.ogg", true);
    clickSound = MIX_LoadAudio(mixer, "assets/sound/click.wav", true);

    if (!music || !clickSound)
    {
        SDL_Log("音訊加載失敗: %s", SDL_GetError());
    }

    // 控制音量
    bgmTrack = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(bgmTrack, music);

    // 調整音量 (使用 0.0f 到 1.0f 的浮點數)
    // 獲取軌道的屬性 ID
    SDL_PropertiesID trackProps = MIX_GetTrackProperties(bgmTrack);
    SDL_SetFloatProperty(trackProps, 0, 0.25f);

    // 播放背景音乐
    SDL_PropertiesID playProps = SDL_CreateProperties();
    SDL_SetNumberProperty(playProps, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

    MIX_PlayTrack(bgmTrack, playProps);

    SDL_DestroyProperties(playProps);

    /* SDL_TTF初始化 */
    if (TTF_Init() == false)
    {
        printf("SDL TTF 初始化失败: %s\n", SDL_GetError());
        return -1;
    }

    // 加载字体
    font = TTF_OpenFont("assets/font/VonwaonBitmap-16px.ttf", 24);
    if (!font)
    {
        SDL_Log("字体加载失败: %s", SDL_GetError());
    }

    MainLoop();

    /* 清理与退出 */
    MIX_DestroyTrack(bgmTrack);
    MIX_DestroyAudio(music);
    MIX_DestroyAudio(clickSound);
    MIX_DestroyMixer(mixer);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
