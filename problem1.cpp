#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;               // synchronization
std::condition_variable cond; // signaling
std::vector<bool> hasEaten;   // track whether non-leader guests eaten
bool cupcake = true;          // cupcake availability
int leaderCount = 0;          // num leader-eaten cupcakes
int nGuests;                  // total guests

void enterLabyrinth(int id, bool isLeader)
{
    std::unique_lock<std::mutex> lock(mtx); // lock curr thread

    if (isLeader)
    {
        while (leaderCount < nGuests - 1)
        {
            // wait for cupcake to be eaten by a guest
            cond.wait(lock, [&] { return !cupcake; });

            // leader confirms a guest's visit by eating the cupcake
            cupcake = false; // eat
            leaderCount++;
            std::cout << "Leader has eaten the cupcake. Confirmed guests: " << leaderCount << std::endl;
            cond.notify_all();

            // replace cupcake for other guests
            if (leaderCount < nGuests - 1)
            {
                cupcake = true;
            }
        }
        std::cout << "All " << nGuests << " guests have visited the labyrinth!" << std::endl;
    }
    else
    {
        // wait for the cupcake to be available
        cond.wait(lock, [&] { return cupcake; });

        // non-leader guests eat the cupcake only on their first visit
        if (!hasEaten[id])
        {
            cupcake = false; // eat
            hasEaten[id] = true;
            std::cout << "Guest " << id << " has eaten the cupcake." << std::endl;
            cond.notify_all();
        }
    }
}

int main()
{
    std::cout << "Enter the total number of guests: ";
    std::cin >> nGuests;

    if (nGuests < 1)
    {
        std::cout << "Invalid number of guests." << std::endl;
        return 0;
    }

    if (nGuests == 1)
    {
        std::cout << "The only guest visited the labyrinth and ate the cupcake!" << std::endl;
        return 0;
    }

    hasEaten.resize(nGuests, false);

    // make thread for each guest; leader = last guest
    std::vector<std::thread> guests;
    for (int i = 1; i <= nGuests; i++)
    {
        guests.emplace_back(std::thread(enterLabyrinth, i, i == (nGuests - 1)));
    }

    // wait for all threads to complete
    for (auto &guest : guests)
    {
        guest.join();
    }

    return 0;
}