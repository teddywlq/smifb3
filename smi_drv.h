// SPDX-License-Identifier: GPL-2.0+
// Copyright (c) 2023, SiliconMotion Inc.


#ifndef __SMI_DRV_H__
#define __SMI_DRV_H__


#include "smi_ver.h"

#include <drm/drm_edid.h>
#include <drm/drm_encoder.h>
#include <drm/drm_fb_helper.h>
#include <drm/drm_gem.h>
#include <video/vga.h>


#include <drm/drm_gem_vram_helper.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(5, 5, 0)
#include <drm/drm_vram_mm_helper.h>
#endif


#include <linux/i2c-algo-bit.h>
#include <linux/i2c.h>

#include "smi_priv.h"

#define DRIVER_AUTHOR "SiliconMotion"

#define DRIVER_NAME		"smifb"
#define DRIVER_DESC		"SiliconMotion GPU DRM Driver"
#define DRIVER_DATE		"20231127"

#define DRIVER_MAJOR		3
#define DRIVER_MINOR		0
#define DRIVER_PATCHLEVEL	0

#define SMIFB_CONN_LIMIT 3



#define RELEASE_TYPE "Linux DRM Display Driver Release"
#define SUPPORT_CHIP " SM750, SM768"


#define _version_	"3.0.0.0"

#undef  NO_WC

#ifdef CONFIG_CPU_LOONGSON3
#define NO_WC
#endif
#ifdef UNUSED
#elif defined(__GNUC__)
#define UNUSED(x) UNUSED_##x __attribute__((unused))
#elif defined(__LCLINT__)
#define UNUSED(x) /*@unused@*/ x
#elif defined(__cplusplus)
#define UNUSED(x)
#else
#define UNUSED(x) x
#endif

#define SMI_MAX_FB_HEIGHT 8192
#define SMI_MAX_FB_WIDTH 8192

#define SM768_MAX_MODE_SIZE (32<<20)
#define SM750_MAX_MODE_SIZE (8<<20)

#define MAX_CRTC 2
#define MAX_ENCODER 3


#define smi_DPMS_CLEARED (-1)

extern int smi_pat;
extern int smi_bpp;
extern int force_connect;
extern int lvds_channel;
extern int audio_en;
extern int fixed_width;
extern int fixed_height;
extern int hwi2c_en;
extern int swcur_en;
extern int edid_mode;
extern int lcd_scale;
extern int pwm_ctrl;
extern int ddr_retrain;

struct smi_750_register;
struct smi_768_register;

struct smi_plane {
	struct drm_plane base;

	int crtc;
	void __iomem *vaddr;
	void __iomem *vaddr_base;
	u32 vram_size;
	unsigned long size;
};

static inline struct smi_plane *to_smi_plane(struct drm_plane *plane)
{
	return container_of(plane, struct smi_plane, base);
}

struct smi_device {
	struct drm_device *dev;
	struct snd_card 		*card;	
	unsigned long flags;

	resource_size_t rmmio_base;
	resource_size_t rmmio_size;
	resource_size_t vram_size;
	resource_size_t vram_base;
	void __iomem *rmmio;
	void __iomem *vram;

	int specId;
	
	int m_connector;  //bit 0: DVI, bit 1: VGA, bit 2: HDMI.

	struct drm_encoder *smi_enc_tab[MAX_ENCODER];

	struct smi_mode_info mode_info;

	int num_crtc;
	int fb_mtrr;
	bool need_dma32;
	bool mm_inited;
	void *vram_save;
	union {
		struct smi_750_register *regsave;
		struct smi_768_register *regsave_768;
	};
#ifdef USE_HDMICHIP
	struct edid si9022_edid[2];
#endif
	void *dvi_edid;
	void *vga_edid;
	void *hdmi_edid;
	struct drm_display_mode *fixed_mode;
	bool is_hdmi;
	bool is_boot_gpu;
};

struct smi_encoder {
	struct drm_encoder base;
	int last_dpms;
};

