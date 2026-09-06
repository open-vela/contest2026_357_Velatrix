############################################################################
# Contest 2026 team 357 - 3D Track Visualization App
############################################################################

include $(APPDIR)/Make.defs

PROGNAME  = hello_app
PRIORITY  = SCHED_PRIORITY_DEFAULT
STACKSIZE = 8192
MODULE    = $(CONFIG_LVX_USE_DEMO_CONTEST2026_357_HELLO_APP)


MAINSRC = track3d_app_main.c


# ============================================================
# 源文件 (按模块分类)
# ============================================================

# loader/ - 轨迹注册与扫描
CSRCS += src/loader/track_registry.c
CSRCS += src/loader/track_loader.c
CSRCS += src/core/math3d.c
CSRCS += src/core/renderer3d.c

# pages/ - 页面管理器
CSRCS += src/pages/page_manager.c
CSRCS += src/pages/track_list_page.c
CSRCS += src/pages/scene_3d_page.c

# ============================================================
# 头文件搜索路径
# ============================================================
APP_ROOT := $(APPDIR)/packages/demos/contest2026_357_hello_app

CFLAGS += -I$(APP_ROOT)/inc
CFLAGS += -I$(APP_ROOT)/inc/core
CFLAGS += -I$(APP_ROOT)/inc/pages
CFLAGS += -I$(APP_ROOT)/inc/loader

include $(APPDIR)/Application.mk
