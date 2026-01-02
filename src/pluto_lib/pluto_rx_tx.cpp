#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <complex>
#include <stdio.h>
#include <vector>

void start_rx(struct SoapySDRDevice *sdr, SoapySDRStream *rxStream,
              int16_t *rx_buffer, int buff_size, int rx_mtu, char *filename,
              int work_time) {
  printf("RX STARTED!\n");

  const long timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)

  /*get current time*/
  time_t start = time(NULL);

  /*open file for write receiving samples*/
  FILE *rx_samples = fopen(filename, "wb");

  if(rx_samples == nullptr){
    printf("start_rx: Error in opening file");
    return;
  }

  long long last_time = 0;

  int iteration = 1;

  /*sdr receive samples work_time seconds*/
  while (difftime(time(NULL), start) < work_time) {
    void *rx_buffs[] = {rx_buffer};
    int flags;        // flags set by receive operation
    long long timeNs; // timestamp for receive buffer

    // считали буффер RX, записали его в rx_buffer
    int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, buff_size,
                                       &flags, &timeNs, timeoutUs);

    // Смотрим на количество считаных сэмплов, времени прихода и разницы во
    // времени с чтением прошлого буфера
    printf(
        "Buffer: %lu - Samples: %i, Flags: %i, Time: %lli, TimeDiff: %llins \n",
        iteration++, sr, flags, timeNs, timeNs - last_time);

    last_time = timeNs;

    fwrite(rx_buffer, 2 * rx_mtu * sizeof(int16_t), 1, rx_samples);
  }

  fclose(rx_samples);
}

void start_tx(struct SoapySDRDevice *sdr, SoapySDRStream *txStream,
              SoapySDRStream *rxStream, int16_t *rx_buffer, 
              int16_t *samples, int tx_samples_count, int buff_size, char* filename, int work_time) {

  /*Мы делаем прием во время передачи, потому что pluto SDR хочет получать время
  отправки, а чтобы его высчитать (у нас на 4 мс в будущее), нужно выполнить
  прием данных*/

  printf("TX STARTED!\n");

  /*файл для отправляемых семплов*/
  FILE* txdata = fopen(filename, "wb");

  const long timeoutUs = 400000; // arbitrarily chosen (взяли из srsRAN)

  /*get current time*/
  time_t start = time(NULL);
  long long last_time = 0;

  while (difftime(time(NULL), start) < work_time) {
    for (size_t offset = 0; offset < tx_samples_count; offset += buff_size * 2) {
      void *rx_buffs[] = {rx_buffer};
      int flags;        // flags set by receive operation
      long long timeNs; // timestamp for receive buffer

      // считали буффер RX, записали его в rx_buffer
      int sr = SoapySDRDevice_readStream(sdr, rxStream, rx_buffs, buff_size,
                                         &flags, &timeNs, timeoutUs);

      last_time = timeNs;

      // Переменная для времени отправки сэмплов относительно текущего приема
      long long tx_time = timeNs + (4 * 1000 * 1000); // на 4 [мс] в будущее

      // Здесь отправляем наш tx_buff массив

      /*проверяем, что при отправке мы не выйдем за границу семплов. Если
      выходим, то часть данных не отправляем, чтобы сохранить стандартный размер
      пакетов по 1920 семплов*/
      if (offset + buff_size * 2 >= tx_samples_count)
        break;

      /*формируем отправляемый буфер путем смещения в нашем массиве семплов*/
      void *tx_buffs[] = {samples + offset};

      /*для отладки пишем отправляемые семплы в файл*/
      fwrite(samples + offset, 2 * buff_size * sizeof(int16_t), 1, txdata);

      flags = SOAPY_SDR_HAS_TIME;

      /*функция для отправки*/
      int st = SoapySDRDevice_writeStream(sdr, txStream,
                                          (const void *const *)tx_buffs, buff_size,
                                          &flags, tx_time, timeoutUs);
      /*проверка ошибки отправки*/
      if ((size_t)st != buff_size) {
        printf("TX Failed: %in", st);
      }
    }
  }
}

void close_pluto_sdr(SoapySDRDevice *sdr, SoapySDRStream *rxStream,
                     SoapySDRStream *txStream) {
  printf("Trying to close Pluto SDR\n");
  // stop streaming
  if (sdr || rxStream || txStream) {
    SoapySDRDevice_deactivateStream(sdr, rxStream, 0, 0);
    SoapySDRDevice_deactivateStream(sdr, txStream, 0, 0);

    // shutdown the stream
    SoapySDRDevice_closeStream(sdr, rxStream);
    SoapySDRDevice_closeStream(sdr, txStream);

    // cleanup device handle
    SoapySDRDevice_unmake(sdr);
  }

  printf("Pluto SDR is closed. Streams are deactivated.\n");
}