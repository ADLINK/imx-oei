
ifeq ($(SOM),mx95lp5)

ifeq ($(R),A0)
#here is timming for A1
OBJS += $(OUT)/XIMX95LPD5EVK19_6400mbps_train_timing_a1.o
OBJS += $(OUT)/4GB_MT62F1G32D2DS_timing.o
OBJS += $(OUT)/2GB_MT62F1G16D1DS_timing.o
OBJS += $(OUT)/8GB_MT62F2G32D4DS_timing.o
OBJS += $(OUT)/16G_MT62F4G32D8DV_timing.o
else ifeq ($(R),B0)
#here is timming for B0
OBJS += $(OUT)/MIMX95_LPDDR5_EVK_19X19_6400MTS_FW2024.09_timing.o
OBJS += $(OUT)/4GB_MT62F1G32D2DS_B0_timing.o
OBJS += $(OUT)/2GB_MT62F1G16D1DS_B0_timing.o
OBJS += $(OUT)/8GB_MT62F2G32D4DS_B0_timing.o
OBJS += $(OUT)/16G_MT62F4G32D8DV_B0_timing.o

endif

else ifeq ($(SOM),mx95lp4x-15)
#here is timming for 95 15x15 B0 , 16GB not ready yet
OBJS += $(OUT)/MIMX95_LPDDR4X_EVK_15X15_4000MTS_FW2024.09_timing.o
OBJS += $(OUT)/2GB_MT53E512M32D1ZW_timing.o
OBJS += $(OUT)/4GB_MT53E1G32D2FW_timing.o
OBJS += $(OUT)/8GB_MT53E2G32D4DE_timing.o
OBJS += $(OUT)/16GB_MT53E4G32D8GS_timing.o
endif

VPATH += oei/dat

IMG += oei-$(cpu)-dat

include $(ROOT_DIR)/oei/makefiles/gcc_cross.mak
