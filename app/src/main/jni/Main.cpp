#include <list>
#include <vector>
#include <string.h>
#include <pthread.h>
#include <thread>
#include <cstring>
#include <jni.h>
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <dlfcn.h>
#include "Includes/Logger.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "KittyMemory/MemoryPatch.h"
#include "Menu/Setup.h"
#include "Hooks.h"
#include "ESPManager.h"

#include "Canvas/ESP.h"
#include "Canvas/Bools.h"
#include "Canvas/StructsCommon.h"

#include "And64InlineHook/And64InlineHook.hpp"

//Target lib here
#define targetLibName OBFUSCATE("libil2cpp.so")

#include "Includes/Macros.h"

Color color;
ESPManager *espManager;
ESP espOverlay;

float get_3D_Distance(float Self_x, float Self_y, float Self_z, float Object_x, float Object_y, float Object_z)
{
    float x, y, z;
    x = Self_x - Object_x;
    y = Self_y - Object_y;
    z = Self_z - Object_z;
    return (float)(sqrt(x * x + y * y + z * z));
}

#if defined(__aarch64__) //To compile this code for arm64 lib only. Do not worry about greyed out highlighting code, it still works
    // Hook example. Comment out if you don't use hook
    // Strings in macros are automatically obfuscated. No need to obfuscate!

constexpr uintptr_t IS_MINE_OFFSET = 0xE4; // isMine

#elif defined(__arm__) //To compile this code for armv7 lib only.

constexpr uintptr_t IS_MINE_OFFSET = 0x12; // isMine

#endif

void *enemyPlayer = NULL;
void *updatePlayer = NULL;

void (*old_Player_update)(...);
void Player_update(void *player) {
    if (player != NULL) {
        
        old_Player_update(player);
        updatePlayer = player;
        
        bool isMine = *reinterpret_cast<bool *>( reinterpret_cast<uintptr_t>(player) + IS_MINE_OFFSET ); // 64-bit public class Player : MonoBehaviour public bool isMine
        
        if (isMine) {
            myPlayer = player;
        }
       
    if(myPlayer) {
        if(GetPlayerTeam(myPlayer) != GetPlayerTeam(player)) {
            enemyPlayer = player;
        }
    }

        if (Esp) {              
                if (enemyPlayer) {             
                        espManager->tryAddEnemy(player);             
                }
                espManager->updateEnemies(player);
            }
        }
    }

void (*old_Player_ondestroy)(void *player);
void Player_ondestroy(void *player) {
    if (player != NULL) {
        old_Player_ondestroy(player);
        espManager->removeEnemyGivenObject(player);
    }
}   

void DrawESP(ESP esp, int screenWidth, int screenHeight) {
    
    esp.DrawText(Color::Red(), "Dark Team Mod Menu", Vector2(screenWidth / 2, screenHeight / 1.02f), 25.0f); 
    
    if(Esp) {
    if (espManager->enemies->empty()) {
        return;
    }
    for (int i = 0; i < espManager->enemies->size(); i++) {
        void *Player = (*espManager->enemies)[i]->object;
        if (PlayerAlive(Player)) {  
              
            Vector3 PlayerPos = getPosition(Player);
            Vector3 MyPos = getPosition(myPlayer);
               
            //Head
            Vector3 HeadPos = getPosition(Player);  
            Vector3 Head = Vector3(HeadPos.x, HeadPos.y + 0.5,HeadPos.z);
           
            //Bottom
            Vector3 BottomPos = getPosition(Player);
            Vector3 Bottom = Vector3(BottomPos.x, BottomPos.y - 1.2,BottomPos.z);
              
            //WorldToScreen
            auto HeadPosition = WorldToScreen(Head);
            auto BottomPosition = WorldToScreen(Bottom);  
            
            if (HeadPosition.z < 1.f) continue;
		    if (BottomPosition.z < 1.f) continue;
            
            float boxHeight = abs(HeadPosition.y - BottomPosition.y);
            float boxWidth = boxHeight * 0.65f;
            Rect PlayerRect(HeadPosition.x - (boxWidth / 2), (screenHeight - HeadPosition.y), boxWidth, boxHeight);                                   
                                      
            if(EspObject){
               std::string Allplayers;
     
               Allplayers += "Enemy Players: ";
     
               Allplayers += std::to_string((uintptr_t) espManager->enemies->size());
  
               esp.DrawText(Color::Black(), Allplayers.c_str(), Vector2(screenWidth / 2 ,screenHeight / 2), 35.0f);     
   
            }  
                
            if (EspLine) {                 
                esp.DrawLine(Color::White(), 1,Vector2(screenWidth / 2, 0),Vector2(HeadPosition.x,screenHeight - HeadPosition.y)); 
            }
            
            if(EspBox){
                esp.DrawBox(Color::White(), 1, PlayerRect);                 
            }         
            
             if(EspHealth){
                 esp.DrawHorizontalHealthBar(Vector2(PlayerRect.x + (PlayerRect.width / 30), PlayerRect.y - 20),30, 100, GetPlayerHealth(Player));
             }
             
              if(EspDistance){
                    char extra[30];
                    float DistanceTo = get_3D_Distance(MyPos.x, MyPos.y, MyPos.z, PlayerPos.x, PlayerPos.y, PlayerPos.z);                  
                    sprintf(extra, "%0.0f m", DistanceTo);
                    esp.DrawText(Color::White(), extra,Vector2(PlayerRect.x + (PlayerRect.width / 2),
					PlayerRect.y + PlayerRect.height + 50.5f), 20);      
               } 
            }
        }
    }
}

