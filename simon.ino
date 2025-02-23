#include <Arduino_FreeRTOS.h>
#include <queue.h>

#include "game.h"

TaskHandle_t game_task_handle;
TaskHandle_t sound_task_handle;
TaskHandle_t led_task_handle;

QueueHandle_t input_queue_handle;
QueueHandle_t sound_queue_handle;
QueueHandle_t led_queue_handle;

#define RED_INPUT_PIN 8
#define GREEN_INPUT_PIN 9
#define YELLOW_INPUT_PIN 10
#define BLUE_INPUT_PIN 11

#define RED_OUTPUT_PIN 2
#define GREEN_OUTPUT_PIN 3
#define YELLOW_OUTPUT_PIN 4
#define BLUE_OUTPUT_PIN 5

#define BUZZER_OUTPUT_PIN 7

#define BUTTON_DEBOUNCE_TIME 100L

const int RED = 0;
const int GREEN = 1;
const int YELLOW = 2;
const int BLUE = 3;

typedef struct sound_message_t {
  int freq;
  // If duration > 0 turn sound on for duration ms
  // If duration == 0 turn sound off
  // If duration < 0 turn sound on
  int duration;
  int pause;
};

typedef struct led_message_t {
  int color;
  // If duration > 0 turn led on for duration ms
  // If duration == 0 turn led off
  // If duration < 0 turn led on
  int duration;
  int pause;
};

// The ATmega328P has three Pin Change Interrupt (PCI) groups, each associated with an interrupt vector:
//
//     PCINT0 (PCI0) – Port B (Digital Pins 8–13 + SPI pins)
//         Covers: PB0 to PB7 (Arduino D8, D9, D10, D11, D12, D13)
//         Interrupt Vector: PCINT0_vect
//
//     PCINT1 (PCI1) – Port C (Analog Pins A0–A5)
//         Covers: PC0 to PC5 (Arduino A0, A1, A2, A3, A4, A5)
//         Interrupt Vector: PCINT1_vect
//
//     PCINT2 (PCI2) – Port D (Digital Pins 0–7)
//         Covers: PD0 to PD7 (Arduino D0, D1, D2, D3, D4, D5, D6, D7)
//         Interrupt Vector: PCINT2_vect
ISR(PCINT0_vect) {
  static unsigned int r = 0;
  static unsigned int g = 0;
  static unsigned int y = 0;
  static unsigned int b = 0;
  static unsigned long last = 0L;
  unsigned long now = millis();

  if (now - last < BUTTON_DEBOUNCE_TIME) {
    return;
  }

  last = now;

  if (!(PINB & (1 << PB0)) && !digitalRead(RED_INPUT_PIN)) {
    xQueueSendToBackFromISR(input_queue_handle, &RED, NULL);
    return;
  }

  if (!(PINB & (1 << PB1)) && !digitalRead(GREEN_INPUT_PIN)) {
    xQueueSendToBackFromISR(input_queue_handle, &GREEN, NULL);
    return;
  }

  if (!(PINB & (1 << PB2)) && !digitalRead(YELLOW_INPUT_PIN)) {
    xQueueSendToBackFromISR(input_queue_handle, &YELLOW, NULL);
    return;
  }

  if (!(PINB & (1 << PB3)) && !digitalRead(BLUE_INPUT_PIN)) {
    xQueueSendToBackFromISR(input_queue_handle, &BLUE, NULL);
    return;
  }
}

static void enable_buttons(void) {
  // Arduino Pin	ATmega328P Pin	Pin Change Interrupt Group
  // D0 (RX) PD0 PCINT16 (PCI2)
  // D1 (TX) PD1 PCINT17 (PCI2)
  // D2  PD2 PCINT18 (PCI2)
  // D3  PD3 PCINT19 (PCI2)
  // D4  PD4 PCINT20 (PCI2)
  // D5  PD5 PCINT21 (PCI2)
  // D6  PD6 PCINT22 (PCI2)
  // D7  PD7 PCINT23 (PCI2)
  // D8  PB0 PCINT0 (PCI0)
  // D9  PB1 PCINT1 (PCI0)
  // D10 PB2 PCINT2 (PCI0)
  // D11 PB3 PCINT3 (PCI0)
  // D12 PB4 PCINT4 (PCI0)
  // D13 PB5 PCINT5 (PCI0)
  // A0  PC0 PCINT8 (PCI1)
  // A1  PC1 PCINT9 (PCI1)
  // A2  PC2 PCINT10 (PCI1)
  // A3  PC3 PCINT11 (PCI1)
  // A4 (SDA) PC4 PCINT12 (PCI1)
  // A5 (SCL) PC5 PCINT13 (PCI1)

  // Enable pins D8 - D11
  PCICR |= (1 << PCIE0);
  PCMSK0 = (1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3);
}

static void disable_buttons(void) {
  // Disable pins D8 - D11
  PCMSK0 &= ~((1 << PCINT0) | (1 << PCINT1) | (1 << PCINT2) | (1 << PCINT3));
}

