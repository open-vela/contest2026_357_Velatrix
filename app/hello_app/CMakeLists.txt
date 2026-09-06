# Contest 2026 team 357 - 3D Track Visualization App
if(CONFIG_LVX_USE_DEMO_CONTEST2026_357_HELLO_APP)
  nuttx_add_application(
    NAME hello_app
    SRCS track3d_app_main.c
         src/loader/track_registry.c
         src/loader/track_loader.c
         src/core/math3d.c
         src/core/renderer3d.c
         src/pages/page_manager.c
         src/pages/track_list_page.c
         src/pages/scene_3d_page.c
    STACKSIZE 8192)
endif()
