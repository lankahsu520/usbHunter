/***************************************************************************
 * Copyright (C) 2017 - 2020, Lanka Hsu, <lankahsu@gmail.com>, et al.
 *
 * This software is licensed as described in the file COPYING, which
 * you should have received as part of this distribution.
 *
 * You may opt to use, copy, modify, merge, publish, distribute and/or sell
 * copies of the Software, and permit persons to whom the Software is
 * furnished to do so, under the terms of the COPYING file.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ***************************************************************************/
#include <signal.h>
#include <getopt.h>

#include "usbHunter.h"

// ** app **
static int is_quit = 0;
static int is_srv = 0;

UsbX_t usbx_main =
{
	.usb_req= NULL,
	.usb_dev_handle = NULL,
	.usb_cnt = -1,

	.vendor_id = VENDOR_ID,
	.product_id = PRODUCT_ID,

	.usb_hotplug_handle = -1,
	.usb_hotplug_fn = usb_hotplug_cb,

	.usb_reset = 0,

	.usb_iface_idx = 0,
	.usb_claim = -1,
	.usb_endpoint = 1,
	.bmAttributes = 0x03,
	.wMaxPacketSize = 0x40,

	.res_size = 0,
	.req_size = 0,
	.usb_timeout = 2000,

	.dbg_lvl = LIBUSB_LOG_LEVEL_INFO,
	//.dbg_lvl = LIBUSB_LOG_LEVEL_DEBUG,
};

#define SYS_TTY_PATH "/sys/class/tty"

static void usb2tty(struct libusb_context *ctx, struct libusb_device *usb_dev, libusb_hotplug_event event, void *user_data)
{
	//UsbX_t *usbX_req = (UsbX_t *)user_data;

	struct libusb_device_descriptor desc;
	int ret = libusb_get_device_descriptor(usb_dev, &desc);
	if (ret < 0) {
		DBG_ER_LN("libusb_get_device_descriptor error !!!");
		return;
	}

	char usb_path[LEN_OF_VAL32] = "";
	usbX_dev_path(usb_dev, usb_path, LEN_OF_VAL32);

	if (desc.bDeviceClass == LIBUSB_CLASS_HUB)
	{
		return;
	}

	struct dirent* ent;
	//char* endptr;
	DIR *dir;
	if ((dir=opendir(SYS_TTY_PATH)) == NULL)
	{
		return;
	}

	//usleep(500*1000);
	char tty_name[LEN_OF_NAME64] = "";
	while( (ent = readdir(dir)) != NULL)
	{
		char filename[LEN_OF_FULLNAME] = "";
		if ( SAFE_STRCMP(ent ->d_name, ".") == 0 ) continue;
		if ( SAFE_STRCMP(ent ->d_name, "..") == 0 ) continue;

		SAFE_SPRINTF_EX(filename, "%s/%s", SYS_TTY_PATH, ent->d_name);

		DBG_DB_LN("(filename: %s)", filename);
		char spath[LEN_OF_DIRNAME] = "";
		if ( file_spath(filename, spath, LEN_OF_DIRNAME) )
		{
			if ( SAFE_STRSTR(spath, usb_path) )
			{
				SAFE_SPRINTF_EX(tty_name, "%s", ent->d_name);
				break;
			}
		}
	}
	closedir(dir);

	switch ( event )
	{
		case LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED:
			if (is_srv)
			{
				DBG_WN_LN("ARRIVED (%04X:%04X;%s;%s;)", desc.idVendor, desc.idProduct, usb_path, tty_name);
			}
			else
			{
				printf("ARRIVED;%04X:%04X;%s;%s;\n", desc.idVendor, desc.idProduct, usb_path, tty_name);
			}
			break;
		case LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT:
		default:
			if (is_srv)
			{
				DBG_WN_LN("LEFT (%04X:%04X;%s;%s;)", desc.idVendor, desc.idProduct, usb_path, tty_name);
			}
			else
			{
				printf("LEFT;%04X:%04X;%s;%s;\n", desc.idVendor, desc.idProduct, usb_path, tty_name);
			}
			break;
	}
}

int usb_hotplug_cb(struct libusb_context *ctx, struct libusb_device *usb_dev, libusb_hotplug_event event, void *user_data)
{
	//UsbX_t *usbX_req = (UsbX_t *)user_data;

	usb2tty(ctx, usb_dev, event, user_data);

	return 0;
}

static int app_quit(void)
{
	return is_quit;
}

#ifdef USE_UV

static uv_loop_t *uv_loop = NULL;

