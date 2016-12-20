#include <stdio.h>
#include <stdlib.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

#include "sample_ioctl.h"
#include "led_main.h"

#define DEVICE_NAME "/dev/DoorTalk_Drivers"

#define DEBUG_VERBOSE   0

int fd = 0;

DTDRIVERS_DATA dtdrivers_data;

void init() {
	fd = open(DEVICE_NAME, O_RDWR);
	if (fd <= 0) {
		printf("open() Failed\n");
		exit(-1);
	}
}

void deinit() {
	close(fd);
}

void display_instruction(char* argv[]) {
	printf("\nInvalid parameters !!!\n");
	printf("%s [1] ON\n", argv[0]);
	printf("%s [2] ON (5mins)\n", argv[0]);
	printf("%s [3] SLOW\n", argv[0]);
	printf("%s [4] FAST\n", argv[0]);
	printf("%s [5] ONCE\n", argv[0]);
	printf("%s [6] TWICE\n", argv[0]);
	printf("%s [7] RECOVERY\n", argv[0]);
	printf("%s [8] OFF!\n", argv[0]);
	printf("%s [9] IRLED_ON\n", argv[0]);
	printf("%s [10] IRLED_OFF\n", argv[0]);
	printf("%s [11] SENSOR_ENABLED\n", argv[0]);
	printf("%s [4] SENSOR_DISABLED\n", argv[0]);
	printf("%s [5] MOTION_SENSITIVITY\n", argv[0]);
	printf("%s [6] SENSOR_STATUS\n", argv[0]);
	printf("%s [7] ETHERNET_DISABLED\n", argv[0]);
	printf("%s [8] ETHERNET_ENABLED\n", argv[0]);
	printf("%s [9] DOORLOCK_DISABLED\n", argv[0]);
	printf("%s [10] DOORLOCK_ENABLED\n", argv[0]);
	printf("%s [11] GETRELAY\n", argv[0]);

}

int check_valid_parameters(int argc, char* argv[], int x) {
	int i;
	int operation = 0;

#if DEBUG_VERBOSE
	printf("argc == %d\n", argc); /*TODO*/
	printf("argv == %c\n", *(argv[1])); /*TODO*/
	for(i=0; i<argc; i++) printf("argv[%d] == [%s]\n", i, argv[i]);
#endif

	if (argc < 2)
		goto err1;

	operation = x;
	printf("check%d\n", operation);
	//led_data.special_function_register = VA_GPIO_BASE + strtol(argv[2], NULL, 16);
	dtdrivers_data.value = 0;

	switch (operation) {
	case ON_OPS:
		if (argc != 2)
			goto err1;
		break;

	case ON_DELAY_OPS:
		if (argc != 2)
			goto err1;
		break;

	case SLOW_OPS:
		if (argc != 2)
			goto err1;
		break;

	case FAST_OPS:
		if (argc != 2)
			goto err1;
		break;
	case ONCE_OPS:
		if (argc != 2)
			goto err1;
		break;

	case TWICE_OPS:
		if (argc != 2)
			goto err1;
		break;

	case RECOVERY_OPS:
		if (argc != 2)
			goto err1;
		break;

	case OFF_OPS:
		if (argc != 2)
			goto err1;
		break;

	case IRLED_ON_OPS:
		if (argc != 2)
			goto err1;
		break;

	case IRLED_OFF_OPS:
		if (argc != 2)
			goto err1;
		break;

	case SENSOR_ENABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case SENSOR_DISABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case MOTION_SENSITIVITY_OPS:
		if (argc != 3)
			goto err1;
		dtdrivers_data.value = strtol(argv[2], NULL, 16);
		break;

	case ETHERNET_DISABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case ETHERNET_ENABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case DOORLOCK_DISABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case DOORLOCK_ENABLED_OPS:
		if (argc != 2)
			goto err1;
		break;

	case GETRELAY_OPS:
		if (argc != 2)
			goto err1;
		break;

	default:
		goto err1;
	}

#if DEBUG_VERBOSE
	printf("addr=0x%x val=0x%x\n", dtdrivers_data.special_function_register, dtdrivers_data.value);
#endif
	return operation;

	err1: display_instruction(argv);
	exit(0);
}

