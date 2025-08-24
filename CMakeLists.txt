
cmake_minimum_required(VERSION 3.13)
project(quicklang-gui)
include("$ENV{VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "QuickLang GUI")
set(VITA_TITLEID  "QLNGGUI01")
set(VITA_VERSION  "01.00")

add_executable(${PROJECT_NAME}
    src/main.c
)

target_link_libraries(${PROJECT_NAME}
    SceCtrl_stub
    SceKernel_stub
    SceDisplay_stub
    ScePower_stub
    SceAppMgr_stub
    vita2d_stub
    SceSysmodule_stub
    SceGxm_stub
    SceFont_stub
    SceIo_stub
    SceShell_stub
    SceRegMgr_stub
)

vita_create_self(${PROJECT_NAME}.self ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} ${PROJECT_NAME}.self
    VERSION ${VITA_VERSION}
    NAME ${VITA_APP_NAME}
    FILE sce_sys/icon0.png sce_sys/icon0.png
)
