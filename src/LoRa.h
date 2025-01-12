// Copyright (c) Sandeep Mistry. All rights reserved.
// Licensed under the MIT license. See LICENSE file in the project root for full license information.

#pragma once


#include <functional>
#include <mutex>
#include <atomic>
#include <array>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <Arduino.h>
#include <SPI.h>

#if defined(ARDUINO_SAMD_MKRWAN1300)
#define LORA_DEFAULT_SPI           SPI1
#define LORA_DEFAULT_SPI_FREQUENCY 200000
#define LORA_DEFAULT_SS_PIN        LORA_IRQ_DUMB
#define LORA_DEFAULT_RESET_PIN     -1
#define LORA_DEFAULT_DIO0_PIN      -1
#elif defined(ARDUINO_SAMD_MKRWAN1310)
#define LORA_DEFAULT_SPI           SPI1
#define LORA_DEFAULT_SPI_FREQUENCY 200000
#define LORA_DEFAULT_SS_PIN        LORA_IRQ_DUMB
#define LORA_DEFAULT_RESET_PIN     -1
#define LORA_DEFAULT_DIO0_PIN      LORA_IRQ
#else
#define LORA_DEFAULT_SPI           SPI
#define LORA_DEFAULT_SPI_FREQUENCY 8E6 
#define LORA_DEFAULT_SS_PIN        10
#define LORA_DEFAULT_RESET_PIN     9
#define LORA_DEFAULT_DIO0_PIN      2
#endif

#define PA_OUTPUT_RFO_PIN          0
#define PA_OUTPUT_PA_BOOST_PIN     1

class LoRaClass : public Stream {
public:
  LoRaClass();
  ~LoRaClass();

  int begin(long frequency);
  void end();

  int beginPacket(int implicitHeader = false);
  int endPacket(bool async = false);

  int parsePacket(int size = 0);
  int packetRssi();
  float packetSnr();
  long packetFrequencyError();

  int rssi();

  // from Print
  virtual size_t write(uint8_t byte);
  virtual size_t write(const uint8_t *buffer, size_t size);

  // from Stream
  virtual int available();
  virtual int read();
  virtual int peek();
  virtual void flush();


  void onReceive(std::function<void(int)> callback);
  void onCadDone(std::function<void(bool)> callback);
  void onTxDone(std::function<void()> callback);

  void receive(int size = 0);
  void channelActivityDetection(void);

  void idle();
  void sleep();

  void setTxPower(int level, int outputPin = PA_OUTPUT_PA_BOOST_PIN);
  void setFrequency(long frequency);
  void setSpreadingFactor(int sf);
  void setSignalBandwidth(long sbw);
  void setCodingRate4(int denominator);
  void setPreambleLength(long length);
  void setPreambleTimeout(uint16_t timeout);
  void setSyncWord(int sw);
  void enableCrc();
  void disableCrc();
  void enableInvertIQ();
  void disableInvertIQ();
  void enableLowDataRateOptimize();
  void disableLowDataRateOptimize();
  
  void setOCP(uint8_t mA); // Over Current Protection control
  
  void setGain(uint8_t gain); // Set LNA gain

  // deprecated
  void crc() { enableCrc(); }
  void noCrc() { disableCrc(); }

  byte random();

  void setPins(int ss = LORA_DEFAULT_SS_PIN, int reset = LORA_DEFAULT_RESET_PIN, int dio0 = LORA_DEFAULT_DIO0_PIN);
  void setSPI(SPIClass& spi);
  void setSPIFrequency(uint32_t frequency);

  void dumpRegisters(Stream& out);

  bool isTransmitting();

private:

  void explicitHeaderMode();
  void implicitHeaderMode();

  int getSpreadingFactor();
  long getSignalBandwidth();

  void setLdoFlag();
  void setLdoFlagForced(const boolean);

  uint8_t readRegister(uint8_t address);
  void writeRegister(uint8_t address, uint8_t value);
  uint8_t singleTransfer(uint8_t address, uint8_t value);

  
  void handleDio0Rise();
  static IRAM_ATTR void Dio0RiseHandler(void* arg);
  
  void registerInterruptHandler();
  void unregisterInterruptHandler();

private:
  SPISettings _spiSettings;
  SPIClass* _spi;
  int _ss;
  int _reset;
  int _dio0;
  long _frequency;
  int _packetIndex;
  int _implicitHeaderMode;

  //callbacks
  std::mutex m_onReceiveCBMutex;
  std::mutex m_onCadDoneCBMutex;
  std::mutex m_onTxDoneCBMutex;
  std::function<void(int)> _onReceive;
  std::function<void(bool)> _onCadDone;
  std::function<void()> _onTxDone;

private:
  //Dio0 handler task
  TaskHandle_t m_dio0HandlerTask;

  //Dio0 handler task stack size
  static constexpr size_t m_dio0HandlerTaskStackSize = 2048;
  //Dio0 handler task priority
  const UBaseType_t m_dio0HandlerTaskPriority = configMAX_PRIORITIES - 1;
  //Dio0 handler task stack
  std::array<uint8_t, m_dio0HandlerTaskStackSize> m_dio0HandlerTaskStack;
  //Dio0 handler task buffer
  StaticTask_t m_dio0HandlerTaskBuffer;

  bool spawnDio0HandlerTask();




};