void evaluate_operation(int operation) {
	int ret;

	printf("evaluate%d\n", operation);

	switch (operation) {

	case ON_OPS:
		//		printf("ON_OPS\n");
		if (ioctl(fd, ON, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case ON_DELAY_OPS:
		//		printf("ON_DELAY_OPS\n");
		if (ioctl(fd, ON_DELAY, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case SLOW_OPS:
		//		printf("SLOW_OPS\n");
		if (ioctl(fd, SLOW, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case FAST_OPS:
		//		printf("FAST_OPS\n");
		if (ioctl(fd, FAST, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case ONCE_OPS:
		//		printf("NOTIFY_OPS\n");
		if (ioctl(fd, ONCE, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case TWICE_OPS:
		//		printf("NOTIFY_OPS\n");
		if (ioctl(fd, TWICE, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case RECOVERY_OPS:
		//		printf("NOTIFY_OPS\n");
		if (ioctl(fd, RECOVERY, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case OFF_OPS:
		//		printf("OFF_OPS\n");
		if (ioctl(fd, OFF, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case IRLED_ON_OPS:
		//		printf("IRLED_ON_OPS\n");
		if (ioctl(fd, IRLED_ON, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case IRLED_OFF_OPS:
		//		printf("IRLED_OFF_OPS\n");
		if (ioctl(fd, IRLED_OFF, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case SENSOR_ENABLED_OPS:
		//		printf("SENSOR_ENABLED_OPS\n");
		if (ioctl(fd, SENSOR_ENABLED, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case SENSOR_DISABLED_OPS:
		//		printf("SENSOR_DISABLED_OPS\n");
		if (ioctl(fd, SENSOR_DISABLED, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case MOTION_SENSITIVITY_OPS:
		//		printf("MOTION_SENSITIVITY_OPS\n");
		if (ioctl(fd, MOTION_SENSITIVITY, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case SENSOR_STATUS_OPS:
		//		printf("SENSOR_STATUS_OPS\n");
		if (ioctl(fd, SENSOR_STATUS, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		ret = dtdrivers_data.value;
		break;

	case ETHERNET_DISABLED_OPS:
		printf("ETHERNET_DISABLED_OPS\n");
		//			if (ioctl(fd, DOORLOCK_OFF, &dtdrivers_data) < 0) {
		//				printf("ioctl() Failed\n");
		//			}
		//		break;

	case ETHERNET_ENABLED_OPS:
		printf("ETHERNET_ENABLED_OPS\n");
		//	if (ioctl(fd, DOORLOCK_ON, &dtdrivers_data) < 0) {
		//		printf("ioctl() Failed\n");
		//	}
		//	break;

	case DOORLOCK_DISABLED_OPS:
		printf("DOORLOCK_DISABLED_OPS\n");
		if (ioctl(fd, DOORLOCK_OFF, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case DOORLOCK_ENABLED_OPS:
		//		printf("DOORLOCK_ENABLED_OPS\n");
		if (ioctl(fd, DOORLOCK_ON, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		break;

	case GETRELAY_OPS:
		//		printf("SENSOR_STATUS_OPS\n");
		if (ioctl(fd, RELAY_STATUS, &dtdrivers_data) < 0) {
			printf("ioctl() Failed\n");
		}
		ret = dtdrivers_data.value;
		break;
	}
}

int main(int argc, char* argv[]) {
	int operation = 0;
	int x;

	x = atoi(argv[1]);

	printf("%s\n", argv[1]);

	printf("DoorTalk_Drivers Controls\n");

	init();

	operation = check_valid_parameters(argc, argv, x);

	evaluate_operation(operation);

	deinit();
	return 0;
}
