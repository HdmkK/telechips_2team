#include "vent_app.h"
#include "i2c.h"
#include "filter.h"
#include "motor.h"
#include "ultra.h"
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>




int start_ventilate(){

	//창문이 닫힌 상태가 아님
	if (cur_vent_state != CLOSED_COMPLETELY) return -1;

	printf("opening....\n");

	cur_vent_state = BEING_OPEN;
	vent_lock = 1;

	backward(3, 1000);
	//forward(2, 1200);
	set_ventilate_timer(TIME_KEEP_VENTILATE);
	cur_vent_state = OPEN_COMPLETELY;
}

int stop_ventilate(){

	//창문이 열린 상태가 아님
	if (cur_vent_state != OPEN_COMPLETELY) return -1;

	printf("closing....\n");

	cur_vent_state = BEING_CLOSED;
	//backward(2, 1200);
	forward(3, 1000);
	cur_vent_state = CLOSED_COMPLETELY;
}

void timer_handler(int signum){
	//환기 lock 중지(내부 공기가 좋으면 환기 중지할 수 있다.
	vent_lock = 0;
}

// 데이터 수집 쓰레드
void* thread_func1(void* arg) {


	FILTER precipitation_filter1;



	FILTER precipitation_filter2;
	FILTER distance_filter;
	FILTER air_quality_filter;
	FILTER fine_dust_filter;

	struct mv_avg_queue queue_for_precipitation1 = {
	    .head = 0,
	    .tail = 0,
	    .count = 0,
	    .sum = 0,
	};

	struct mv_avg_queue queue_for_precipitation2 = {
	    .head = 0,
	    .tail = 0,
	    .count = 0,
	    .sum = 0,
	};


	struct mv_avg_queue queue_for_distance = {
	    .head = 0,
	    .tail = 0,
	    .count = 0,
	    .sum = 0,
	};

	struct mv_avg_queue queue_for_air_quality = {
	    .head = 0,
	    .tail = 0,
	    .count = 0,
	    .sum = 0,
	};

	struct mv_avg_queue queue_for_fine_dust = {
	    .head = 0,
	    .tail = 0,
	    .count = 0,
	    .sum = 0,
	};

	precipitation_filter1.data = &queue_for_precipitation1;
	precipitation_filter1.filtering = moving_average;

	precipitation_filter2.data = &queue_for_precipitation2;
	precipitation_filter2.filtering = moving_average;


	distance_filter.data = &queue_for_distance;
	distance_filter.filtering = moving_average;

	air_quality_filter.data = &queue_for_air_quality;
	air_quality_filter.filtering = moving_average;

	fine_dust_filter.data = &queue_for_fine_dust;
	fine_dust_filter.filtering = moving_average;


	float tmp_precipitation1, tmp_precipitation2, tmp_distance, tmp_air_quality, tmp_fine_dust;

	while(1){

		
		//강우량 데이터 수집
		tmp_precipitation1 = precipitation_filter1.filtering(&precipitation_filter1, read_precipitation1());

		
		usleep(SENSOR_M_DLY * 1000);

		tmp_precipitation2 = precipitation_filter2.filtering(&precipitation_filter2, read_precipitation2());
		usleep(SENSOR_M_DLY * 1000);

		//거리 데이터 수집
		tmp_distance = distance_filter.filtering(&distance_filter, read_distance());
		usleep(SENSOR_M_DLY * 1000);

		//공기질 데이터 수집
		tmp_air_quality = air_quality_filter.filtering(&air_quality_filter, read_air_quality());
		usleep(SENSOR_M_DLY * 1000);

		//미세먼지 데이터 수집
		tmp_fine_dust = fine_dust_filter.filtering(&fine_dust_filter, read_fine_dust());
		usleep(SENSOR_M_DLY * 1000);


		//필터링

		printf("강우량1 : %f, 강우량2 : %f, 거리 : %fcm, 공기질 : %f, 미세먼지 : %f\n", tmp_precipitation1, tmp_precipitation2, tmp_distance, tmp_air_quality, tmp_fine_dust);

		//전역변수 업데이트
		pthread_mutex_lock(&mutex);

		sensor_data.precipitation1 = tmp_precipitation1;
		sensor_data.precipitation2 = tmp_precipitation2;
		sensor_data.distance = tmp_distance;
		sensor_data.air_quality = tmp_air_quality;
		sensor_data.fine_dust = tmp_fine_dust;

		pthread_mutex_unlock(&mutex);
	}
	
    
    return NULL;
}

#define DURATION 1

