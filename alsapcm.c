#include <stdio.h> 		        /* perror( ) 시스템 호출을 위해 사용 */
#include <limits.h> 		      /* SHRT_MAX 상수를 위해서 사용 */
#include <math.h> 		        /* sin( ) 함수를 위해서 사용 */
#include <alsa/asoundlib.h> 	/* ALSA 사운드 시스템의 헤더 파일 */

#define PCM_DEVICE  "default"
#define FREQUENCY   440         // 440Hz - A4 음
#define SAMPLE_RATE 44100       // 표준 샘플링 레이트
#define DURATION    3           // 출력할 시간 (초 단위)
#define AMPLITUDE   20000       // 사인파 진폭 (16비트 범위 내)
#define MODE        1           // 채널 수 : 모노

static int setupDSP(snd_pcm_t *dev, int format, int sampleRate, int channels, int frames);

int main(int argc, char** argv) 
{
  snd_pcm_t *pcm_handle;
  uint32_t format = SND_PCM_FORMAT_S16_LE;
  uint32_t sample_rate = SAMPLE_RATE;
  int channels = MODE;        // 모노 오디오
  int seconds = DURATION;
  int frames_per_buffer = 32; // 버퍼당 프레임 수
  int num_samples = SAMPLE_RATE * seconds;
  int ret;
  int16_t *buffer;            // 입출력 오디오를 위한 버퍼

  // ALSA PCM 장치 열기
  if ((ret = snd_pcm_open(&pcm_handle, PCM_DEVICE, SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
    fprintf(stderr, "ERROR: Can't open \"%s\" PCM device. %s\n", PCM_DEVICE, snd_strerror(ret));
    return 1;
  }

  /* 오디오 디바이스 설정 */
  if(setupDSP(pcm_handle, format, sample_rate, channels, frames_per_buffer) < 0) {
    perror("Cound not set output audio device");
    return -1;
  }

  // 정현파 데이터 생성
  buffer = (int16_t *)malloc(num_samples * sizeof(int16_t));
  for (int i = 0; i < num_samples; i++) {
    buffer[i] = AMPLITUDE * sin((2.0 * M_PI * FREQUENCY * i) / SAMPLE_RATE);
  }

  // PCM 데이터 쓰기 및 출력
  for (int i = 0; i < num_samples; i += frames_per_buffer) {
    if ((ret = snd_pcm_writei(pcm_handle, &buffer[i], frames_per_buffer)) == -EPIPE) {
      snd_pcm_prepare(pcm_handle);
    } else if (ret < 0) {
      fprintf(stderr, "ERROR: Can't write to PCM device. %s\n", snd_strerror(ret));
      break;
    }
  }

  /* 오디오 버퍼 버리기 */
  //  snd_pcm_drop(pcm_handle);

  // 리소스 해제
  snd_pcm_drain(pcm_handle);
  free(buffer);

  /* 오디오 디바이스 닫기 */
  snd_pcm_close(pcm_handle);

  return 0;
}

int setupDSP(snd_pcm_t *dev, int format, int sampleRate, int channels, int frames)
{
  snd_pcm_hw_params_t *params;
  int ret;

  // 하드웨어 파라미터 설정
  /* 오디오 디바이스의 매개변수 구조체를 위한 메모리 할당 */
  snd_pcm_hw_params_alloca(&params);

  /* 오디오 디바이스의 매개변수들을 초기화한다. */
  if(snd_pcm_hw_params_any(dev, params) < 0) {
    perror("Cound not initialize parameter");
    return -1;
  }

  /* 오디오 데이터의 접근(access) 타입(인터리브드, 논인터리브드)을 설정한다. */
  if(snd_pcm_hw_params_set_access(dev, params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
    perror("Cound not set access type");
    return -1;
  }

  /* 샘플의 포맷을 설정한다: 부호 있는 16비트 리틀 엔디안 */
  if(snd_pcm_hw_params_set_format(dev, params, format) < 0) {
    perror("Cound not set sample format");
    return -1;
  }

  /* 샘플링 레이트를 설정한다: 44.1KHz(CD 수준의 품질) */
  if(snd_pcm_hw_params_set_channels(dev, params, channels) < 0) {
    perror("Cound not set sampling rate");
    return -1;
  }

  /* 채널 설정: MONO(1) */
  if(snd_pcm_hw_params_set_rate_near(dev, params, &sampleRate, NULL) < 0) {
    perror("Cound not set channel count");
    return -1;
  }

  /* 프레임 주기 설정 */
  if(snd_pcm_hw_params_set_periods_near(dev, params, &frames, 0) < 0) {
    perror("Count not set fragments");
    return -1;
  }

  /* 앞에서 설정한 오디오 디바이스의 매개변수를 ALSA 시스템에 적용 */
  if ((ret = snd_pcm_hw_params(dev, params)) < 0) {
    fprintf(stderr, "ERROR: Can't set hardware parameters. %s\n", snd_strerror(ret));
    return -1;
  }

  return 0;
}

