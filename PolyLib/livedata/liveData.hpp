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