// we will run our hacks in a new thread so our while loop doesn't block process main thread
void *hack_thread(void *) {
    LOGI(OBFUSCATE("pthread created"));

    //Check if target lib is loaded
    do {
        sleep(1);
    } while (!isLibraryLoaded(targetLibName));
    
    espManager = new ESPManager();
    

    //Anti-lib rename
    /*
    do {
        sleep(1);
    } while (!isLibraryLoaded("libYOURNAME.so"));*/

    LOGI(OBFUSCATE("%s has been loaded"), (const char *) targetLibName);

#if defined(__aarch64__) //To compile this code for arm64 lib only. Do not worry about greyed out highlighting code, it still works
    // Hook example. Comment out if you don't use hook
    // Strings in macros are automatically obfuscated. No need to obfuscate!
	
	A64HookFunction((void *) getAbsoluteAddress(targetLibName, 0x15E95A0), (void *) &Player_update,
                   (void **) &old_Player_update); //Player.Update(); public class Player : MonoBehaviour private void Update() { }
                   
    A64HookFunction((void *) getAbsoluteAddress(targetLibName, 0x15EF404), (void *) &Player_ondestroy,
                   (void **) &old_Player_ondestroy); //Player.OnDestroy(); public class Player : MonoBehaviour private void OnDestroy() { }
				   
				   
#elif defined(__arm__) //To compile this code for armv7 lib only.

    MSHookFunction((void *) getAbsoluteAddress(targetLibName, 0x123456), (void *) &Player_update,
                   (void **) &old_Player_update); //Player.Update(); public class Player : MonoBehaviour private void Update() { }
                   
    MSHookFunction((void *) getAbsoluteAddress(targetLibName, 0x123456), (void *) &Player_ondestroy,
                   (void **) &old_Player_ondestroy); //Player.OnDestroy(); public class Player : MonoBehaviour private void OnDestroy() { }
				   

    LOGI(OBFUSCATE("Done"));
#endif

    //Anti-leech
    /*if (!iconValid || !initValid || !settingsValid) {
        //Bad function to make it crash
        sleep(5);
        int *p = 0;
        *p = 0;
    }*/

    return NULL;
}

// Do not change or translate the first text unless you know what you are doing
// Assigning feature numbers is optional. Without it, it will automatically count for you, starting from 0
// Assigned feature numbers can be like any numbers 1,3,200,10... instead in order 0,1,2,3,4,5...
// ButtonLink, Category, RichTextView and RichWebView is not counted. They can't have feature number assigned
// Toggle, ButtonOnOff and Checkbox can be switched on by default, if you add True_. Example: CheckBox_True_The Check Box
// To learn HTML, go to this page: https://www.w3schools.com/

