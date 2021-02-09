
#define RDWR 0x01
#define RDONLY 0x10
#define WRONLY 0x11

struct pcdev_platform_data {
	int size;
	int perm;
	const char *serial_number;
};