void app_stop_uv(uv_async_t *handle, int force)
{
	static int is_free = 0;
	if ( (is_free==0) && (app_quit()==1) )
	{
		is_free = 1;
		if (uv_loop)
		{
			if (handle)
			{
				SAFE_UV_CLOSE(handle, NULL);
			}

			if (force)
			{
				SAFE_UV_LOOP_CLOSE(uv_loop);
			}
		}
	}
}

#ifdef USE_ASYNC_CREATE
uv_async_t uv_async_fd;

void async_loop(uv_async_t *handle)
{
	app_stop_uv(handle, 0);
}
#endif
#endif

static void app_set_quit(int mode)
{
	is_quit = mode;
}

static void app_stop(void)
{
	if (app_quit()==0)
	{
		app_set_quit(1);

		if (is_srv==1)
		{
			usbX_listen_close(&usbx_main);
		}
		else
		{
			usbX_close(&usbx_main);
		}
#ifdef USE_UV
#ifdef USE_ASYNC_CREATE
		SAFE_UV_ASYNC(&uv_async_fd);
#else
#error "Please use USE_ASYNC_CREATE !!!"
		app_stop_uv(NULL, 1);
#endif
#endif
		DBG_DB_LN("%s (%s)", DBG_TXT_BYE_BYE, TAG);
	}
}

static void app_loop(void)
{
	if (is_srv==1)
	{
		usbX_listen_open(&usbx_main, NULL);
	}
	else
	{
		usbX_browse(&usbx_main, NULL);
	}

#ifdef USE_UV
	{
		SAFE_UV_LOOP_INIT(uv_loop);

#ifdef USE_ASYNC_CREATE
		SAFE_UV_ASYNC_INIT(uv_loop, &uv_async_fd, async_loop);
#endif

		SAFE_UV_LOOP_RUN(uv_loop);
		SAFE_UV_LOOP_CLOSE(uv_loop);
	}
#else
	while ( (is_srv==1) && (app_quit()==0) )
	{
		sleep(1);
	}
#endif

	goto exit_loop;

exit_loop:
	app_stop();
}

static int app_init(void)
{
	int ret = 0;

	return ret;
}

static void app_exit(void)
{
	app_stop();
}

static void app_signal_handler(int signum)
{
	DBG_ER_LN("(signum: %d)", signum);
	switch (signum)
	{
		case SIGINT:
		case SIGTERM:
		case SIGHUP:
			app_stop();
			break;
		case SIGPIPE:
			break;

		case SIGUSR1:
			break;

		case SIGUSR2:
			dbg_lvl_round();
			DBG_ER_LN("dbg_lvl_get(): %d", dbg_lvl_get());
			DBG_ER_LN("(Version: %s)", version_show());
			break;
	}
}

static void app_signal_register(void)
{
	signal(SIGINT, app_signal_handler );
	signal(SIGTERM, app_signal_handler );
	signal(SIGHUP, app_signal_handler );
	signal(SIGUSR1, app_signal_handler );
	signal(SIGUSR2, app_signal_handler );

	signal(SIGPIPE, SIG_IGN );
}

int option_index = 0;
const char* short_options = "d:sh";
static struct option long_options[] =
{
	{ "debug",       required_argument,   NULL,    'd'  },
	{ "srv",         required_argument,   NULL,    's'  },
	{ "help",        no_argument,         NULL,    'h'  },
	{ 0,             0,                      0,    0    }
};

static void app_showusage(int exit_code)
{
	printf( "Usage: %s\n"
					"  -d, --debug       debug level\n"
					"  -s, --srv         daemon mode\n"
					"  -h, --help\n", TAG);
	printf( "Version: %s\n", version_show());
	printf( "Example:\n"
					"  %s -s\n", TAG);
	exit(exit_code);
}

static void app_ParseArguments(int argc, char **argv)
{
	int opt;

	while((opt = getopt_long (argc, argv, short_options, long_options, &option_index)) != -1)
	{
		switch (opt)
		{
			case 'd':
				if (optarg)
				{
					dbg_lvl_set(atoi(optarg));
				}
				break;
			case 's':
				is_srv = 1;
				break;
			default:
				app_showusage(-1);
				break;
		}
	}
}

int main(int argc, char *argv[])
{
	app_ParseArguments(argc, argv);
	app_signal_register();
	atexit(app_exit);
	//dbg_lvl_set(DBG_LVL_DEBUG);

	SAFE_STDOUT_NONE();
	if ( app_init() == -1 )
	{
		return -1;
	}

	app_loop();

	//DBG_WN_LN(DBG_TXT_BYE_BYE);
	return 0;
}