// 판단 및 제어 쓰레드
void* thread_func2(void* arg) {

	float cur_precipitation1, cur_precipitation2, cur_distance, cur_air_quality, cur_fine_dust;
	int outer_condition_bad = 0;

	int start_time1 = 0, end_time1 = 0;
	int start_time2 = 0, end_time2 = 0;
	int start_time3 = 0, end_time3 = 0;

	



	while(1){


		cur_precipitation1 = sensor_data.precipitation1;
		cur_precipitation2 = sensor_data.precipitation2;
		cur_distance = sensor_data.distance;
		cur_air_quality = sensor_data.air_quality;
		cur_fine_dust = sensor_data.fine_dust;

		//printf("강우량 1 : %f, 강우량 2 :%f, 거리 : %fcm, 공기질 : %f, 미세먼지 : %f\n", cur_precipitation1, cur_precipitation2,  cur_distance, cur_air_quality, cur_fine_dust);


		//외부 환경 check
		outer_condition_bad = is_outer_condition_bad(cur_precipitation1, cur_precipitation2, cur_distance, cur_fine_dust);


		//외부 환경 안좋음
		if (outer_condition_bad){

			if (cur_vent_state == OPEN_COMPLETELY){
				vent_lock = 0; //락에 상관없이 긴급 정지
				//환기중지

				if (start_time1 == 0){
					start_time1 = time(NULL);
				}
				else{
					end_time1 = time(NULL);
					if (end_time1 - start_time1 >= DURATION){
						stop_ventilate();
						start_time1 = 0;
						end_time1 = 0;
					}
				}
				
			}
			
			//idle
			continue;
		}

		start_time1 = end_time1 = 0;


		/*여기까지 왔다는 건 외부환경이 환기해도 괜찮은 조건*/

		//printf("cur_air_quality : %d\n", cur_air_quality);
		//내부 이산화탄소 농도가 높은지
		if (cur_air_quality >= AIR_QUALITY_THRESHOLD){
			//내부 이산화탄소 농도 높음
			//printf("cur_vent_state : %d\n", cur_vent_state);
			if (cur_vent_state == CLOSED_COMPLETELY){
				//환기 시작

				//printf("start_time : %d, end_time : %d\n", start_time2, end_time2);//no print

				if (start_time2 == 0){
					start_time2 = time(NULL);
				}
				else{
					end_time2 = time(NULL);
					if (end_time2 - start_time2 >= DURATION){
						start_ventilate();
						start_time2 = 0;
						end_time2 = 0;
					}
				}
			}

			continue;
		}

		start_time2 = end_time2 = 0;


		/*내부 이산화탄소 농도 낮음*/


		if (cur_vent_state == OPEN_COMPLETELY && !vent_lock){
			//환기 중지

			if (start_time3 == 0){
				start_time3 = time(NULL);
			}
			else{
				end_time3 = time(NULL);

				if ( end_time3 - start_time3 >= DURATION){
					stop_ventilate();
					start_time3 = 0;
					end_time3 = 0;
				}
			}
		}
	}
    return NULL;
}


int is_raining(float value1, float value2){
	printf("value1 : %f, value2 : %f\n", value1, value2);
	return ((value1 <= PRECIPITATION_THRESHOLD) && (value2 <= PRECIPITATION_THRESHOLD)) ? 1 : 0;
}


#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8081
#define SEND_INTERVAL 1

void* thread_func3(void* arg){


	int sock;
    struct sockaddr_in server_addr;

    // 소켓 생성
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return EXIT_FAILURE;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);

    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("Invalid address or address not supported");
        close(sock);
        return EXIT_FAILURE;
    }

    // 서버에 연결
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection to the server failed");
        close(sock);
        return EXIT_FAILURE;
    }

    printf("Connected to server %s:%d\n", SERVER_IP, SERVER_PORT);

    // VentDTO 데이터를 주기적으로 전송
    struct VentDTO vent_data = {0};
    while (1) {
        // 예제 데이터 업데이트

        //vent_data.raining = sensor_data.precipitation1;
        vent_data.raining = is_raining(sensor_data.precipitation1, sensor_data.precipitation2);
        printf("rain : %d\n", vent_data.raining);
        vent_data.in_tunnel = sensor_data.distance;
        vent_data.air_condition = sensor_data.air_quality;
        vent_data.fine_dust = sensor_data.fine_dust;

        // 데이터 전송
        ssize_t bytes_sent = send(sock, &vent_data, sizeof(vent_data), 0);
        if (bytes_sent < 0) {
            perror("Failed to send data");
            break;
        }
        //printf("Sent data: raining=%d, in_tunnel=%d, air_condition=%d, fine_dust=%d\n",
               //vent_data.raining, vent_data.in_tunnel, vent_data.air_condition, vent_data.fine_dust);

        // 주기적으로 전송
        sleep(SEND_INTERVAL);
    }

    // 소켓 닫기
    close(sock);
    return EXIT_SUCCESS;
}



	
int is_outer_condition_bad(float cur_precipitation1, float cur_precipitation2, float cur_distance, float cur_fine_dust){

		int condition_bad = 0;

		//비가 오는가?
		if ((cur_precipitation1 <= PRECIPITATION_THRESHOLD) && (cur_precipitation2 <= PRECIPITATION_THRESHOLD)){
			condition_bad = 1;
		}

		//터널인가?
		else if (cur_distance <= DISTANCE_THRESHOLD){
			condition_bad = 1;
		}

		//외부 미세먼지 농도가 높은가?
		else if (cur_fine_dust >= FINE_DUST_THRESHOLD){
			condition_bad = 1;
		}
		//printf("condition? : %d\n", condition_bad);

		return condition_bad;
}

