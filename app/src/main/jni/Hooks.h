#include "Canvas/StructsCommon.h"
#include <cstdint>

#define targetLibName OBFUSCATE("libil2cpp.so")

// Offsets должны быть отдельно получены для каждой ABI.
// Значения ниже известны только для той ABI/сборки,
// для которой ты их изначально получил.

#if defined(__aarch64__)

    // arm64-v8a
    constexpr uintptr_t OFFSET_GET_POSITION = 0xFB0FE0;
    constexpr uintptr_t OFFSET_GET_TRANSFORM = 0x11962D4;
    constexpr uintptr_t OFFSET_CAMERA_GET_MAIN = 0x1A0ECF4;
    constexpr uintptr_t OFFSET_WORLD_TO_SCREEN = 0x1193FF0;
	constexpr uintptr_t HEALTH_OFFSET = 0x204; // 64-bit public class Player : MonoBehaviour private float FEHAJLBCGIN
	constexpr uintptr_t TEAM_OFFSET = 0x228; // 64-bit public class Player : MonoBehaviour private int BMFGOOEECIC;

#elif defined(__arm__)

    // armeabi-v7a
    constexpr uintptr_t OFFSET_GET_POSITION = 0x123456;
    constexpr uintptr_t OFFSET_GET_TRANSFORM = 0x123456;
    constexpr uintptr_t OFFSET_CAMERA_GET_MAIN = 0x123456;
    constexpr uintptr_t OFFSET_WORLD_TO_SCREEN = 0x123456;
	constexpr uintptr_t HEALTH_OFFSET = 0x123456;
	constexpr uintptr_t TEAM_OFFSET = 0x123456;

#else
    #error "Unsupported ABI"
#endif

//private extern void get_position_Injected(out Vector3 ret);
//Transform.get_position_Injected();
Vector3 get_position(void *transform) {
    if (!transform)
        return Vector3();

    Vector3 position;

    static const auto fn =
        reinterpret_cast<uintptr_t(__fastcall *)(void *, Vector3 &)>(
            getAbsoluteAddress(targetLibName, OFFSET_GET_POSITION)
        );

    fn(transform, position);
    return position;
}

//Component.get_transform();
void *get_transform(void *player) {
    if (!player)
        return nullptr;

    static const auto fn =
        reinterpret_cast<uintptr_t(__fastcall *)(void *)>(
            getAbsoluteAddress(targetLibName, OFFSET_GET_TRANSFORM)
        );

    return reinterpret_cast<void *>(fn(player));
}


class Camera {
public:
    static Camera *get_main() {
        auto fn =
            reinterpret_cast<Camera *(*)()>(
                getAbsoluteAddress(targetLibName, OFFSET_CAMERA_GET_MAIN)
            );

        return fn();
    }
};


Vector3 WorldToScreen(Vector3 pos) {
    auto main = Camera::get_main();

    if (main) {
        auto fn =
            reinterpret_cast<Vector3 (*)(Camera *, Vector3)>(
                getAbsoluteAddress(targetLibName, OFFSET_WORLD_TO_SCREEN)
            );

        return fn(main, pos);
    }

    return {0, 0, 0};
}

//FEHAJLBCGIN
float GetPlayerHealth(void *player) {
    if (!player)
        return 0.0f;

    auto address = reinterpret_cast<uintptr_t>(player) + HEALTH_OFFSET; 
	return *reinterpret_cast<float *>(address);
}


bool PlayerAlive(void *player) {
    return player != nullptr && GetPlayerHealth(player) > 0.0f;
}

// BMFGOOEECIC
int32_t GetPlayerTeam(void *player) {
    if (!player)
        return 0;

    auto address = reinterpret_cast<uintptr_t>(player) + TEAM_OFFSET;
    return *reinterpret_cast<int32_t *>(address);
}

bool IsPlayerDead(void *player) {
    return player == nullptr || GetPlayerHealth(player) < 1.0f;
}


Vector3 getPosition(void *transform) {
    return get_position(get_transform(transform));
}

