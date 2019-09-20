#ifndef _DISP_DEF_H_
#define _DISP_DEF_H_

#include "hdmitx.h"

#define fgResIs4k2k(res) ((res == HDMI_VIDEO_3840x2160P_23_976HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_24HZ) || (res == HDMI_VIDEO_3840x2160P_25HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_29_97HZ) || (res == HDMI_VIDEO_3840x2160P_30HZ) ||\
			(res == HDMI_VIDEO_4096x2160P_24HZ) || (res == HDMI_VIDEO_3840x2160P_60HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_50HZ) || (res == HDMI_VIDEO_4096x2160P_60HZ) ||\
			(res == HDMI_VIDEO_4096x2160P_50HZ))

#define fgIsFullHDRes(res) ((res == HDMI_VIDEO_1920x1080i_60Hz) || (res == HDMI_VIDEO_1920x1080i_50Hz) ||\
			(res == HDMI_VIDEO_1920x1080p_30Hz) || (res == HDMI_VIDEO_1920x1080p_25Hz) ||\
			(res == HDMI_VIDEO_1920x1080p_24Hz) || (res == HDMI_VIDEO_1920x1080p_23Hz) ||\
			(res == HDMI_VIDEO_1920x1080p_29Hz) || (res == HDMI_VIDEO_1920x1080p_60Hz) ||\
			(res == HDMI_VIDEO_1920x1080p_50Hz) || (res == HDMI_VIDEO_1280x720p3d_60Hz) ||\
			(res == HDMI_VIDEO_1280x720p3d_50Hz) || (res == HDMI_VIDEO_1920x1080i3d_60Hz) ||\
			(res == HDMI_VIDEO_1920x1080i3d_50Hz) || (res == HDMI_VIDEO_1920x1080p3d_24Hz) ||\
			(res == HDMI_VIDEO_1920x1080p3d_23Hz) || (res == HDMI_VIDEO_3840x2160P_23_976HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_24HZ) || (res == HDMI_VIDEO_3840x2160P_25HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_29_97HZ) || (res == HDMI_VIDEO_3840x2160P_30HZ) ||\
			(res == HDMI_VIDEO_4096x2160P_24HZ) || (res == HDMI_VIDEO_3840x2160P_60HZ) ||\
			(res == HDMI_VIDEO_3840x2160P_50HZ) || (res == HDMI_VIDEO_4096x2160P_60HZ) ||\
			(res == HDMI_VIDEO_4096x2160P_50HZ))
#endif				/* __DISP_SESSION_H */