void init_ventilate_timer(){

	struct sigaction sa;

	struct itimerspec* timer = &ventilate_timer.timer;
	timer_t* timerid = &ventilate_timer.timerid;
	
	sa.sa_flags = SA_SIGINFO;
	sa.sa_handler = timer_handler;
	sigemptyset(&sa.sa_mask);
	sigaction(SIGALRM, &sa, NULL);


	// 타이머 생성
    timer_create(CLOCK_REALTIME, NULL, timerid);

}




void set_ventilate_timer(int sec){

	// 타이머 간격 및 초기 시간 설정
    ventilate_timer.timer.it_value.tv_sec = sec; 
    ventilate_timer.timer.it_value.tv_nsec = 0;
    ventilate_timer.timer.it_interval.tv_sec = 0;   
    ventilate_timer.timer.it_interval.tv_nsec = 0;

    timer_settime(ventilate_timer.timerid, 0, &ventilate_timer.timer, NULL);
}

int read_air_quality(){
	ret48 = 0;
	ret48 = get_data_from_addr(i2cfile,I2C_ADDR);
    //usleep(100*1000);
    return ret48;
}

int read_precipitation1(){
	ret49 = 0;
    ret49 = get_data_from_addr(i2cfile,I2C_ADDR2);
    //usleep(100*1000);
    return ret49;
}

//commit yang
int read_precipitation2(){
	ret4B = 0;
	ret4B = get_data_from_addr(i2cfile,I2C_ADDR4);
	//usleep(100*1000);
	return ret4B;
}

int read_distance(){
	return get_distance();
}

int read_fine_dust(){
	ret4A=0;
    fprintf(valueFile, "0");
    fflush(valueFile);
    //200us
    usleep(SAMPLING_TIME);
    //80us
    ret4A = get_data_from_addr(i2cfile,I2C_ADDR3);
    fprintf(valueFile, "1");
    fflush(valueFile);
    usleep(OFF_TIME);

    return ret4A;
}

int main() {

	printf("start\n");
	init_i2c();
	motor_gpio_init();
	ultra_gpio_init();


	cur_vent_state = CLOSED_COMPLETELY;

    pthread_t thread1, thread2, thread3;

    init_ventilate_timer();

    pthread_mutex_init(&mutex, NULL);

    // 데이터 수집 쓰레드 생성
    if (pthread_create(&thread1, NULL, thread_func1, NULL) != 0) {
        perror("Failed to create thread1");
        return EXIT_FAILURE;
    }

    // 판단 및 제어 쓰레드 생성
    if (pthread_create(&thread2, NULL, thread_func2, NULL) != 0) {
        perror("Failed to create thread2");
        return EXIT_FAILURE;
    }

    if (pthread_create(&thread3, NULL, thread_func3, NULL) != 0) {
        perror("Failed to create thread3");
        return EXIT_FAILURE;
    }


    // 스레드 1 종료 대기
    if (pthread_join(thread1, NULL) != 0) {
        perror("Failed to join thread1");
        return EXIT_FAILURE;
    }

    // 스레드 2 종료 대기
    if (pthread_join(thread2, NULL) != 0) {
        perror("Failed to join thread2");
        return EXIT_FAILURE;
    }

    if (pthread_join(thread3, NULL) != 0) {
        perror("Failed to join thread3");
        return EXIT_FAILURE;
    }



    pthread_mutex_destroy(&mutex);

    destory_i2c();

    return EXIT_SUCCESS;
}