jobjectArray GetFeatureList(JNIEnv *env, jobject context) {
    jobjectArray ret;

    const char *features[] = {
            OBFUSCATE("Category_ESP"),//Not counted
            OBFUSCATE("Toggle_Esp"),//0                 
            OBFUSCATE("Toggle_Esp Line"),//1 
            OBFUSCATE("Toggle_Esp Box"),//2
            OBFUSCATE("Toggle_Esp Object"),//3
            OBFUSCATE("Toggle_Esp Health"),//4
            OBFUSCATE("Toggle_Esp Distance"),//5
    };

    //Now you dont have to manually update the number everytime;
    int Total_Feature = (sizeof features / sizeof features[0]);
    ret = (jobjectArray)
            env->NewObjectArray(Total_Feature, env->FindClass(OBFUSCATE("java/lang/String")),
                                env->NewStringUTF(""));

    for (int i = 0; i < Total_Feature; i++)
        env->SetObjectArrayElement(ret, i, env->NewStringUTF(features[i]));

    return (ret);
}

void Changes(JNIEnv *env, jclass clazz, jobject obj,
                                        jint featNum, jstring featName, jint value,
                                        jboolean boolean, jstring str) {

    LOGD(OBFUSCATE("Feature name: %d - %s | Value: = %d | Bool: = %d | Text: = %s"), featNum,
         env->GetStringUTFChars(featName, 0), value,
         boolean, str != NULL ? env->GetStringUTFChars(str, 0) : "");

    //BE CAREFUL NOT TO ACCIDENTLY REMOVE break;

    switch (featNum) {
        case 0:
            Esp = boolean;
            break;
        case 1:
            EspLine = boolean;
            break;    
        case 2:
            EspBox = boolean;
            break;
        case 3:
            EspObject = boolean;
            break;
        case 4:
            EspHealth = boolean;
            break;        
        case 5:
            EspDistance = boolean;
            break;      
        case 6: 
            EspSkeleton = boolean;
            break;
    }
}

__attribute__((constructor))
void lib_main() {
    // Create a new thread so it does not block the main thread, means the game would not freeze
    pthread_t ptid;
    pthread_create(&ptid, NULL, hack_thread, NULL);
}

extern "C"
JNIEXPORT void JNICALL
Java_com_android_support_Menu_DrawOn(JNIEnv *env, jclass type, jobject espView, jobject canvas) {
    espOverlay = ESP(env, espView, canvas);
    if (espOverlay.isValid()) {
        DrawESP(espOverlay, espOverlay.getWidth(), espOverlay.getHeight());
    }
}

int RegisterMenu(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Icon"), OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(Icon)},
            {OBFUSCATE("IconWebViewData"),  OBFUSCATE("()Ljava/lang/String;"), reinterpret_cast<void *>(IconWebViewData)},
            {OBFUSCATE("IsGameLibLoaded"),  OBFUSCATE("()Z"), reinterpret_cast<void *>(isGameLibLoaded)},
            {OBFUSCATE("Init"),  OBFUSCATE("(Landroid/content/Context;Landroid/widget/TextView;Landroid/widget/TextView;)V"), reinterpret_cast<void *>(Init)},
            {OBFUSCATE("SettingsList"),  OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(SettingsList)},
            {OBFUSCATE("GetFeatureList"),  OBFUSCATE("()[Ljava/lang/String;"), reinterpret_cast<void *>(GetFeatureList)},
    };

    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Menu"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterPreferences(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("Changes"), OBFUSCATE("(Landroid/content/Context;ILjava/lang/String;IZLjava/lang/String;)V"), reinterpret_cast<void *>(Changes)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Preferences"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;
    return JNI_OK;
}

int RegisterMain(JNIEnv *env) {
    JNINativeMethod methods[] = {
            {OBFUSCATE("CheckOverlayPermission"), OBFUSCATE("(Landroid/content/Context;)V"), reinterpret_cast<void *>(CheckOverlayPermission)},
    };
    jclass clazz = env->FindClass(OBFUSCATE("com/android/support/Main"));
    if (!clazz)
        return JNI_ERR;
    if (env->RegisterNatives(clazz, methods, sizeof(methods) / sizeof(methods[0])) != 0)
        return JNI_ERR;

    return JNI_OK;
}

extern "C"
JNIEXPORT jint JNICALL
JNI_OnLoad(JavaVM *vm, void *reserved) {
    JNIEnv *env;
    vm->GetEnv((void **) &env, JNI_VERSION_1_6);
    if (RegisterMenu(env) != 0)
        return JNI_ERR;
    if (RegisterPreferences(env) != 0)
        return JNI_ERR;
    if (RegisterMain(env) != 0)
        return JNI_ERR;
    return JNI_VERSION_1_6;
}