void setup() {
  Serial.begin(9600);
  Serial.println("setup() starting up");

  Serial.println("setup() setup input pins");
  pinMode(RED_INPUT_PIN, INPUT_PULLUP);
  pinMode(GREEN_INPUT_PIN, INPUT_PULLUP);
  pinMode(YELLOW_INPUT_PIN, INPUT_PULLUP);
  pinMode(BLUE_INPUT_PIN, INPUT_PULLUP);


  Serial.println("setup() setup ouput pins");
  pinMode(RED_OUTPUT_PIN, OUTPUT);
  pinMode(GREEN_OUTPUT_PIN, OUTPUT);
  pinMode(YELLOW_OUTPUT_PIN, OUTPUT);
  pinMode(BLUE_OUTPUT_PIN, OUTPUT);

  pinMode(BUZZER_OUTPUT_PIN, OUTPUT);

  Serial.println("setup() led queue");
  led_queue_handle = xQueueCreate(1, sizeof(led_message_t));
  Serial.println("setup() sound queue");
  sound_queue_handle = xQueueCreate(1, sizeof(sound_message_t));
  Serial.println("setup() input queue");
  input_queue_handle = xQueueCreate(1, sizeof(int));

  // Enable interrupt port B
  PCICR |= (1 << PCIE0);

  Serial.println("setup() create led task");
  xTaskCreate(led_task,
              "led_task",
              64,
              NULL,
              2,
              &led_task_handle);

  Serial.println("setup() create sound task");
  xTaskCreate(sound_task,
              "sound_task",
              64,
              NULL,
              2,
              &sound_task_handle);

  Serial.println("setup() create game task");
  xTaskCreate(game_task,
              "game_task",
              164,
              NULL,
              2,
              &game_task_handle);

  enable_buttons();
}


void loop() { }

static void play_win_sound(void) {
  sound_message_t soundmsg;

  static const int duration = 100;
  static const int pause = 25;

  for (int i = 0; i < 4; i++) {
    for (int j = 0; j < 3; j++) {
      soundmsg = {
        .freq = 440 + j * 100,
        .duration = duration,
        .pause = pause,
      };
      xQueueSendToBack(sound_queue_handle, &soundmsg, portMAX_DELAY);
    }
  }
}

static void play_lose_sound(void) {
  sound_message_t soundmsg;

  static const int duration = 200;
  static const int pause = 25;

  for (int i = 0; i < 3; i++) {
    soundmsg = {
      .freq = 200,
      .duration = duration,
      .pause = pause,
    };
    xQueueSendToBack(sound_queue_handle, &soundmsg, portMAX_DELAY);
  }
}

void game_task(void *pvParameters) {
  (void) pvParameters;

  game_t g;
  int round = 0;
  int button;

  static const int duration = 500;
  static const int pause = 300;

  sound_message_t soundmsg;
  led_message_t ledmsg;

  for (;;) {

    game_new(&g, round);

    // Drain any lingering button press
    xQueueReceive(input_queue_handle, &button, 0);

    // Wait one second before the next round
    vTaskDelay(1000 / portTICK_PERIOD_MS);

    // Play the sequence for the player
    int color;
    while (game_has_more_colors(&g)) {
      color = game_current_color(&g);
      soundmsg = {
        .freq = 440 + color * 100,
        .duration = duration,
        .pause = pause,
      };
      ledmsg = {
        .color = color,
        .duration = duration,
        .pause = pause,
      };
      xQueueSendToBack(led_queue_handle, &ledmsg, portMAX_DELAY);
      xQueueSendToBack(sound_queue_handle, &soundmsg, 0);
      game_next(&g);
    };

    game_reset(&g);

    enable_buttons();

    // Record and match the sequence entered by the player
    while (game_has_more_colors(&g)) {
      color = game_current_color(&g);

      if (xQueueReceive(input_queue_handle, &button, portMAX_DELAY) != pdTRUE) {
        continue;
      }

      soundmsg = {
        .freq = 440 + button * 100,
        .duration = duration,
        .pause = pause,
      };
      ledmsg = {
        .color = button,
        .duration = duration,
        .pause = pause,
      };
      xQueueSendToBack(led_queue_handle, &ledmsg, portMAX_DELAY);
      xQueueSendToBack(sound_queue_handle, &soundmsg, 0);

      if (color == button) {
        game_next(&g);
        if (game_has_more_colors(&g)) {
          continue;
        }

        play_win_sound();
        round++;
        break;
      }

      // game_over, start over
      play_lose_sound();
      round = 0;
      break;
    };

    disable_buttons();
  }
}


void sound_task(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    sound_message_t msg;

    if (xQueuePeek(sound_queue_handle, &msg, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    switch (msg.duration) {
      case -1:
        tone(BUZZER_OUTPUT_PIN, msg.freq);
        break;
      case 0:
        noTone(BUZZER_OUTPUT_PIN);
        break;
      default:
        tone(BUZZER_OUTPUT_PIN, msg.freq);
        vTaskDelay(msg.duration / portTICK_PERIOD_MS);
        noTone(BUZZER_OUTPUT_PIN);
        vTaskDelay(msg.pause / portTICK_PERIOD_MS);
        break;
    }

    xQueueReceive(sound_queue_handle, &msg, 0);
  }
}

void led_task(void *pvParameters) {
  (void) pvParameters;

  for (;;) {
    led_message_t msg;

    if (xQueuePeek(led_queue_handle, &msg, portMAX_DELAY) != pdTRUE) {
      continue;
    }

    int pin;
    switch (msg.color) {
      case RED: pin = RED_OUTPUT_PIN; break;
      case GREEN: pin = GREEN_OUTPUT_PIN; break;
      case YELLOW: pin = YELLOW_OUTPUT_PIN; break;
      default:
      case BLUE: pin = BLUE_OUTPUT_PIN; break;
    }

    switch (msg.duration) {
      case -1:
        digitalWrite(pin, HIGH);
        break;
      case 0:
        digitalWrite(pin, LOW);
        break;
      default:
        digitalWrite(pin, HIGH);
        vTaskDelay(msg.duration / portTICK_PERIOD_MS);
        digitalWrite(pin, LOW);
        vTaskDelay(msg.pause / portTICK_PERIOD_MS);
        break;
    }

    xQueueReceive(led_queue_handle, &msg, 0);
  }
}
