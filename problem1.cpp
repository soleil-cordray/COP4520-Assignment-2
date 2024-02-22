// reference:

// invite N guests
// enter lab 1 at a time WHEN invited
// choice to eat cupcake or leave it @ end of lab
// if cupcake eaten by prev guest,
// plate empty - can request new one
// using servants
// when new cupcake brought, choice to eat it or leave it
// guests CANNOT talk to other guests about visit after game started
// CAN come up w strategy BEFORE start
// same guest can be picked to play multiple times
// before party over: guests must announce if all visited lab at least once
// guests need strategy to let know every guest entered

// each guest = 1 running thread
// choose concrete num N or ask user to specify N at start

// STRATEGY
// guests eat cupcake if available
// leader only eats cupcake when its available
    // goal: eat n-1 times ... once reached, know every guest has visited
    // each instance of eating cupcake confirms unique guest has visited

#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx; // synchronization
std::condition_variable cond; // signaling
bool cupcake = true; // cupcake availability
int nGuests;
std::vector<bool> hasEaten; // track whether non-leader guests eaten
int nGuestEaten = 0;
int nLeaderEaten = 0;

void enterLabyrinth(int id, bool isLeader) {
    std::unique_lock<std::mutex> lock(mtx); // lock curr thread

    if (isLeader) {
        // leader
        while (nLeaderEaten < nGuests - 1) {
            if (!cupcake) {
                cond.wait(lock, [] { return cupcake; });
            }
            else
            {
                // leader eats cupcake if present
                cupcake = false;
                nLeaderEaten++;
                std::cout << "Leader has eaten the cupcake. Confirmed guests: " << nLeaderEaten
                    << std::endl;

                // check if all guests accounted for
                if (nLeaderEaten == nGuests - 1) {
                    std::cout << "All " << nGuests << " guests have visited the labyrinth!"
                        << std::endl;

                    // coordinate cupcake status
                    cond.notify_all();

                    return;
                }
            }

            // coordinate threads
            cond.notify_all();

            // cupcake = true;

            // wait for leader to eat cupcake
            cond.wait(lock, [] { return !cupcake; });

            // only replace cupcake if more than 2 guests
            if (nGuests > 2) {
                cupcake = true;
            }
        }
    } else {
        // non-leader
        if (cupcake && !hasEaten[id]) {
            // eat cupcake
            cupcake = false;
            hasEaten[id] = true;
            std::cout << "Guest " << id << " has eaten the cupcake."
                << std::endl;

            // coordinate cupcake status
            cond.notify_all();
        }
        else {
            // cupcake not available; wait
            cond.wait(lock, [] { return !cupcake; });
        }
    }
}

int main() {
    std::cout << "Enter the total number of guests: ";
    std::cin >> nGuests;

    if (nGuests <= 0) {
        std::cout << "Invalid number of guests." << std::endl;
        return 0;
    }

    if (nGuests == 1) {
        std::cout << "The only guest visited the labyrinth and ate the cupcake!" << std::endl;
        return 0;
    }

    hasEaten.resize(nGuests, false);

    std::vector<std::thread> guests;
    for (int i = 0; i < nGuests; i++) {
        // last guest = leader
        // all guests get the chance to enter, leader confirms their entering
        guests.push_back(std::thread(enterLabyrinth, i, i == 0));
    }

    // wait for all threads to complete
    for (auto& guest : guests) {
        guest.join();
    }

    return 0;
}