#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#ifdef _WIN32
	#include <windows.h> // For sleep - If OS is windows, program will use this for sleep.
#else
	#include <unistd.h>  // Sleep for Linux/Unix
#endif


// ========== CONSTANTS ==========
#define COUNTDOWN_START     10
#define MAX_TEMP_C          85
#define MIN_FUEL_PERCENT    95
#define MAX_PRESSURE_BAR    150
#define GRAVITY 9.81
#define LOG_FILE "telemetry.txt"
#define STARTING_FUEL_PERCENT 100

enum LaunchStatus {
	STATUS_STANDBY = 0,
	STATUS_COUNTDOWN = 1,
	STATUS_IGNITION = 2,
	STATUS_LIFTOFF = 3,
	STATUS_ABORT = 4
};

enum SystemCheck {
	CHECK_PASSED = 1,
	CHECK_FAILED = 0
};

typedef union {
	uint8_t all;
	struct {
		uint8_t engine_ready	:1;
		uint8_t fuel_loaded		:1;
		uint8_t nav_online		:1;
		uint8_t comm_active		:1;
		uint8_t weather_clear	:1;
		uint8_t range_safe		:1;
		uint8_t crew_ready		:1;
		uint8_t abort_armed		:1;
	}flags;
}SystemFlags;

typedef struct Telemetry {
	uint8_t time_sec;
	uint8_t engine_temp_c;
	uint8_t fuel_percent;
	uint16_t chamber_pressure;
	int16_t altitude_m;
	SystemFlags systems;
}TelemetryPacket;

// ========== FUNCTIONS ==========

void ABORT_OPERATION(TelemetryPacket *telemetry) {
	telemetry->systems.all = 0x00;
	printf("\n [EMERGENCY] ABORT SEQUENCE ACTIVATED! \n");
	printf("Reason: Critical sensor thresholds exceeded.\n");
	printf("All engines cut off. Ground safety systems engaged.\n");
}

void init_systems(SystemFlags *sys) {
	sys->all = 0xFF;
}

uint8_t read_sensors() {
	uint8_t temperature_C = rand() % 101;
	return temperature_C;
}

uint16_t read_pressure(TelemetryPacket *telemetry) {
	return telemetry->chamber_pressure = rand() % 151;
}

uint8_t read_fuel(TelemetryPacket *telemetry) {
	static uint8_t current_fuel = STARTING_FUEL_PERCENT;

	if (current_fuel > 0) {
		current_fuel--;
	}
	return telemetry->fuel_percent = current_fuel;
}

uint8_t is_system_safe(TelemetryPacket *telemetry) {
	if(telemetry->systems.all != 0xFF) {
		printf("\n--------------------\nCan't start launch procedure! All conditions were not met!\n");
		printf("--------------------\n");
		ABORT_OPERATION(telemetry);
		return 0;
	}

	if (telemetry->chamber_pressure > 135 || telemetry->engine_temp_c > 85 || telemetry->fuel_percent< 10) {
		printf("\n--------------------\nCan't start launch procedure! All conditions were not met!\n");
		printf("--------------------\n");
		ABORT_OPERATION(telemetry);
		return 0;
	}

	return 1;
}

// ========== END OF FUNCTIONS==========

// ========== MAIN BLOCK ==========
int main() {
	srand(time(NULL));
	TelemetryPacket flight_data;
	init_systems(&flight_data.systems);

	FILE *log_file = fopen(LOG_FILE, "w");
	if (log_file == NULL) {
		printf("\nFailed to create log file!\n");
		printf("Logs will be visible on terminal only.");
	}

	printf("== ROKET FIRLATMA KONTROL MERKEZİ ==\n");
    printf("====================================\n\n");

	uint8_t launch_success = 1;

	for(int8_t t = 	COUNTDOWN_START; t>=0; t--) {
		//UPDATE TELEMETRY
		flight_data.time_sec = t;
        flight_data.engine_temp_c = read_sensors();
        read_pressure(&flight_data);
        flight_data.fuel_percent = read_fuel(&flight_data);

		if (!is_system_safe(&flight_data)) {
			launch_success = 0; //Launch failed!
			break;
		}

		printf("T-%2d | Temp: %2d C | Press: %3d Bar | Fuel: %%%2d | Status: GO\n",
				t, flight_data.engine_temp_c, flight_data.chamber_pressure, flight_data.fuel_percent);

		if (log_file != NULL) {
			fprintf(log_file, "T-%d, Temp:%d, Press:%d, Fuel:%d, Status:GO\n",
					t, flight_data.engine_temp_c, flight_data.chamber_pressure, flight_data.fuel_percent);
			//Even if rocket explodes, we can see last logs thanks to fflush
			fflush(log_file);
		}

		#ifdef _WIN32
				Sleep(1000);
		#else
				sleep(1);
		#endif
	}
	fclose(log_file);
	if (launch_success) {
		printf("\n----- ATEŞLEME! -----\n");
		printf("||| ROKET HAVALANDI! YUKSELIS BASLIYOR... |||\n");
		// Fizik motoru buraya gelecek!
	} else {
		printf("\n*** FIRLATMA IPTAL EDILDI. LOGLARI KONTROL EDIN. ***\n");
	}

	return 0;

}