struct smi_connector {
	struct drm_connector base;
	struct i2c_adapter adapter;
	struct i2c_algo_bit_data bit_data;
	unsigned char i2c_scl;
	unsigned char i2c_sda;
	unsigned char i2cNumber;
	bool i2c_hw_enabled;
};

static inline struct smi_connector *to_smi_connector(struct drm_connector *connector)
{
	return container_of(connector, struct smi_connector, base);
}



/* smi_main.c */
int smi_device_init(struct smi_device *cdev, struct drm_device *ddev, struct pci_dev *pdev,
		    uint32_t flags);
void smi_device_fini(struct smi_device *cdev);
int smi_driver_load(struct drm_device *dev, unsigned long flags);
void smi_driver_unload(struct drm_device *dev);


/* smi_plane.c */
struct drm_plane *smi_plane_init(struct smi_device *cdev, unsigned int possible_crtcs,
				 enum drm_plane_type type);

/* smi_mode.c */
int smi_modeset_init(struct smi_device *cdev);
void smi_modeset_fini(struct smi_device *cdev);
int smi_calc_hdmi_ctrl(int m_connector);

#define to_smi_crtc(x) container_of(x, struct smi_crtc, base)
#define to_smi_encoder(x) container_of(x, struct smi_encoder, base)


/* smi_mm.c */
int smi_mm_init(struct smi_device *smi);
void smi_mm_fini(struct smi_device *smi);

/* smi_prime.c */
struct sg_table *smi_gem_prime_get_sg_table(struct drm_gem_object *obj);
struct drm_gem_object *smi_gem_prime_import_sg_table(struct drm_device *dev,
						     struct dma_buf_attachment *attach,
						     struct sg_table *sg);

int smi_audio_init(struct drm_device *dev);
void smi_audio_remove(struct drm_device *dev);

void smi_audio_suspend(void);
void smi_audio_resume(void);

#ifndef DRM_IRQ_ARGS
#define DRM_IRQ_ARGS int irq, void *arg
#endif

irqreturn_t smi_drm_interrupt(DRM_IRQ_ARGS);



#define smi_LUT_SIZE 256
#define CURSOR_WIDTH 64
#define CURSOR_HEIGHT 64

#define PALETTE_INDEX 0x8
#define PALETTE_DATA 0x9

#define USE_DVI 1
#define USE_VGA (1 << 1)
#define USE_HDMI (1 << 2)
#define USE_DVI_VGA (USE_DVI | USE_VGA)
#define USE_DVI_HDMI (USE_DVI | USE_HDMI)
#define USE_VGA_HDMI (USE_VGA | USE_HDMI)
#define USE_ALL (USE_DVI | USE_VGA | USE_HDMI)

/* please use revision id to distinguish sm750le and sm750*/
#define SPC_SM750 	0
#define SPC_SM712 	1
#define SPC_SM502   2
#define SPC_SM768   3
//#define SPC_SM750LE 8

#define PCI_VENDOR_ID_SMI 	0x126f
#define PCI_DEVID_LYNX_EXP	0x0750
#define PCI_DEVID_SM768		0x0768


#define BPP32_RED    0x00ff0000
#define BPP32_GREEN  0x0000ff00
#define BPP32_BLUE   0x000000ff
#define BPP32_WHITE  0x00ffffff
#define BPP32_GRAY   0x00808080
#define BPP32_YELLOW 0x00ffff00
#define BPP32_CYAN   0x0000ffff
#define BPP32_PINK   0x00ff00ff
#define BPP32_BLACK  0x00000000


#define BPP16_RED    0x0000f800
#define BPP16_GREEN  0x000007e0
#define BPP16_BLUE   0x0000001f
#define BPP16_WHITE  0x0000ffff
#define BPP16_GRAY   0x00008410
#define BPP16_YELLOW 0x0000ffe0
#define BPP16_CYAN   0x000007ff
#define BPP16_PINK   0x0000f81f
#define BPP16_BLACK  0x00000000

#endif				/* __SMI_DRV_H__ */
