#pragma once

#include "datacore/datacore.hpp"
#include <list>
#include <string>
#include <vector>

struct Key {
    uint8_t note;
    uint8_t velocity;
};

struct NoteOutStates {
    uint8_t note;
    uint8_t velocity;
    uint8_t gate;
};

// this class handles all data occuring during live play
class LiveData {
  public:
    LiveData() {}
    ~LiveData() {}

    // inputs
    std::list<Key> pressedKeys[2];

    // live settings

    // outputs
    NoteOutStates noteOutputs[2][8];

    // functions
    void keyPressed(uint8_t channel, uint8_t note, uint8_t velocity) {

        for (uint16_t l = 0; l < layers.size(); l++) {
            Key key;
            key.note = note;
            key.velocity = velocity;
            pressedKeys[l].push_front(key);
        }
    }

    void keyReleased(uint8_t channel, uint8_t note) {
        for (uint16_t l = 0; l < layers.size(); l++) {

            for (std::list<Key>::iterator it = pressedKeys[l].begin(); it != pressedKeys[l].end();) {
                if (it->note == note) {
                    pressedKeys[l].erase(it);
                }
                else {
                    it++;
                }
            }
        }
    }

    void controlChange(uint8_t channel, uint8_t cc, uint8_t value) {
        //
    }

    // import Layers
    void init(std::vector<Layer *> &layers) {
        this->layers = layers;
        for (uint16_t l = 0; l < layers.size(); l++) {
            pressedKeys[l].clear();
        }
    }

    std::vector<Layer *> layers;
};

#pragma once

#include "polyControl.hpp"

#define NUMBERLAYER 2
#define NUMBERVOICES 8

/*

Middleman

Zuweisung der Note, Gate, Trigger Signale

clocking

arp



*/

typedef enum { FREE, PLAYED, OFF } voiceStatus;

// struct pro voice für den aktuellen status: gespielter ton, c
typedef struct {
    voiceStatus status = FREE;
    uint8_t note = 0;
    uint8_t velocity = 0;
    uint8_t voiceID = 0;
    uint8_t layerID = 0;
} voiceStruct;

/* class VoiceHandler {
  public:
    VoiceHandler() {
        // init Voices
        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            voicesA[i].voiceID = i;
            voicesA[i].layerID = 0;
        }

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            voicesB[i].voiceID = i;
            voicesB[i].layerID = 1;
        }
    }
    void playNote(uint8_t note, uint8_t velocity, uint8_t numberVoices) {
        getNextVoices(numberVoices);
        for (voiceStruct *v : foundVoices) {
            v->status = PLAYED;
            v->note = note;
            v->velocity = velocity;
            // TODO send Note, Gate, Velocity
        }
    }
    void freeNote(uint8_t note) {
        findVoices(note);
        for (voiceStruct *v : foundVoices) {
            v->status = FREE;
            // TODO send Gate off
        }
    }

    void findVoices(uint8_t note) {
        foundVoices.clear();

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            if (voicesA[i].note == note && voicesA[i].status == PLAYED) {
                foundVoices.push_back(&voicesA[i]);
            }
        }

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            if (voicesB[i].note == note && voicesB[i].status == PLAYED) {
                foundVoices.push_back(&voicesB[i]);
            }
        }
    }

    getNextVoices(uint8_t numberVoices) {




        foundVoices.clear();

        for (uint8_t i = 0; i < NUMBERVOICES; i++) {
            if (voices[i].note == note && voices[i].status == PLAYED) {
                foundVoices.push_back(&voices[i]);
            }
        }
    }

    std::vector<voiceStruct *> foundVoices;
    uint8_t layerID = 0;

    voiceStruct voicesA[NUMBERVOICES];
    voiceStruct voicesB[NUMBERVOICES];
}; */
